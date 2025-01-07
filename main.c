#include "deps/mongoose.h"
#include "deps/sqlite3.h"

static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        if (mg_match(hm->uri, mg_str("/api/time/get"), NULL)) {
            mg_http_reply(c, 200, "", "{%m:%lu}\n", MG_ESC("time"), time(NULL));
        } else {
            mg_http_reply(c, 500, "", "{%m:%m}\n", MG_ESC("error"), MG_ESC("Unsupported URI"));
        }
    }
}

int create_tables_if_not_exist(sqlite3 **db) {
    char *err_msg = NULL;
    const char *create_advertisement_table_sql = "CREATE TABLE IF NOT EXISTS Advertisement ("
                                                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                 "created_at TEXT NOT NULL,"
                                                 "updated_at TEXT NOT NULL,"
                                                 "title TEXT NOT NULL,"
                                                 "description TEXT,"
                                                 "is_standing_order INTEGER NOT NULL,"
                                                 "owner_token TEXT NOT NULL,"
                                                 "FOREIGN KEY(owner_token) REFERENCES User(token)"
                                                 ");";

    const char *create_user_table_sql = "CREATE TABLE IF NOT EXISTS User ("
                                        "token TEXT PRIMARY KEY,"
                                        "last_interacted_at TEXT"
                                        ");";

    const char *create_chat_table_sql = "CREATE TABLE IF NOT EXISTS Chat ("
                                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                        "advertisement_id INTEGER,"
                                        "owner_token TEXT,"
                                        "seeker_token TEXT,"
                                        "FOREIGN KEY(advertisement_id) REFERENCES Advertisement(id),"
                                        "FOREIGN KEY(owner_token) REFERENCES User(token),"
                                        "FOREIGN KEY(seeker_token) REFERENCES User(token)"
                                        ");";

    const char *create_message_table_sql = "CREATE TABLE IF NOT EXISTS Message ("
                                           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                           "sent_at TEXT NOT NULL,"
                                           "read_at TEXT,"
                                           "text TEXT NOT NULL,"
                                           "sender_token TEXT NOT NULL,"
                                           "chat_id INTEGER,"
                                           "FOREIGN KEY(sender_token) REFERENCES User(token),"
                                           "FOREIGN KEY(chat_id) REFERENCES Chat(id)"
                                           ");";

    const char *create_advertisement_chats_table_sql = "CREATE TABLE IF NOT EXISTS AdvertisementChats ("
                                                       "advertisement_id INTEGER,"
                                                       "chat_id INTEGER,"
                                                       "FOREIGN KEY(advertisement_id) REFERENCES Advertisement(id),"
                                                       "FOREIGN KEY(chat_id) REFERENCES Chat(id),"
                                                       "PRIMARY KEY(advertisement_id, chat_id)"
                                                       ");";

    const char *create_user_chats_table_sql = "CREATE TABLE IF NOT EXISTS UserChats ("
                                              "user_token TEXT,"
                                              "chat_id INTEGER,"
                                              "FOREIGN KEY(user_token) REFERENCES User(token),"
                                              "FOREIGN KEY(chat_id) REFERENCES Chat(id),"
                                              "PRIMARY KEY(user_token, chat_id)"
                                              ");";

    const char *sql[] = {create_advertisement_table_sql,
                         create_user_table_sql,
                         create_chat_table_sql,
                         create_message_table_sql,
                         create_advertisement_chats_table_sql,
                         create_user_chats_table_sql};

    for (int i = 0; i < (sizeof(sql) / sizeof(sql[0])); i++) {
        if (sqlite3_exec(*db, sql[i], 0, 0, &err_msg) != SQLITE_OK) {
            MG_ERROR(("Error creating db table: %s", err_msg));
            sqlite3_free(err_msg);
            return 1;
        }
    }

    return 0;
}

int main(void) {
    sqlite3 *db;
    int rc;

    rc = sqlite3_open("kryptori.db", &db);

    if (rc) {
        MG_ERROR(("Can't open database: %s", sqlite3_errmsg(db)));
        return 1;
    } else {
        MG_INFO(("Opened database successfully"));
    }

    create_tables_if_not_exist(&db);

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8000", ev_handler, NULL);
    MG_INFO(("Listening on http://0.0.0.0:8000"));
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    sqlite3_close(db);
    return 0;
}
