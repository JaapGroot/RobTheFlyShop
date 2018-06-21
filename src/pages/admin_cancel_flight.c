#include "includes.h"

//@description Back-end function to cancel a flight
//Input The http_request of the page
//Output None
int serve_admin_cancel_flight(struct http_request *req) {
	char			*fID, *fLoc, *fDate, query[150];
	struct kore_buf		*buf;
	size_t 			len;
	u_int8_t 		*data;
	struct kore_pgsql 	sql;			
	int			rows, i,  success = 0;

	//Empty the values
	fID = NULL;		//fID(flight ID) is the ID of the flight
	fLoc = NULL;		//fLoc(flight location) is the location of the flight
	fDate = NULL;		//fDate(flight date)
	rows = NULL;		//Rows that are returned from the query

	//Buffer to store the HTML code and init the database.
	buf = kore_buf_alloc(0);
	kore_pgsql_init(&sql);

	//Add the buffer to the page.
	kore_buf_append(buf, asset_cancelFlight_html, asset_len_cancelFlight_html);
	
	//If the site gets a get request do this. To find a user in the database.
	if(req->method == HTTP_METHOD_GET){
		//Validate input
		http_populate_get(req);
	
		//Check if an argument is given.
		if (!http_argument_get_string(req, "flightLoc", &fLoc)){
			kore_buf_replace_string(buf, "<!--$searchFlightLoc$-->", NULL, 0);
		}
		//An value is given, so continue.
		else {
			kore_buf_replace_string(buf, "<!--$searchFlightLoc$-->", fLoc, strlen(fLoc));
		
			//If the DB connection failed, show a error.
	
			if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
				kore_pgsql_logerror(&sql);
			}
			else{
				//Save the query in a var. Limit 10, because we don't want more then 10 results and only the last 10 results.
				snprintf(query, sizeof(query), "SELECT * FROM flight WHERE location LIKE \'%%%s%%\' AND cancelled = 'f'  ORDER BY flight_date DESC LIMIT 10",fLoc);
				//Return on the cmd which query is executed.
				//If the query failed, show a error.
				if(!kore_pgsql_query(&sql, query)){
					kore_pgsql_logerror(&sql);
				}
				//Else query succesfully executed. 
				else {	
				//Get the rows and make a char to create the HTML list. 	
					rows = kore_pgsql_ntuples(&sql);
					char list[300];
					//For the amount of rows.
					for(i=0; i<rows; i++) {
						//Put the data from the SQL query in the vars.
						fID = kore_pgsql_getvalue(&sql, i, SQL_FLIGHT_NUMBER);
						fLoc = kore_pgsql_getvalue(&sql, i, SQL_FLIGHT_DESTINATION);
						fDate = kore_pgsql_getvalue(&sql, i, SQL_FLIGHT_DATE);
						//Put the results in a string in list.
						snprintf(list, sizeof(list), "<option value=\"%s\">%s %s</option><!--listentry-->", fID, fLoc, fDate);
						//Replace listenty with the new list, so it grows.
						kore_buf_replace_string(buf, "<!--listentry-->", list, strlen(list));
					}
				}
			}
			//Release the database after use.
			kore_pgsql_cleanup(&sql);
		}
	}
	//If it is a POST method. To cancell the flight
	if(req->method == HTTP_METHOD_POST){
		//Get a post request and empty the searcFlightLoc tag, because we don't need it.
		http_populate_post(req);
		
		//Check if a flight is selected.
		if (!http_argument_get_string(req, "selectFlight", &fID)) {
			kore_buf_replace_string(buf, "<!--$searchFlightLoc$-->", NULL, 0);
		}
		else {
			//If the database connection is not succesfull.
			if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
				kore_pgsql_logerror(&sql);
			}
			//Else it is succesfull.
			else{
				//Put the new SQL statement in the query. And print the query to check it.
				snprintf(query, sizeof(query), "UPDATE flight SET cancelled = 't' WHERE flight_nr = \'%s\'", fID);
				//If the query did not execute succesfull, show a error.
				if(!kore_pgsql_query(&sql, query)){
					kore_pgsql_logerror(&sql);
				}
				//Flight succesfully cancelled
				else {	
					success = 1;
				}
			}
			//Close db connection
			kore_pgsql_cleanup(&sql);
		}	
		
		//If succeed, show at the page, otherwise show a fail.
		if (success == 1){
			kore_buf_append(buf,asset_cancelSucces_html,asset_len_cancelSucces_html); 
		}
		else {
			kore_buf_append(buf,asset_cancelFailed_html,asset_len_cancelFailed_html); 
		}	
	}

	//Serve the page
	data = kore_buf_release(buf, &len);
	serve_page(req, data, len);
	kore_free(data);
	return (KORE_RESULT_OK);
}
