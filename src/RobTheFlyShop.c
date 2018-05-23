#include <kore/kore.h>
#include <kore/http.h>

#include "assets.h"

//function prototypes
//serving pages
int		serve_index(struct http_request *);
int		serve_login(struct http_request *);
int		serve_logedin(struct http_request *);

//serve full page
int serve_page(struct http_request *, u_int8_t *, size_t len);

//actual functions
int
serve_index(struct http_request *req)
{
	//to serve a page from a buffer, like with more dynamic content do the following:
	//create a buffer
	size_t			len;
	struct kore_buf		*buff;
	u_int8_t		*data;
	
	//Init cookie variables and struct
	char			*cookie_value;
	//struct http_cookie	*cookie;

	//first cookie implementation
	http_populate_cookies(req);
	if(http_request_cookie(req, "Simple", &cookie_value))
		kore_log(LOG_DEBUG, "Got simple: %s", cookie_value);

	http_response_cookie(req, "Simple", "hello world", req->path, 0, 0, NULL);
	
	//Implement buffer for the index
	buff = kore_buf_alloc(0);

	//add the content you want to the buffer
	kore_buf_append(buff, asset_index_html, asset_len_index_html);

	//release the buffer, and get the length
	data = kore_buf_release(buff, &len);
	
	//call serve page to get a pretty header and footer
	serve_page(req ,data, len);

	//free the buffer
	kore_free(data);
	return (KORE_RESULT_OK);
}
	
int
serve_login(struct http_request *req)
{	
	u_int8_t success = 1;
	
	struct kore_buf		*b;
	u_int8_t		*d;
	size_t			len;
	int			r, i;
	char	*test, name[10];
	
	if (req->method == HTTP_METHOD_GET)
		http_populate_get(req);
	else if (req->method == HTTP_METHOD_POST)
		http_populate_post(req);

	
	b = kore_buf_alloc(asset_len_login_html);
	kore_buf_append(b, asset_login_html, asset_len_login_html);

	if (req->method == HTTP_METHOD_GET) {
		kore_buf_replace_string(b, "$mail$", NULL, 0);
		kore_buf_replace_string(b, "$password$", NULL, 0);
		//kore_buf_replace_string(b, "$test3$", NULL, 0);

		//if (http_argument_get_uint16(req, "id", &r))
		//	kore_log(LOG_NOTICE, "id: %d", r);
		//else
		//	kore_log(LOG_NOTICE, "No id set");

		http_response_header(req, "content-type", "text/html");
		d = kore_buf_release(b, &len);
		serve_page(req, d, len);
		kore_free(d);

		return (KORE_RESULT_OK);
	}	

	http_response_header(req, "content-type", "text/html");
	d = kore_buf_release(b, &len);
	serve_page(req, d, len);
	kore_free(d);
	
	return (KORE_RESULT_OK);
}

int serve_logedin(struct http_request *req){
	//to serve a page with static content, simply call serve page with the content you want
	serve_page(req, asset_logedin_html, asset_len_logedin_html);
	return (KORE_RESULT_OK);
}

//serve page
//this function takes a buffer containting the content of the page and sends an http_response for the full page
int serve_page(struct http_request *req, u_int8_t *content, size_t content_length){
	//create buffer for the full page
	size_t		len;
	struct kore_buf	*buff;
	u_int8_t	*data;

	buff = kore_buf_alloc(0);

	//add the header, content and footer to the page
	kore_buf_append(buff, asset_DefaultHeader_html, asset_len_DefaultHeader_html);
	kore_buf_append(buff, content, content_length);
	kore_buf_append(buff, asset_DefaultFooter_html, asset_len_DefaultFooter_html);
	
	//serve the page to the user
	data = kore_buf_release(buff, &len);
	http_response(req, 200, data, len);

	//free the buffer for the full page
	kore_free(data);

	return(KORE_RESULT_OK);
}
