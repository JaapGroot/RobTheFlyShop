#include "includes.h"
//@description Page to view the booked and paied flights
//@input http_request
//@output nono
int
serve_orders(struct http_request *req)
{

	//to serve a page from a buffer, like with more dynamic content do the following:
	//create a buffer
	char			*fNr, *fDate, *fLoc, *fCan, *fCom, *fPaid, query[300];
	size_t			len;
	struct kore_buf		*buf;
	u_int8_t		*data;
	struct kore_pgsql 	sql;
	int 			uID, rows;

	//Clear the vars
	fNr = NULL; 		//Var for flight number
	fDate = NULL;		//Var for flight date
	fLoc = NULL;		//Var for flight location
	fCan = NULL;		//Var if the flight is cancelled
	fCom = NULL;		//Var for flight comments
	fPaid = NULL;		//Var if the flight is paid

	//Implement buffer for the index
	buf = kore_buf_alloc(0);
	kore_pgsql_init(&sql);

	//Get the uID from the cookie
	uID = getUIDFromCookie(req);
	
	//If the UID is empty, this page should not be reached.
	if(uID == NULL) {
		kore_buf_append(buf, asset_infoPageFail_html, asset_len_infoPageFail_html);
		own_log("LOG_ALERT", "%s", "Page accessed without UID!");
	}
	//UID is valid so continue
	else {	
		//Return error if no connection available.
		if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
			own_log("LOG_ERR", "%s", "No connection to database on page ORDERS");	
		}
		//Connection with database
		else{
			snprintf(query, sizeof(query), "SELECT flight_nr, flight_date, location, cancelled, comment, paid FROM flight INNER JOIN users_flight ON flight.flight_nr=users_flight.flightflight_nr WHERE useruser_id = \'%d\' AND paid = 't'",uID);
			//Return on the cmd which query is executed.
			own_log("LOG_NOTICE", "%s %d %s", "Following query executed on page ORDERS by userid: ", uID, query);
			//If the query failed, show a error.
			if(!kore_pgsql_query(&sql, query)){
				own_log("LOG ERR", "%s", (&sql));
			}
			//Else query succesfully executed. 
			else {	
			 	rows = kore_pgsql_ntuples(&sql);
				if (rows == 0){
					kore_buf_append(buf, asset_noOrder_html, asset_len_noOrder_html);
				}
				else {
					kore_buf_append(buf, asset_orders_html, asset_len_orders_html);
					for (int i = 0; i < rows; i++) {
						fNr = kore_pgsql_getvalue(&sql, i, 0);
						fDate = kore_pgsql_getvalue(&sql, i, 1);
						fLoc = kore_pgsql_getvalue(&sql, i, 2);
						fCan = kore_pgsql_getvalue(&sql, i, 3);
						fCom = kore_pgsql_getvalue(&sql, i, 4);
						fPaid = kore_pgsql_getvalue(&sql, i, 5);
						if(strcmp(fCan, "t") == 0) {
							fCan = "Flight is cancelled, order a new flight. Rob miles are lost in space";
						}
						else {
							fCan = "Your flight is still going!";
						}

						if(strcmp(fPaid, "t") == 0) {
							fPaid = "Your flight is paid!";
						}
						else{
							fPaid = "This shouldn't be here";
						}

						kore_buf_append(buf, asset_flight_list_html, asset_len_flight_list_html);

						kore_buf_replace_string(buf, "$fnum$", fNr, strlen(fNr));
						kore_buf_replace_string(buf, "$data$", fDate, strlen(fDate));
						kore_buf_replace_string(buf, "$loc$", fLoc, strlen(fLoc));
						kore_buf_replace_string(buf, "$canc$", fCan, strlen(fCan));
						kore_buf_replace_string(buf, "$desc$", fCom, strlen(fCom));
						kore_buf_replace_string(buf, "$paid$", fPaid, strlen(fPaid));
					}
				}
			}
		}
	}
	//add the welcome message to the page
	




	kore_pgsql_cleanup(&sql);
	//release the buffer, and get the length
	data = kore_buf_release(buf, &len);
	
	//call serve page to get a pretty header and footer
	serve_page(req ,data, len);

	//free the buffer
	kore_free(data);
	return (KORE_RESULT_OK);
}
