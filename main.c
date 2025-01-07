#include "deps/mongoose.h"
#include "deps/sqlite3.h"

static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_match(hm->uri, mg_str("/api/time/get"), NULL)) {
            mg_http_reply(c, 200, "", "{%m:%lu}\n", MG_ESC("time"), time(NULL));
        } else {
            mg_http_reply(c, 500, "", "{%m:%m}\n", MG_ESC("error"), MG_ESC("Unsupported URI")); 
        }
    }
}

int main(void) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("kryptori.db", &db);

    if(rc) {
        MG_ERROR(("Can't open database: %s", sqlite3_errmsg(db)));
        return(0);
    } else {
        MG_INFO(("Opened database successfully"));
    }

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

