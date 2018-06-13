#include "includes.h"


//function prototypes
//initialization
int init(int);

//serve full page
int serve_page(struct http_request *, u_int8_t *, size_t len);

//initializes stuff
int init(int state){
	// init logger
	openlog("RobTheFlyShop", 0, LOG_USER);
	//init database
	kore_pgsql_register("DB", "host=localhost user=pgadmin password=root dbname=rtfsdb");

	//remove the active sessions from the database
	struct kore_pgsql sql;
	//connect to db
	kore_pgsql_init(&sql);

	if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
		kore_pgsql_logerror(&sql);
	}
	//delete all sessions
	if(!kore_pgsql_query(&sql, "DELETE FROM session")){
		kore_pgsql_logerror(&sql);
	}
	//clean up the sql
	kore_pgsql_cleanup(&sql);
	
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
	//change content of sidebar based on user role
	//do quick database check for user role
	//if the role is admin, show admin sidebar,
	//if the role is user, show logout button,
	//else show login button because the user is not logedin
	kore_buf_replace_string(buff, "$sideoptions$", asset_adminoptions_html,
			asset_len_adminoptions_html);
	
	kore_buf_append(buff, content, content_length);
	kore_buf_append(buff, asset_DefaultFooter_html, asset_len_DefaultFooter_html);
	
	//serve the page to the user
	data = kore_buf_release(buff, &len);
	http_response(req, 200, data, len);

	//free the buffer for the full page
	kore_free(data);

	return(KORE_RESULT_OK);
}
