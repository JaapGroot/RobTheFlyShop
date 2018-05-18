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
	size_t		len;
	struct kore_buf	*buff;
	u_int8_t	*data;

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
	//if this page was loaded with a post request
	if(req->method == HTTP_METHOD_POST){
	//check if the entered credentials werent wrong
	http_populate_post(req);
	//TODO: FIND OUT WHERE I CAN FIND THE POST VARIABLES AND SEE IF THEY ARENT SCRAPPED BY THE VALIDATOR
	//TODO: GIVE WARNING THAT INVALID PARAMETERS WERE ENTERED
	//also, maybe stick this in a different function in a different file

	//TODO: ADD LOGIN LOGIC HERE!
	
	//see if loging in was a success,
	if(success){
		//serve the logged in page
	serve_page(req, asset_logedin_html, asset_len_logedin_html);
	}else{
		//TODO: ADD a warning to the content, and serve the login page again
		serve_page(req, asset_login_html, asset_len_login_html);
	}


	}else{
		//if this was not a post request, just serve the page
	//to serve static content simply call serve page with the static content
	serve_page(req, asset_login_html, asset_len_login_html);
	}
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
