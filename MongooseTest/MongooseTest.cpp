

#include "mongoose/mongoose.h"  


/*
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


*/


/*

//Websocket Tutorial
// https://mongoose.ws/documentation/tutorials/websocket/websocket-server/



// Copyright (c) 2020 Cesanta Software Limited
// All rights reserved
//
// Example Websocket server. See https://mongoose.ws/tutorials/websocket-server/

#include "mongoose/mongoose.h"

static const char* s_listen_on = "ws://localhost:8100";
static const char* s_web_root = ".";
static const char* s_ca_path = "ca.pem";
static const char* s_cert_path = "cert.pem";
static const char* s_key_path = "key.pem";
struct mg_str s_ca, s_cert, s_key;




static void timer_fn(void* arg)
{
    puts("In timer");
    struct mg_mgr* mgr = (struct mg_mgr*)arg;
    // Broadcast "hi" message to all connected websocket clients.
    // Traverse over all connections
    for (struct mg_connection* c = mgr->conns; c != NULL; c = c->next)
    {
        puts("Found conn");
        if (c->data[0] == 'W') 
            mg_ws_send(c, "hi", 2, WEBSOCKET_OP_TEXT);
    }
}



// This RESTful server implements the following endpoints:
//   /websocket - upgrade to Websocket, and implement websocket echo server
//   /rest - respond with JSON string {"result": 123}
//   any other URI serves static files from s_web_root
static void fn(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_OPEN) {
        // c->is_hexdumping = 1;
    }
    else if (c->is_tls && ev == MG_EV_ACCEPT) {
        s_ca = mg_file_read(&mg_fs_posix, s_ca_path);
        s_cert = mg_file_read(&mg_fs_posix, s_cert_path);
        s_key = mg_file_read(&mg_fs_posix, s_key_path);
        struct mg_tls_opts opts = { .ca = s_ca, .cert = s_cert, .key = s_key };
        mg_tls_init(c, &opts);
    }
    else if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        if (mg_match(hm->uri, mg_str("/websocket"), NULL)) {
            // Upgrade to websocket. From now on, connection is full-duplex
            // Websocket connection, which will receive MG_EV_WS_MSG events.
            mg_ws_upgrade(c, hm, NULL);
        }
        else if (mg_match(hm->uri, mg_str("/rest"), NULL)) {
            // Serve REST response
            mg_http_reply(c, 200, "", "{\"result\": %d}\n", 123);
        }
        else {
            // Serve static files
            struct mg_http_serve_opts opts = { .root_dir = s_web_root };
            mg_http_serve_dir(c, (mg_http_message*)ev_data, &opts);
        }
    }
    else if (ev == MG_EV_WS_MSG) {
        // Got websocket frame. Received data is wm->data. Echo it back!
        struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
        mg_ws_send(c, wm->data.buf, wm->data.len, WEBSOCKET_OP_TEXT);
    }
    else if (ev == MG_EV_CLOSE && c->is_tls) {
        mg_free(s_ca.buf);
        mg_free(s_cert.buf);
        mg_free(s_key.buf);
    }
}

int main(int argc, char* argv[]) {
    struct mg_mgr mgr;  // Event manager
    int i;

    // Parse command-line flags
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-url") == 0 && argv[i + 1] != NULL) {
            s_listen_on = argv[++i];
        }
        else if (strcmp(argv[i], "-ca") == 0 && argv[i + 1] != NULL) {
            s_ca_path = argv[++i];
        }
        else if (strcmp(argv[i], "-cert") == 0 && argv[i + 1] != NULL) {
            s_cert_path = argv[++i];
        }
        else if (strcmp(argv[i], "-key") == 0 && argv[i + 1] != NULL) {
            s_key_path = argv[++i];
        }
        else {
            printf(
                "Usage: %s OPTIONS\n"
                "  -ca PATH  - Path to the CA file, default: '%s'\n"
                "  -cert PATH  - Path to the CERT file, default: '%s'\n"
                "  -key PATH  - Path to the KEY file, default: '%s'\n"
                "  -url URL  - Listen on URL, default: '%s'\n",
                argv[0], s_ca_path, s_cert_path, s_key_path, s_listen_on);
            return 1;
        }
    }

    mg_mgr_init(&mgr);  // Initialise event manager
    printf("Starting WS listener on %s/websocket\n", s_listen_on);
    mg_http_listen(&mgr, s_listen_on, fn, NULL);  // Create HTTP listener
    mg_timer_add(&mgr, 1000, MG_TIMER_REPEAT, timer_fn, &mgr);
    for (;;) mg_mgr_poll(&mgr, 1000);             // Infinite event loop
    mg_mgr_free(&mgr);
    return 0;
}

*/

//webUI push
//https://mongoose.ws/documentation/tutorials/webui/webui-push/



// Copyright (c) 2020 Cesanta Software Limited
// All rights reserved
//
// Example Websocket server with timers. This is a simple Websocket echo
// server, which sends a message to all connected clients periodically,
// using timer API.

#include "mongoose/mongoose.h"

static const char* s_listen_on = "http://localhost:8400";
static const char* s_web_root = "./web_root/";

static void fn(struct mg_connection* c, int ev, void* ev_data) 
//static void fn(struct mg_connection* c, int ev, void* ev_data, void* fn_data) 
{
    if (ev == MG_EV_HTTP_MSG) 
    {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        if (mg_match(hm->uri, mg_str("/api/watch"), NULL)) { 
        //if (mg_http_match_uri(hm, "/api/watch")) {

            mg_ws_upgrade(c, hm, NULL);  // Upgrade HTTP to Websocket
            c->data[0] = 'W';           // Set some unique mark on the connection
        }
        else {
            struct mg_http_serve_opts opts;
            opts.root_dir = "./web_root/";
            opts.ssi_pattern = 0;    // SSI file name pattern, e.g. #.shtml
            opts.extra_headers = 0;  // Extra HTTP headers to add in responses
            opts.mime_types = 0;     // Extra mime types, ext1=type1,ext2=type2,..
            opts.page404 = 0;        // Path to the 404 page, or NULL by default
            opts.fs = 0;
            mg_http_serve_dir(c, (mg_http_message*)ev_data, &opts);
        }
    }

    //(void) fn_data;
}


// Push to all watchers
static void push(struct mg_mgr* mgr, const char* name, const void* data) {
    struct mg_connection* c;
    for (c = mgr->conns; c != NULL; c = c->next) {
        if (c->data[0] != 'W') continue;
        mg_ws_printf(c, WEBSOCKET_OP_TEXT, "{%m:%m,%m:%m}", MG_ESC("name"), MG_ESC(name), MG_ESC("data"), MG_ESC(data));
    }
}



static void timer_fn(void* arg) {
    struct mg_mgr* mgr = (struct mg_mgr*)arg;
    char buf[50];
    mg_snprintf(buf, sizeof(buf), "[%lu, %d]", (unsigned long)time(NULL),10 + (int)((double)rand() * 10 / RAND_MAX));
    push(mgr, "metrics", buf);
}

int main(void) {
    struct mg_mgr mgr;  // Event manager
    mg_mgr_init(&mgr);  // Initialise event manager
    //mg_log_set("2");    // Set debug log level
    mg_timer_add(&mgr, 2000, MG_TIMER_REPEAT, timer_fn, &mgr);
    mg_http_listen(&mgr, s_listen_on, fn, NULL);  // Create HTTP listener
    for (;;) mg_mgr_poll(&mgr, 500);              // Infinite event loop
    mg_mgr_free(&mgr);                            // Free manager resources
    return 0;
}
