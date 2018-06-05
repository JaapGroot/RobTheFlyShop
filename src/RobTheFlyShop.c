#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include <stdio.h>
#include <stdlib.h>

#include "assets.h"

//function prototypes
//initialization
int init(int);

//serving pages
int		serve_index(struct http_request *);
int		serve_login(struct http_request *);
int		serve_logedin(struct http_request *);

//serve full page
int serve_page(struct http_request *, u_int8_t *, size_t len);

//initializes stuff
int init(int state){
	//init database
	//the connection string might be wrong... also make sure to turn on the database server
	kore_pgsql_register("DB", "host=localhost user=pgadmin password=root dbname=rtfsdb"); 
	return (KORE_RESULT_OK);
}

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
	u_int8_t success = 0;
	
	struct kore_buf		*b;
	u_int8_t		*d;
	size_t			len;
	char			*mail, *pass;
	int			UserId = 0;

	//first allocate the buffer
	b = kore_buf_alloc(0);

	if (req->method == HTTP_METHOD_GET){
		http_populate_get(req);
	}

	else if (req->method == HTTP_METHOD_POST){
		//populate and do regex validation on post request
		http_populate_post(req);

		//check if the entry was correct
		if(http_argument_get_string(req, "Email", &mail) && http_argument_get_string(req, "Password", &pass)){
			//TODO: add pass salting and stuff
			//variables are stored at *mail and *pass
			//
			//reserve some variables
			struct kore_pgsql sql;
			int rows;
			
			//init the database
			kore_pgsql_init(&sql);

			//try to connect to the database we called DB for synchronous database searches.
			if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
				//if we couln't connect log the error,
				//youll be resent to the login page as if nothing happened
				success = 0;
				kore_pgsql_logerror(&sql);
			}else{
			//if we did connect you'll be sent to the page that tells you youre logged in
				//build query
				char query[100];
				snprintf(query, sizeof(query), "SELECT * FROM users WHERE mail=\'%s\' AND password=\'%s\'", mail, pass); 

				kore_log(LOG_NOTICE, "%s", query);
				//preform the query
				if(!kore_pgsql_query(&sql, query)){
					kore_pgsql_logerror(&sql);
				}

				//if there were no results the mail or password were incorrect, if there are more then 1 multiple users were selected wich shouldn't happen, so there should only be result if the login is succesful
				rows = kore_pgsql_ntuples(&sql);
				kore_log(LOG_NOTICE, "rows: %i", rows);
				if(rows == 1){
					//set the user id from the database
					UserId = atoi(kore_pgsql_getvalue(&sql, 0, 0));
					success = 1;
				}
			}

			//interfacing with the database is done, clean up...
			kore_pgsql_cleanup(&sql);
		}
		if(!success){
			//else let the user know they did it wrong
			kore_buf_append(b, asset_loginwarning_html, asset_len_loginwarning_html);
			success = 0;
		}
	}
	
	//if login was successful
	if(success){
		//TODO: give a cookie to the user
	
		//the user id should be stored in UserId
		kore_log(LOG_NOTICE, "UID of user: %i", UserId);

		//show the user the logedin page
		kore_buf_append(b, asset_logedin_html, asset_len_logedin_html);
	}else{
		//seve the normal page again
		kore_buf_append(b, asset_login_html, asset_len_login_html);
	}
	
	//serve the page.
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
