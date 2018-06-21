#include "includes.h"

int serve_cart(struct http_request *req){
	//first process the post request,
	//then serve the rest of the page
	//payed orders should dissapear from the list	
	
	//create a buffer to store the page in
	size_t			len;
	struct kore_buf		*buf;
	u_int8_t		*data;
	
	buf = kore_buf_alloc(0);
	
	//get the user id
	int uID = getUIDFromCookie(req);

	if(req->method == HTTP_METHOD_POST){
		//validate input
		http_populate_post(req);

		//check wich flight was selected
		int64_t flightid = NULL;
		//get the flight id
		if(!http_argument_get_int64(req, "flightid", &flightid)){
			own_log(LOG_WARNING, "User: %d made pressed a nonexistent button on the cart page");
		}else{
			//if the flight id was succesfully found
			//open a database connection
			struct kore_pgsql sql;
			kore_pgsql_init(&sql);
			
			if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
				own_log("LOG_ERR", "No connection to database on page CART");
				kore_pgsql_logerror(&sql);
			}else {
				//get the price of the flight from the db
				char query[300];
				snprintf(query, sizeof(query), "SELECT * FROM users_flight WHERE useruser_id = \'%d\' and flightflight_nr = \'%d\' and paid = \'f\'", uID, flightid);
				if(!kore_pgsql_query(&sql, query)){
					own_log("LOG_ERR", "Query %s failed", query);
					kore_pgsql_logerror(&sql);	
				}else{
				       	if(kore_pgsql_ntuples(&sql) == 1){
						//get the robmiles from the datbase
						int robmiles = 0;
						int cost = 0;
						snprintf(query, sizeof(query), "SELECT rob_miles FROM users WHERE user_id = \'%d\'", uID);
						if(!kore_pgsql_query(&sql, query)){	//this query can't go  wrong, as a connection has already been made and the uid has already been esteblished and used.
							kore_pgsql_logerror(&sql);
						}
						robmiles = atoi(kore_pgsql_getvalue(&sql, 0,0));
						snprintf(query, sizeof(query), "SELECT cost_rob_miles FROM flight WHERE flight_nr = \'%d\'", flightid);
						kore_pgsql_query(&sql, query);
						cost = atoi(kore_pgsql_getvalue(&sql, 0, 0));
						//see if the user has enough miles to pay for the flight
						int remainder = robmiles - cost;
						if(remainder >= 0){
							//update the miles in the users account
							//set the flight as payed
							snprintf(query, sizeof(query), "UPDATE users SET rob_miles = \'%d\' WHERE user_id = \'%d\'; UPDATE users_flight SET paid = \'t\' WHERE flightflight_nr = \'%d\'", remainder, uID, flightid);
							if(!kore_pgsql_query(&sql, query)){
								kore_pgsql_logerror(&sql);
							}
						}else{
							//let the user know paying went wrong
							kore_buf_append(buf, asset_insufficient_funds_html, asset_len_insufficient_funds_html);
						}
					}
				}
			}
			//free the sql object
			kore_pgsql_cleanup(&sql);
		}
	}//post request
	//the post request, if it happend has been handled, great!
	//now lets build the actual page  to show
	//first we make a connection to the database to see if this user has any booked, and unpayed flights
	struct kore_pgsql sql;
	int rows;
	
	kore_pgsql_init(&sql);
	if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
		kore_pgsql_logerror(&sql);
	}else{
		//see if the user has any unpayed bookings
		char query[500];
		//get how many robmiles this user has
		int robmiles = 0;
		snprintf(query, sizeof(query), "SELECT rob_miles FROM users WHERE user_id = \'%d\'", uID);
		if(!kore_pgsql_query(&sql, query)){
			kore_pgsql_logerror(&sql);
		}else{
			robmiles = atoi(kore_pgsql_getvalue(&sql, 0, 0));
		}
		snprintf(query, sizeof(query), "SELECT flight_nr, location, flight_date, cost_rob_miles FROM flight INNER JOIN users_flight ON flight.flight_nr = users_flight.flightflight_nr WHERE useruser_id = \'%d\' AND paid = \'f\'", uID);
		if(!kore_pgsql_query(&sql, query)){
			kore_pgsql_logerror(&sql);
		}else{
			rows = kore_pgsql_ntuples(&sql);
			if(rows > 0){
				int cost = 0;
				int remaining = 0;
				char *location, *date, *flightid;
				char numbuf[10];


				//add a template to the buffer
				for(int i = 0; i < rows; i++){
					kore_buf_append(buf, asset_cart_html, asset_len_cart_html);
					location = kore_pgsql_getvalue(&sql, i, 1);
					date = kore_pgsql_getvalue(&sql, i, 2);
					cost = atoi(kore_pgsql_getvalue(&sql, i, 3));
					flightid = kore_pgsql_getvalue(&sql, i, 0);	
					remaining = robmiles - cost;

					kore_buf_replace_string(buf, "$location$", location, strlen(location));
					kore_buf_replace_string(buf, "$date$", date, strlen(date));
					snprintf(numbuf, sizeof(numbuf), "%d", cost);
					kore_buf_replace_string(buf, "$price$", numbuf, strlen(numbuf));
					snprintf(numbuf, sizeof(numbuf), "%d", robmiles);
					kore_buf_replace_string(buf, "$current$", numbuf, strlen(numbuf));
					snprintf(numbuf, sizeof(numbuf), "%d", remaining);
					kore_buf_replace_string(buf, "$remaining$", numbuf, strlen(numbuf));

					//if the user does not have enough robmiles, show the red button
					//else show the green button
					if(remaining < 0){
						kore_buf_replace_string(buf, "$button$", asset_nopaybutton_html,asset_len_nopaybutton_html); 
					}else{
						kore_buf_replace_string(buf, "$button$", asset_paybutton_html, asset_len_paybutton_html);
						kore_buf_replace_string(buf, "$flightid$", flightid, strlen(flightid));
					}
				}
			}else{
				//user has no unpayed flights, let him know
				kore_buf_append(buf,asset_nounpayed_html, asset_len_nounpayed_html);
			}
		}
	}
	kore_pgsql_cleanup(&sql);

	data = kore_buf_release(buf, &len);
	serve_page(req, data, len);
	kore_free(data);
	return(KORE_RESULT_OK);
}//serve page
