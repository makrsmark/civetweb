/*
* Copyright (c) 2013-2015 the CivetWeb developers
* Copyright (c) 2013 No Face Press, LLC
* License http://opensource.org/licenses/mit-license.php MIT License
*/

/* Simple example program on how to use CivetWeb embedded into a C program. */
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "civetweb.h"


#define DOCUMENT_ROOT "."
#ifdef NO_SSL
#ifdef USE_IPV6
#define PORT "[::]:8888"
#else
#define PORT "8888"
#endif
#else
#ifdef USE_IPV6
#define PORT "[::]:8888r,[::]:8843s,8884"
#else
#define PORT "8888r,8843s"
#endif
#endif
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"
int exitNow = 0;


int
ExampleHandler(struct mg_connection *conn, void *cbdata)
{
	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	mg_printf(conn, "<html><body>");
	mg_printf(conn, "<h2>This is an example text from a C handler</h2>");
	mg_printf(
	    conn,
	    "<p>To see a page from the A handler <a href=\"A\">click A</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the A handler <a href=\"A/A\">click "
	          "A/A</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the A/B handler <a "
	          "href=\"A/B\">click A/B</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the B handler (0) <a "
	          "href=\"B\">click B</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the B handler (1) <a "
	          "href=\"B/A\">click B/A</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the B handler (2) <a "
	          "href=\"B/B\">click B/B</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the *.foo handler <a "
	          "href=\"xy.foo\">click xy.foo</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the FileHandler handler <a "
	          "href=\"form\">click form</a> (this is the form test page)</p>");
#ifdef USE_WEBSOCKET
	mg_printf(conn,
	          "<p>To test websocket handler <a href=\"/websocket\">click "
	          "websocket</a></p>");
#endif
	mg_printf(conn, "<p>To exit <a href=\"%s\">click exit</a></p>", EXIT_URI);
	mg_printf(conn, "</body></html>\n");
	return 1;
}


int
ExitHandler(struct mg_connection *conn, void *cbdata)
{
	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
	mg_printf(conn, "Server will shut down.\n");
	mg_printf(conn, "Bye!\n");
	exitNow = 1;
	return 1;
}


int
AHandler(struct mg_connection *conn, void *cbdata)
{
	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	mg_printf(conn, "<html><body>");
	mg_printf(conn, "<h2>This is the A handler!!!</h2>");
	mg_printf(conn, "</body></html>\n");
	return 1;
}


int
ABHandler(struct mg_connection *conn, void *cbdata)
{
	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	mg_printf(conn, "<html><body>");
	mg_printf(conn, "<h2>This is the AB handler!!!</h2>");
	mg_printf(conn, "</body></html>\n");
	return 1;
}


int
BXHandler(struct mg_connection *conn, void *cbdata)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);

	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	mg_printf(conn, "<html><body>");
	mg_printf(conn, "<h2>This is the BX handler %p!!!</h2>", cbdata);
	mg_printf(conn, "<p>The actual uri is %s</p>", req_info->uri);
	mg_printf(conn, "</body></html>\n");
	return 1;
}


int
FooHandler(struct mg_connection *conn, void *cbdata)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);

	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	mg_printf(conn, "<html><body>");
	mg_printf(conn, "<h2>This is the Foo handler!!!</h2>");
	mg_printf(conn,
	          "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>",
	          req_info->request_method,
	          req_info->uri,
	          req_info->http_version);
	mg_printf(conn, "</body></html>\n");
	return 1;
}


int
FileHandler(struct mg_connection *conn, void *cbdata)
{
	/* In this handler, we ignore the req_info and send the file "fileName". */
	const char *fileName = (const char *)cbdata;

	mg_send_file(conn, fileName);
	return 1;
}


/**********************/
/* proposed interface */

enum {
	FORM_DISPOSITION_SKIP = 0x0,
	FORM_DISPOSITION_GET = 0x1,
	FORM_DISPOSITION_STORE = 0x2,
	/*	FORM_DISPOSITION_READ = 0x3, not in the first step */
	FORM_DISPOSITION_ABORT = 0x10
};


struct mg_form_data_handler {
	int (*field_found)(const char *key,
	                   const char *filename,
	                   char *path,
	                   size_t pathlen,
	                   void *user_data);
	int (*field_get)(const char *key,
	                 const char *filename,
	                 const char *value,
	                 size_t valuelen,
	                 void *user_data);
	void *user_data;
};

int mg_handle_form_data(struct mg_connection *conn,
                        struct mg_form_data_handler *fdh);

/* end of interface */
/********************/


int
field_found(const char *key,
            const char *filename,
            char *path,
            size_t pathlen,
            void *user_data)
{
	struct mg_connection *conn = (struct mg_connection *)user_data;

	mg_printf(conn, "%s:\r\n", key);

	if (filename && *filename) {
		_snprintf(path, pathlen, "C:\\tmp\\%s", filename);
		return FORM_DISPOSITION_STORE;
	}
	return FORM_DISPOSITION_GET;
}


int
field_get(const char *key,
          const char *filename,
          const char *value,
          size_t valuelen,
          void *user_data)
{
	struct mg_connection *conn = (struct mg_connection *)user_data;

	mg_printf(conn, "%s = ", key);
	mg_write(conn, value, valuelen);
	mg_printf(conn, "\r\n\r\n");

	return 0;
}


int
FormHandler(struct mg_connection *conn, void *cbdata)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	int ret;
	struct mg_form_data_handler fdh = {field_found, field_get, 0};

	/* TODO: Checks before calling handle_form_data ? */
	(void)req_info;

	mg_printf(conn, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n");
	fdh.user_data = (void *)conn;

	/* TODO: Handle the return value */
	ret = mg_handle_form_data(conn, &fdh);

	return 1;
}


int
WebSocketStartHandler(struct mg_connection *conn, void *cbdata)
{
	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");

	mg_printf(conn, "<!DOCTYPE html>\n");
	mg_printf(conn, "<html>\n<head>\n");
	mg_printf(conn, "<meta charset=\"UTF-8\">\n");
	mg_printf(conn, "<title>Embedded websocket example</title>\n");

#ifdef USE_WEBSOCKET
	/* mg_printf(conn, "<script type=\"text/javascript\"><![CDATA[\n"); ...
	 * xhtml style */
	mg_printf(conn, "<script>\n");
	mg_printf(
	    conn,
	    "function load() {\n"
	    "  var wsproto = (location.protocol === 'https:') ? 'wss:' : 'ws:';\n"
	    "  connection = new WebSocket(wsproto + '//' + window.location.host + "
	    "'/websocket');\n"
	    "  websock_text_field = "
	    "document.getElementById('websock_text_field');\n"
	    "  connection.onmessage = function (e) {\n"
	    "    websock_text_field.innerHTML=e.data;\n"
	    "  }\n"
	    "  connection.onerror = function (error) {\n"
	    "    alert('WebSocket error');\n"
	    "    connection.close();\n"
	    "  }\n"
	    "}\n");
	/* mg_printf(conn, "]]></script>\n"); ... xhtml style */
	mg_printf(conn, "</script>\n");
	mg_printf(conn, "</head>\n<body onload=\"load()\">\n");
	mg_printf(
	    conn,
	    "<div id='websock_text_field'>No websocket connection yet</div>\n");
#else
	mg_printf(conn, "</head>\n<body>\n");
	mg_printf(conn, "Example not compiled with USE_WEBSOCKET\n");
#endif
	mg_printf(conn, "</body>\n</html>\n");

	return 1;
}


#ifdef USE_WEBSOCKET

/* MAX_WS_CLIENTS defines how many clients can connect to a websocket at the
 * same time. The value 5 is very small and used here only for demonstration;
 * it can be easily tested to connect more than MAX_WS_CLIENTS clients.
 * A real server should use a much higher number, or better use a dynamic list
 * of currently connected websocket clients. */
#define MAX_WS_CLIENTS (5)

struct t_ws_client {
	struct mg_connection *conn;
	int state;
} static ws_clients[MAX_WS_CLIENTS];


#define ASSERT(x)                                                              \
	{                                                                          \
		if (!(x)) {                                                            \
			fprintf(stderr,                                                    \
			        "Assertion failed in line %u\n",                           \
			        (unsigned)__LINE__);                                       \
		}                                                                      \
	}


int
WebSocketConnectHandler(const struct mg_connection *conn, void *cbdata)
{
	struct mg_context *ctx = mg_get_context(conn);
	int reject = 1;
	int i;

	mg_lock_context(ctx);
	for (i = 0; i < MAX_WS_CLIENTS; i++) {
		if (ws_clients[i].conn == NULL) {
			ws_clients[i].conn = (struct mg_connection *)conn;
			ws_clients[i].state = 1;
			mg_set_user_connection_data(conn, (void *)(ws_clients + i));
			reject = 0;
			break;
		}
	}
	mg_unlock_context(ctx);

	fprintf(stdout,
	        "Websocket client %s\r\n\r\n",
	        (reject ? "rejected" : "accepted"));
	return reject;
}


void
WebSocketReadyHandler(struct mg_connection *conn, void *cbdata)
{
	const char *text = "Hello from the websocket ready handler";
	struct t_ws_client *client = mg_get_user_connection_data(conn);

	mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, text, strlen(text));
	fprintf(stdout, "Greeting message sent to websocket client\r\n\r\n");
	ASSERT(client->conn == conn);
	ASSERT(client->state == 1);

	client->state = 2;
}


int
WebsocketDataHandler(struct mg_connection *conn,
                     int bits,
                     char *data,
                     size_t len,
                     void *cbdata)
{
	struct t_ws_client *client = mg_get_user_connection_data(conn);
	ASSERT(client->conn == conn);
	ASSERT(client->state >= 1);

	fprintf(stdout, "Websocket got data:\r\n");
	fwrite(data, len, 1, stdout);
	fprintf(stdout, "\r\n\r\n");

	return 1;
}


void
WebSocketCloseHandler(const struct mg_connection *conn, void *cbdata)
{
	struct mg_context *ctx = mg_get_context(conn);
	struct t_ws_client *client = mg_get_user_connection_data(conn);
	ASSERT(client->conn == conn);
	ASSERT(client->state >= 1);

	mg_lock_context(ctx);
	client->state = 0;
	client->conn = NULL;
	mg_unlock_context(ctx);

	fprintf(stdout,
	        "Client droped from the set of webserver connections\r\n\r\n");
}


void
InformWebsockets(struct mg_context *ctx)
{
	static unsigned long cnt = 0;
	char text[32];
	int i;

	sprintf(text, "%lu", ++cnt);

	mg_lock_context(ctx);
	for (i = 0; i < MAX_WS_CLIENTS; i++) {
		if (ws_clients[i].state == 2) {
			mg_websocket_write(ws_clients[i].conn,
			                   WEBSOCKET_OPCODE_TEXT,
			                   text,
			                   strlen(text));
		}
	}
	mg_unlock_context(ctx);
}
#endif


int
main(int argc, char *argv[])
{
	const char *options[] = {"document_root",
	                         DOCUMENT_ROOT,
	                         "listening_ports",
	                         PORT,
	                         "request_timeout_ms",
	                         "10000",
	                         "error_log_file",
	                         "error.log",
#ifdef USE_WEBSOCKET
	                         "websocket_timeout_ms",
	                         "3600000",
#endif
#ifndef NO_SSL
	                         "ssl_certificate",
	                         "../../resources/cert/server.pem",
#endif
	                         0};
	struct mg_callbacks callbacks;
	struct mg_context *ctx;
	struct mg_server_ports ports[32];
	int port_cnt, n;
	int err = 0;

/* Check if libcivetweb has been built with all required features. */
#ifdef USE_IPV6
	if (!mg_check_feature(8)) {
		fprintf(stderr,
		        "Error: Embedded example built with IPv6 support, "
		        "but civetweb library build without.\n");
		err = 1;
	}
#endif
#ifdef USE_WEBSOCKET
	if (!mg_check_feature(16)) {
		fprintf(stderr,
		        "Error: Embedded example built with websocket support, "
		        "but civetweb library build without.\n");
		err = 1;
	}
#endif
#ifndef NO_SSL
	if (!mg_check_feature(2)) {
		fprintf(stderr,
		        "Error: Embedded example built with SSL support, "
		        "but civetweb library build without.\n");
		err = 1;
	}
#endif
	if (err) {
		fprintf(stderr, "Cannot start CivetWeb - inconsistent build.\n");
		return EXIT_FAILURE;
	}

	/* Start CivetWeb web server */
	memset(&callbacks, 0, sizeof(callbacks));
	ctx = mg_start(&callbacks, 0, options);

	/* Add handler EXAMPLE_URI, to explain the example */
	mg_set_request_handler(ctx, EXAMPLE_URI, ExampleHandler, 0);
	mg_set_request_handler(ctx, EXIT_URI, ExitHandler, 0);

	/* Add handler for /A* and special handler for /A/B */
	mg_set_request_handler(ctx, "/A", AHandler, 0);
	mg_set_request_handler(ctx, "/A/B", ABHandler, 0);

	/* Add handler for /B, /B/A, /B/B but not for /B* */
	mg_set_request_handler(ctx, "/B$", BXHandler, (void *)0);
	mg_set_request_handler(ctx, "/B/A$", BXHandler, (void *)1);
	mg_set_request_handler(ctx, "/B/B$", BXHandler, (void *)2);

	/* Add handler for all files with .foo extention */
	mg_set_request_handler(ctx, "**.foo$", FooHandler, 0);

	/* Add handler for /form  (serve a file outside the document root) */
	mg_set_request_handler(ctx,
	                       "/form",
	                       FileHandler,
	                       (void *)"../../test/form.html");

	/* Add handler for form data */
	mg_set_request_handler(ctx,
	                       "/handle_form.embedded_c.example.callback",
	                       FormHandler,
	                       (void *)0);

	/* Add HTTP site to open a websocket connection */
	mg_set_request_handler(ctx, "/websocket", WebSocketStartHandler, 0);

#ifdef USE_WEBSOCKET
	/* WS site for the websocket connection */
	mg_set_websocket_handler(ctx,
	                         "/websocket",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         0);
#endif

	/* List all listening ports */
	memset(ports, 0, sizeof(ports));
	port_cnt = mg_get_server_ports(ctx, 32, ports);
	printf("\n%i listening ports:\n\n", port_cnt);

	for (n = 0; n < port_cnt && n < 32; n++) {
		const char *proto = ports[n].is_ssl ? "https" : "http";
		const char *host;

		if ((ports[n].protocol & 1) == 1) {
			/* IPv4 */
			host = "127.0.0.1";
			printf("Browse files at %s://%s:%i/\n", proto, host, ports[n].port);
			printf("Run example at %s://%s:%i%s\n",
			       proto,
			       host,
			       ports[n].port,
			       EXAMPLE_URI);
			printf(
			    "Exit at %s://%s:%i%s\n", proto, host, ports[n].port, EXIT_URI);
			printf("\n");
		}

		if ((ports[n].protocol & 2) == 2) {
			/* IPv6 */
			host = "[::1]";
			printf("Browse files at %s://%s:%i/\n", proto, host, ports[n].port);
			printf("Run example at %s://%s:%i%s\n",
			       proto,
			       host,
			       ports[n].port,
			       EXAMPLE_URI);
			printf(
			    "Exit at %s://%s:%i%s\n", proto, host, ports[n].port, EXIT_URI);
			printf("\n");
		}
	}

	/* Wait until the server should be closed */
	while (!exitNow) {
#ifdef _WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
#ifdef USE_WEBSOCKET
		InformWebsockets(ctx);
#endif
	}

	/* Stop the server */
	mg_stop(ctx);
	printf("Server stopped.\n");
	printf("Bye!\n");

	return EXIT_SUCCESS;
}
