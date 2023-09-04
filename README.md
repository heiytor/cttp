<h1 align="center">CTTP: C HTTP Library with Go-like Syntax</h1>

CTTP is designed to be fast and straightforward. Inspired by the simplicity and elegance of Golang's HTTP package, it offers an intuitive way to establish web servers in C without external dependencies. With CTTP, you can efficiently read and write headers, handle request parameters, and manage request/response bodies. 

<h1 align="center">Examples</h1>

### Getting Started
Set up a simple server with just one route:

```c
#include "cttp.h"
#include "cttp-status.h"

int get_user(CTTP_Writer *w, CTTP_Request *r)
{
	CTTP_write_header(w, "Content-Type", "application/json");
	char *user_id = CTTP_read_request_header(r, "X-User-Id");
	
	if (!user_id)
	{
		char message[] = "{\"error\": \"not found\"}";
		CTTP_write_status(w, CTTP_STATUS_NOT_FOUND);
		CTTP_write_body(w, message, strlen(message));
		return CTTP_ERROR_NIL;
	}
	
	char message[] = "{\"user\": \"some info here\"}";
	CTTP_write_status(w, CTTP_STATUS_OK);
	CTTP_write_body(w, message, strlen(message));

	return CTTP_ERROR_NIL;
}

int main()
{
	CTTP_Server cs = CTTP_new_server(8080);
	CTTP_add_route(&cs, "GET", "/user", get_user);
	CTTP_start_server(&cs);
}
```

### Setting Routes:
Define routes by providing a function that accepts pointers to a writer and request structure and return an integer. This value informs the server about the result of the request. If any arbitrary integer is returned, the server assumes successful request processing. However, for certain specific error values, the server responds with a default error message. To avoid this, use `CTTP_ERROR_NIL` value or simply always return `n > 0`.

```c
int get_user(CTTP_Writer *w, CTTP_Request *r)
{
	CTTP_write_header(w, "Content-Type", "application/json");
	char *user_id = CTTP_read_request_header(r, "X-User-Id");
	
	if (!user_id)
	{
		char message[] = "{\"error\": \"not found\"}";
		CTTP_write_status(w, CTTP_STATUS_NOT_FOUND);
		CTTP_write_body(w, message, strlen(message));
		return 1;
	}
	
	char message[] = "{\"user\": \"some info here\"}";
	CTTP_write_status(w, CTTP_STATUS_OK);
	CTTP_write_body(w, message, strlen(message));

	return CTTP_ERROR_NIL;
}
```

### Sending Default Errors:
Given the C language's constraints on built-in error handling, CTTP uses an error tracking system. Typically, these errors are addressed automatically, however, sometimes you may need to use it manually. For example, you can send a standard internal server error:

```c
// ...
char *user_id = CTTP_read_request_header(r, "X-User-Id");
if (!user_id) return CTTP_ERROR_INTERNAL_SERVER_ERROR;
// ...
```

The internal functions will automatically handle Not Found, Method Not Allowed (and write the `Allow` header), Request Header Too Large and Content Too Large errors.

### Retrieving Parameters and Headers:

```c
char *sort = CTTP_read_request_param(r, "sort-by");
if (sort == NULL)
{
	// ...
}

char *id = CTTP_read_request_header(r, "X-User-Id");
if (sort == NULL)
{
	// ...
}
```

You can also read a writer header with `CTTP_read_writer_header`.

### Crafting a Response:
Construct responses employing functions like `CTTP_write_header`, `CTTP_write_status`, and `CTTP_write_buffer`. For example:

```c
char res_body[] = "Hello World!";

CTTP_write_header("Content-Type", "text/plain", w);
CTTP_write_status(CTTP_STATUS_OK, w);
CTTP_write_buffer(res_body, strlen(res_body), w);
```

<h1 align="center">Features</h1>

- [x] Handle unregistered routes.
- [x] Manage requests to unimplemented route methods and automatically set the `Allow` header.
- [x] Handle unaddressed errors automatically.
- [x] Process oversized headers or content seamlessly.
- [ ] Middleware integration and management.
- [ ] Concurrency, parallelism and non-blocking I/O.
- [ ] Dynamic routes with regex matching.

<h1 align="center">License</h1>

CTTP is licensed under *[MIT License](https://github.com/heiytor/cttp/blob/main/LICENSE)*. 
