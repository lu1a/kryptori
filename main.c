#include "deps/mongoose.c"
#include "deps/sqlite3.c"

struct HandlerData {
    sqlite3 *db;
    struct Pages *pages;
};

struct Pages {
    char *index_page;
    char *manage_ad_page;
};

struct Advertisement {
    char *title;
    char *description;
};

int create_tables_if_not_exist(sqlite3 **db) {
    char *err_msg = NULL;
    const char *create_advertisement_table_sql = "CREATE TABLE IF NOT EXISTS Advertisement ("
                                                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                 "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                                                 "updated_at TEXT,"
                                                 "title TEXT NOT NULL,"
                                                 "description TEXT NOT NULL,"
                                                 "is_standing_order INTEGER NOT NULL,"
                                                 "owner_email TEXT NOT NULL,"
                                                 "owner_token TEXT NOT NULL UNIQUE"
                                                 ");";

    const char *sql[] = {create_advertisement_table_sql};

    for (int i = 0; i < (sizeof(sql) / sizeof(sql[0])); i++) {
        if (sqlite3_exec(*db, sql[i], 0, 0, &err_msg) != SQLITE_OK) {
            MG_ERROR(("Error creating db table: %s", err_msg));
            sqlite3_free(err_msg);
            return 1;
        }
    }

    return 0;
}

char *fetch_advertisements_html(sqlite3 *db) {
    const char *sql = "SELECT title, description FROM Advertisement ORDER BY created_at DESC;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return strdup("<ul></ul>");
    }
    
    size_t buffer_size = 1024;
    char *html = malloc(buffer_size);
    if (!html) return NULL;
    strcpy(html, "<ul>");
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *title = (const char *)sqlite3_column_text(stmt, 0);
        const char *description = (const char *)sqlite3_column_text(stmt, 1);
        
        size_t new_size = buffer_size + strlen(title) + strlen(description) + 50;
        html = realloc(html, new_size);
        if (!html) return NULL;
        
        strcat(html, "<li>");
        strcat(html, title);
        strcat(html, " - ");
        strcat(html, description);
        strcat(html, "</li>");
        
        buffer_size = new_size;
    }
    
    strcat(html, "</ul>");
    sqlite3_finalize(stmt);
    return html;
}

static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    struct HandlerData* handler_data = (struct HandlerData*)c->fn_data;
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        if (mg_match(hm->uri, mg_str("/"), NULL)) {

            char *index_page = (char *)malloc(sizeof(handler_data->pages->index_page));
            strcpy(index_page, handler_data->pages->index_page);
            char *ads_html = fetch_advertisements_html(handler_data->db);
            
            char *pos = strstr(index_page, "{{ ads }}");
            if (pos) {
                size_t new_size = strlen(index_page) - strlen("{{ ads }}") + strlen(ads_html) + 1;
                char *new_page = malloc(new_size);
                if (new_page) {
                    strncpy(new_page, index_page, pos - index_page);
                    new_page[pos - index_page] = '\0';
                    strcat(new_page, ads_html);
                    strcat(new_page, pos + strlen("{{ ads }}"));
                    free(index_page);
                    index_page = new_page;
                }
            }
            
            mg_http_reply(c, 200, "", "%s\n", index_page, time(NULL));
            free(index_page);
            free(ads_html);

        } else if (mg_match(hm->uri, mg_str("/manage-ad"), NULL)) {
            mg_http_reply(c, 200, "", "%s\n", handler_data->pages->manage_ad_page, time(NULL));
        } else {
            mg_http_reply(c, 500, "", "{%m:%m}\n", MG_ESC("error"), MG_ESC("Unsupported URI"));
        }
    }
}

int main(void) {
    sqlite3 *db;

    if (sqlite3_open("kryptori.db", &db) == 0) {
        MG_INFO(("Opened database successfully"));
    } else {
        MG_ERROR(("Can't open database: %s", sqlite3_errmsg(db)));
        return 1;
    }

    if (create_tables_if_not_exist(&db) == 0) {
        MG_INFO(("Created db tables if they didn't exist"));
    } else {
        return 1;
    }

    int index_page_fd = open("pages/index.html", O_RDONLY);
    int index_page_len = lseek(index_page_fd, 0, SEEK_END);
    void *index_page = mmap(0, index_page_len, PROT_READ, MAP_PRIVATE, index_page_fd, 0);
    int manage_ad_page_fd = open("pages/manage_ad.html", O_RDONLY);
    int manage_ad_page_len = lseek(manage_ad_page_fd, 0, SEEK_END);
    void *manage_ad_page = mmap(0, manage_ad_page_len, PROT_READ, MAP_PRIVATE, manage_ad_page_fd, 0);
    struct Pages pages = { .index_page = (char*)index_page, .manage_ad_page = (char*)manage_ad_page };

    struct HandlerData handler_data = { .db = db, .pages = &pages };
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8000", ev_handler, &handler_data);
    MG_INFO(("Listening on http://0.0.0.0:8000"));
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    sqlite3_close(db);
    return 0;
}
