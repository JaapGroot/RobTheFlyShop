#include "includes.h"

//@description Back-end to serve page to book flights
//@input the http request.
//@output nono
int serve_viewflight(struct http_request *req) {
	struct kore_buf		*buf;
	u_int8_t 		*data;
	struct kore_pgsql	sql;
	size_t			len;
	int			uID;
	char			*fID, *fLoc, *fDate, *fDes, *fPric, query[200];

	//clear the vars
	fID = NULL;		//Var for the flight id
	fLoc = NULL;		//Var for the flight location
	fDate = NULL;		//Var for the flight date
	fDes = NULL;		//Var for the flight destination
	fPric = NULL;		//Var for the flight price

	//Get uID from cookie	
	uID = getUIDFromCookie(req);

	//Alloc the buf and init the sql
	buf = kore_buf_alloc(0);
	kore_pgsql_init(&sql);
	
	//Add the html to the buffer.
	if(uID == NULL) {
		kore_buf_append(buf, asset_infoPageFail_html, asset_len_infoPageFail_html);
		own_log(LOG_ALERT, "User tried entering view_flight page with no user id");
	}	
	else {
		if(req->method == HTTP_METHOD_GET){
			http_populate_get(req);
			if(!http_argument_get_string(req, "flightno", &fID)){
				kore_buf_append(buf, asset_nofid_html, asset_len_nofid_html);
			}
			else {		
				kore_buf_append(buf, asset_viewflight_html, asset_len_viewflight_html);
				if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
					own_log("LOG_ERR", "%s", "No connection to database on page VIEWFLIGHT");
					kore_pgsql_logerror(&sql);
				}
				else {
					//Save the query in a var. Limit 10, because we don't want more then 10 results.
					snprintf(query, sizeof(query), "SELECT * FROM flight WHERE flight_nr = \'%s\'",fID);
					//Return on the cmd which query is executed.
					own_log("LOG_NOTICE", "User: %d %s %s", uID ,"Query on page VIEWFLIGHT: ",  query);
					//If the query failed, show a error.
					if(!kore_pgsql_query(&sql, query)){
						own_log("LOG_ERR", "User: %d %s",uID, "Failed to execute query on page VIEWFLIGHT");
						kore_pgsql_logerror(&sql);
					}
					//Else query succesfully executed. 
					else {			
						//get the values
						fLoc = kore_pgsql_getvalue(&sql, 0, SQL_FLIGHT_DESTINATION);
						fDate = kore_pgsql_getvalue(&sql, 0, SQL_FLIGHT_DATE);
						fPric = kore_pgsql_getvalue(&sql, 0, SQL_FLIGHT_PRICE);
						fDes = kore_pgsql_getvalue(&sql, 0, 5);

						//add the values to the placeholders
						kore_buf_replace_string(buf, "$location$", fLoc, strlen(fLoc));
						kore_buf_replace_string(buf, "$cost$", fPric, strlen(fPric));
						kore_buf_replace_string(buf, "$data$", fDate, strlen(fDate));
						kore_buf_replace_string(buf, "$description$", fDes, strlen(fDes));
						kore_buf_replace_string(buf, "$flightno$", fID, strlen(fID));
					}	
			
				}
			}
		}
		else if(req->method == HTTP_METHOD_POST){
			//Get a post request and empty the searchName tag, because we don't need it.
			http_populate_post(req);
	
			if(!http_argument_get_string(req, "flightno", &fID)){
				kore_buf_append(buf, asset_nofid_html, asset_len_nofid_html);
			}

			else{
				//If the database connection is not succesfull.
				if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
					kore_pgsql_logerror(&sql);
					own_log("LOG_ERR", "%s", "No connection to database on page VIEWFLIGHT");
				}
					//Else it is succesfull.
				else{
					snprintf(query, sizeof(query), "INSERT INTO users_flight (useruser_id, flightflight_nr) VALUES (\'%d\', \'%s\');",uID ,fID);
					//Return on the cmd which query is executed.
					own_log("LOG_NOTICE", "User: %d %s %s", uID ,"Query on page VIEWFLIGHT: ",  query);
					//If the query failed, show a error.
					if(!kore_pgsql_query(&sql, query)){
						own_log("LOG_ERR", "User: %d %s",uID, "Failed to execute query on page VIEWFLIGHT");
						kore_pgsql_logerror(&sql);
						kore_buf_append(buf, asset_viewflighterror_html, asset_len_viewflighterror_html);
					}
					//Else query succesfully executed. 
					else {	
						kore_buf_append(buf, asset_viewflightsucces_html, asset_len_viewflightsucces_html);
					}
				}
			}
		}		
	
			
	}


	kore_pgsql_cleanup(&sql);
		
	data = kore_buf_release(buf, &len);
	serve_page(req, data, len);
	kore_free(data);
	return (KORE_RESULT_OK);
}
