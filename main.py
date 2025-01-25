from fastapi import FastAPI, Request, Form, HTTPException
from fastapi.responses import HTMLResponse, RedirectResponse
from jinja2 import Template
import sqlite3
import uuid
import smtplib

from email.message import EmailMessage

app = FastAPI()

DATABASE = "kryptori.db"


def create_tables_if_not_exist():
    conn = sqlite3.connect(DATABASE)
    cursor = conn.cursor()

    create_advertisement_table_sql = """
    CREATE TABLE IF NOT EXISTS Advertisement (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at TEXT,
        title TEXT NOT NULL,
        description TEXT NOT NULL,
        owner_email TEXT NOT NULL,
        active INT NOT NULL DEFAULT 0,
        owner_token TEXT NOT NULL UNIQUE
    );
    """
    cursor.execute(create_advertisement_table_sql)
    conn.commit()
    conn.close()


def create_advertisement(
    title: str,
    description: str,
    owner_email: str,
):
    owner_token = uuid.uuid4()
    conn = sqlite3.connect(DATABASE)
    cursor = conn.cursor()

    insert_sql = """
    INSERT INTO Advertisement (title, description, owner_email, owner_token)
    VALUES (?, ?, ?, ?)
    """

    cursor.execute(
        insert_sql,
        (title, description, owner_email, str(owner_token)),
    )
    conn.commit()
    conn.close()

    msg = EmailMessage()
    msg["From"] = "noreply@kryptori.com"
    msg["To"] = owner_email
    msg["Subject"] = f"New ad created: {title}"
    msg.set_content(
        (
            f"Thanks for making a new ad!\n"
            f"Title: {title}\n"
            f"Description: {description}\n"
            "Before it goes live, you must go activate the ad by loading the\n"
            "webpage: kryptori.com/manage-ad?token={YOUR_TOKEN_ID}\n\n "
            f"The token for this ad is: {owner_token}\n"
            "and that's where you can update and delete it."
        )
    )

    # Send the message via our own SMTP server.
    s = smtplib.SMTP("localhost")
    s.send_message(msg)
    s.quit()


def fetch_advertisements_html():
    conn = sqlite3.connect(DATABASE)
    cursor = conn.cursor()
    cursor.execute(
        "SELECT id, title, description FROM Advertisement WHERE active = 1 ORDER BY created_at DESC;"
    )
    ads = cursor.fetchall()
    conn.close()

    html = "<ul>"
    for id, title, description in ads:
        html += f"""<li>
        <details>
        <summary>{title} - {description}</summary>
        <p><b>{title}</b>
        <br />
        {description}
        <br />
        Send a reply to the poster!
        </p>
        <form action="/send-message" method="post">
            <input type="hidden" id="ad_id" name="ad_id" value="{id}" required>

            <label for="user_email">Your Email:</label><br>
            <input type="email" id="user_email" name="user_email" required>
            <br />

            <label for="message">Message:</label><br>
            <textarea id="message" name="message" rows="4" required></textarea>
            <br />

            <button type="submit">Send Message</button>
        </form>
        </details>
        </li>"""
    html += "</ul>"
    return html


def fetch_advertisement_by_token(token):
    conn = sqlite3.connect(DATABASE)
    cursor = conn.cursor()
    cursor.execute(
        "SELECT title, description, created_at, updated_at, owner_email FROM Advertisement WHERE owner_token = ? LIMIT 1",
        (token,),
    )
    ad = cursor.fetchone()

    cursor.execute(
        "UPDATE Advertisement SET active = 1 WHERE owner_token = ?",
        (token,),
    )
    conn.commit()
    conn.close()

    return ad


create_tables_if_not_exist()


@app.get("/", response_class=HTMLResponse)
async def index():
    with open("pages/index.html", "r") as f:
        index_page = f.read()

    ads_html = fetch_advertisements_html()
    template = Template(index_page)
    rendered_page = template.render(ads=ads_html)

    return rendered_page


@app.post("/send-message")
async def send_message(
    ad_id: int = Form(...), user_email: str = Form(...), message: str = Form(...)
):
    conn = sqlite3.connect(DATABASE)
    cursor = conn.cursor()

    cursor.execute(
        "SELECT owner_email, title FROM Advertisement WHERE id = ?", (ad_id,)
    )
    owner_email, title = cursor.fetchone()

    if not owner_email:
        conn.close()
        raise HTTPException(status_code=404, detail="Advertisement not found")

    conn.commit()
    conn.close()

    msg = EmailMessage()
    msg["From"] = "noreply@kryptori.com"
    msg["To"] = owner_email
    msg["Reply To"] = user_email
    msg["Subject"] = f"New message about your ad: {title}"
    msg.set_content(
        (
            f"Here's a message from an interested party about your ad {title}!\n"
            f"Message:\n{message}\n"
            f"User email: {user_email}\n"
            "Please reply to their email address."
        )
    )

    # Send the message via our own SMTP server.
    s = smtplib.SMTP("localhost")
    s.send_message(msg)
    s.quit()

    return RedirectResponse(url="/?success=true", status_code=303)


@app.post("/create-ad", response_class=HTMLResponse)
async def create_ad(
    title: str = Form(...),
    description: str = Form(...),
    owner_email: str = Form(...),
):
    create_advertisement(
        title,
        description,
        owner_email,
    )

    return RedirectResponse(url="/", status_code=303)


@app.post("/update-ad", response_class=HTMLResponse)
async def update_ad(
    title: str = Form(...),
    description: str = Form(...),
    token: str = Form(...),
):
    conn = sqlite3.connect(DATABASE)
    cursor = conn.cursor()

    cursor.execute("SELECT * FROM Advertisement WHERE owner_token = ?", (token,))
    existing_ad = cursor.fetchone()

    if not existing_ad:
        conn.close()
        raise HTTPException(status_code=404, detail="Advertisement not found")

    cursor.execute(
        """
        UPDATE Advertisement
        SET title = ?, description = ?, updated_at = CURRENT_TIMESTAMP
        WHERE owner_token = ?
    """,
        (title, description, token),
    )

    conn.commit()
    conn.close()

    return RedirectResponse(
        url=f"/manage-ad?token={token}&success=true", status_code=303
    )


@app.get("/manage-ad", response_class=HTMLResponse)
async def manage_ad(request: Request):
    token = request.query_params.get("token")
    if not token:
        raise HTTPException(status_code=400, detail="Token parameter is required")

    with open("pages/manage_ad.html", "r") as f:
        manage_ad_page = f.read()

    title, description, created_at, updated_at, owner_email = (
        fetch_advertisement_by_token(token)
    )
    template = Template(manage_ad_page)
    rendered_page = template.render(
        title=title,
        description=description,
        created_at=created_at,
        updated_at=updated_at,
        owner_email=owner_email,
        token=token,
    )

    return rendered_page


@app.get("/{path:path}", response_class=HTMLResponse)
async def catch_all(path: str):
    return HTMLResponse(content=f"<h1>Unsupported URI: {path}</h1>", status_code=404)


if __name__ == "__main__":
    import uvicorn

    uvicorn.run(app, host="0.0.0.0", port=8000)
