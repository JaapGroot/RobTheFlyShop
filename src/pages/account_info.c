#include "includes.h"

//@description Back-end to serve the account info page
//Input the http request.
//output none
int serve_account_info(struct http_request *req) {
	char			*fName, *lName, *mail, *rMiles, query[150];
	struct kore_buf		*buf;
	u_int8_t		*data;
	size_t 			len;
	struct kore_pgsql	sql;
	int			rows, uID;

	//Clear all the vars of data.
	fName = NULL;
	lName = NULL;
	mail = NULL;
	rMiles = NULL;
	uID = NULL;

	//TODO Get the uID from the cookie now this is hardcoded for testing.
	
	uID = getUIDFromCookie(req);
//	kore_log(1,"The user ID is: %d", uID);	
	//uID = "3";

	buf = kore_buf_alloc(0);
	kore_pgsql_init(&sql);
	
	//Add the html to the buffer.
	if(uID == NULL) {
		kore_buf_append(buf, asset_infoPageFail_html, asset_len_infoPageFail_html);
		kore_log(LOG_ALERT, "Burp Rob, they are in!");
	}	
	else {
		kore_buf_append(buf, asset_infoPage_html, asset_len_infoPage_html);
	
		if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
			kore_pgsql_logerror(&sql);
		}			
		else{
			snprintf(query, sizeof(query), "SELECT * FROM users WHERE user_id = \'%d\'",uID);
			//Return on the cmd which query is executed.
			kore_log(LOG_NOTICE, "%s", query);
			//If the query failed, show a error.
			if(!kore_pgsql_query(&sql, query)){
				kore_pgsql_logerror(&sql);
			}
			//Else query succesfully executed. 
			else {	
				fName = kore_pgsql_getvalue(&sql, 0, SQL_USERS_FIRST_NAME);
				lName = kore_pgsql_getvalue(&sql, 0, SQL_USERS_LAST_NAME);
				mail = kore_pgsql_getvalue(&sql, 0, SQL_USERS_MAIL);
				rMiles = kore_pgsql_getvalue(&sql, 0, SQL_USERS_ROB_MILES);
				
				//Replace listenty with the new list, so it grows.
				kore_buf_replace_string(buf, "$fName$", fName, strlen(fName));
				kore_buf_replace_string(buf, "$lName$", lName, strlen(lName));
				kore_buf_replace_string(buf, "$mail$", mail, strlen(mail));
				kore_buf_replace_string(buf, "$rMiles$", rMiles, strlen(rMiles));
			}
		kore_pgsql_cleanup(&sql);
		}
	}


	data = kore_buf_release(buf, &len);
	serve_page(req, data, len);
	kore_free(data);
	return (KORE_RESULT_OK);
}
