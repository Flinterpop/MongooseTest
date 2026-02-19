

#include "mongoose/mongoose.h"  



// HTTP server event handler function
void event_handler1(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_HTTP_MSG) 
    {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        struct mg_http_serve_opts opts; //= { .root_dir = "./web_root/" };
        opts.root_dir = "./web_root/";
        opts.ssi_pattern=0;    // SSI file name pattern, e.g. #.shtml
        opts.extra_headers=0;  // Extra HTTP headers to add in responses
        opts.mime_types=0;     // Extra mime types, ext1=type1,ext2=type2,..
        opts.page404=0;        // Path to the 404 page, or NULL by default
        opts.fs=0;

        mg_http_serve_dir(c, hm, &opts);
    }
}


static int pinValue = 45;
static void event_handler2(struct mg_connection* thisConnection, int ev, void* ev_data) {

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        if (mg_match(hm->uri, mg_str("/api/led/get"), NULL)) 
        {
            mg_http_reply(thisConnection, 200, "", "%d\n", pinValue);
        }
        else if (mg_match(hm->uri, mg_str("/api/led/toggle"), NULL)) 
        {
            pinValue++;
            mg_http_reply(thisConnection, 200, "", "true\n");
        }
        else if (mg_match(hm->uri, mg_str("/api/sum"), NULL)) {
            double num1 = 0.0, num2 = 0.0;
            mg_json_get_num(hm->body, "$[0]", &num1);  // Extract first number
            mg_json_get_num(hm->body, "$[1]", &num2);  // Extract second number
            mg_http_reply(thisConnection, 200, "Content-Type: application/json\r\n",
                "{%m:%g}\n", MG_ESC("result"), num1 + num2);
        }
        else 
        {
            struct mg_http_serve_opts opts; //= { .root_dir = "./web_root/" };
            opts.root_dir = "./socket_root/";
            opts.ssi_pattern = 0;    // SSI file name pattern, e.g. #.shtml
            opts.extra_headers = 0;  // Extra HTTP headers to add in responses
            opts.mime_types = 0;     // Extra mime types, ext1=type1,ext2=type2,..
            opts.page404 = 0;        // Path to the 404 page, or NULL by default
            opts.fs = 0;
            mg_http_serve_dir(thisConnection, hm, &opts);
        }
    }
}


static void event_handler3(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_HTTP_MSG)
    {  // New HTTP request received
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;  // Parsed HTTP request
        if (mg_match(hm->uri, mg_str("/api/hello"), NULL))
        {              // REST API call?
            mg_http_reply(c, 200, "", "{%m:%d}\n", MG_ESC("status"), 1);    // Yes. Respond JSON
        }
        else
        {
            struct mg_http_serve_opts opts = { .root_dir = ".", .fs = &mg_fs_posix };
            mg_http_serve_dir(c, hm, &opts);  // For all other URLs, Serve static files
        }
    }
}



static void timer_fn(void* arg)
{

    pinValue += 2;
    //mg_http_reply(c, 200, "", "%d\n", pinValue); 


    return;
    //std::string r = UpdateSerialSummary();

    //AISSummary();

    //puts(r.c_str());


    struct mg_mgr* mgr = (struct mg_mgr*)arg;
    // Broadcast "hi" message to all connected websocket clients.
    // Traverse over all connections
    for (struct mg_connection* c = mgr->conns; c != NULL; c = c->next)
    {
        //puts("Here");
        // Send only to marked connections
        if (c->data[0] == 'W') mg_ws_send(c, "hi", 2, WEBSOCKET_OP_TEXT);
    }
}





int main(void) {
    struct mg_mgr mgr;  // Declare event manager
    mg_mgr_init(&mgr);  // Initialise event manager
    mg_log_set(MG_LL_DEBUG);  // Set log level to debug

    mg_http_listen(&mgr, "http://0.0.0.0:8090", event_handler1, NULL);  // Setup listener for HTML
    mg_http_listen(&mgr, "https://0.0.0.0:8443", event_handler2, NULL);  // Setup HTTPS listener
    mg_http_listen(&mgr, "http://0.0.0.0:8080", event_handler3, NULL);  // Setup listener for WebSocket +_ HTML
  

    mg_timer_add(&mgr, 1000, MG_TIMER_REPEAT, timer_fn, &mgr);

    for (;;) {          // Run an infinite event loop
        mg_mgr_poll(&mgr, 1000);
    }
    return 0;
}