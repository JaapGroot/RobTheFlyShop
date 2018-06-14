#include "includes.h" 

int serve_adminorders(struct http_request *req) {
	char			*name, *firstName, *lastName, *mail, *uID, *destination, *date, *price, *fNr, query[300];
	struct kore_buf		*buf;
	u_int8_t		*data;
	size_t 			len;
	struct kore_pgsql 	sql;
	int			rows, i,  success = 0;

	//Clear the vars Rob!
	name = NULL;		//Var for the get input
	firstName = NULL;	//Var to store user firstName
	lastName = NULL;	//Var to store user lastName
	mail = NULL;		//Var to store user mail
	uID = NULL;		//Var to store user ID
	destination = NULL;	//Var to store flight destination
	date = NULL;		//Var to store flight date
	price = NULL;		//Var to store flight price
	fNr = NULL;		//Var to store flight NR

	//Allocate the buffer and init the database.
	buf = kore_buf_alloc(0);
	kore_pgsql_init(&sql);

	//Fill the buffer with the page.
	kore_buf_append(buf, asset_showOrderAdmin_html, asset_len_showOrderAdmin_html);

	//If there is a get request
	if(req->method == HTTP_METHOD_GET){
		//Validate input
		http_populate_get(req);
		//Check if last name is filled in.
		if (!http_argument_get_string(req, "lastName", &name)) {
			kore_buf_replace_string(buf, "<!--$searchName$-->", NULL, 0);
		}
		//If filled in, continue.
		else {
			kore_buf_replace_string(buf, "<!--$searchName$-->", name, strlen(name));
			//If the DB connection failed, show a error.
			if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
				kore_pgsql_logerror(&sql);
			}
			else {
				//Save the query in a var. Limit 10, because we don't want more then 10 results.
				snprintf(query, sizeof(query), "SELECT * FROM users WHERE last_name LIKE \'%%%s%%\' LIMIT 10",name);
				//Return on the cmd which query is executed.
				kore_log(LOG_NOTICE, "%s", query);
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
						uID = kore_pgsql_getvalue(&sql, i, SQL_USERS_ID);
						lastName = kore_pgsql_getvalue(&sql, i, SQL_USERS_LAST_NAME);
						firstName = kore_pgsql_getvalue(&sql, i, SQL_USERS_FIRST_NAME);
						mail = kore_pgsql_getvalue(&sql, i, SQL_USERS_MAIL);
						//Put the results in a string in list.
						snprintf(list, sizeof(list), "<option value=\"%s\">%s %s %s</option><!--listentry-->", uID, firstName, lastName, mail);
						//Replace listenty with the new list, so it grows.
						kore_buf_replace_string(buf, "<!--listentry-->", list, strlen(list));
					}
				}
				//Release the database after use.
				kore_pgsql_cleanup(&sql);
			}
		}
	}	

	//If it is a POST method. To cancell the flight
	if(req->method == HTTP_METHOD_POST){
		//Get a post request and empty the searcFlightLoc tag, because we don't need it.
		http_populate_post(req);
		
		//Check if a flight is selected.
		if (!http_argument_get_string(req, "selectUser", &uID)) {
			kore_buf_replace_string(buf, "<!--$searchName$-->", NULL, 0);
		}
		else {
			//If the database connection is not succesfull.
			if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
				kore_pgsql_logerror(&sql);
			}
			//Else it is succesfull.
			else{
				//Put the new SQL statement in the query. And print the query to check it.
				//snprintf(query, sizeof(query), "SELECT flight_nr, flight_date, cost_rob_miles, location, cancelled FROM flight JOIN users_flight ON flight.flight_nr = users_flight.flightflight_nr WHERE useruser_id = \'%s\'", uID);
				snprintf(query, sizeof(query), "SELECT first_name, last_name, flight_nr, flight_date, cost_rob_miles, location, cancelled FROM flight JOIN users_flight ON flight.flight_nr = users_flight.flightflight_nr JOIN users ON users.user_id = users_flight.useruser_id WHERE useruser_id = \'%s\'", uID);
				kore_log(LOG_NOTICE, "%s", query);
				//If the query did not execute succesfull, show a error.
				if(!kore_pgsql_query(&sql, query)){
					kore_pgsql_logerror(&sql);
				}
				//Flight succesfully cancelled
				else {	
					rows = kore_pgsql_ntuples(&sql);
					
					//Get the name from the tuple
					lastName = kore_pgsql_getvalue(&sql, 0, 1);
					firstName = kore_pgsql_getvalue(&sql, 0, 0);

					//Put the data in the HTML page.
					kore_buf_replace_string(buf, "<!--first-->", firstName, strlen(firstName));
					kore_buf_replace_string(buf, "<!--last-->", lastName, strlen(lastName));
					//To add all the flights to the page
					for(i=0; i<rows; i++) {
						//Fill the vars with data
						destination = kore_pgsql_getvalue(&sql, i, 5);
						date = kore_pgsql_getvalue(&sql, i,3);
						price = kore_pgsql_getvalue(&sql, i, 4);
						fNr = kore_pgsql_getvalue(&sql, i, 2);

						//Add the show flight page to the buff
						kore_buf_append(buf, asset_show_flight_html, asset_len_show_flight_html);

						//add the values to the placeholders
						kore_buf_replace_string(buf, "$location$", destination, strlen(destination));
						kore_buf_replace_string(buf, "$price$", price, strlen(price));
						kore_buf_replace_string(buf, "$date$", date, strlen(date));
						kore_buf_replace_string(buf, "$flightno$", fNr, strlen(fNr));
					}
				}
			}
			//Close db connection
			kore_pgsql_cleanup(&sql);
		}	
	}

	//Sere the page
	data = kore_buf_release(buf, &len);
	serve_page(req, data, len);
	kore_free(data);
	return (KORE_RESULT_OK);
}
