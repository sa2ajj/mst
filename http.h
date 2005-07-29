#ifndef _HAVE_HTTP_HEADER
#define _HAVE_HTTP_HEADER

typedef struct HttpResponse     HttpResponse;
typedef struct HttpRequest      HttpRequest;
typedef struct HttpRequestParam HttpRequestParam;

HttpRequest *http_server(const char *name, int iPort);

HttpRequest *http_request_new(void);
const char *http_request_get_method(HttpRequest *req);
const char *http_request_param_get(HttpRequest *req, const char *key);
const char *http_request_get_uri(HttpRequest *req);
const char *http_request_get_path_info(HttpRequest *req);
const char *http_request_get_query_string(HttpRequest *req);

HttpResponse *http_response_new(HttpRequest *req);

void http_response_printf(HttpResponse *res, const char *format, ...);
void http_response_printf_alloc_buffer(HttpResponse *res, int bytes);
void http_response_set_content_type(HttpResponse *res, const char *type);
void http_response_set_status(HttpResponse *res, int status_code, const char *status_desc);
void http_response_set_data(HttpResponse *res, void *data, int data_len);
void http_response_write(HttpResponse *res, const void *data, size_t data_len);
void http_response_append_header(HttpResponse *res, const char *header);
void http_response_send_headers(HttpResponse *res);
void http_response_send(HttpResponse *res);


#endif
