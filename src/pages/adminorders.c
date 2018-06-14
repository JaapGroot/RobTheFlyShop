#include "includes.h" 
//Query i need later
//SELECT flight_nr, flight_date, cost_rob_miles, location, cancelled 
//FROM flight 
//JOIN users_flight ON flight.flight_nr = users_flight.flightflight_nr
//WHERE useruser_id = '1';

int serve_adminorders(struct http_request *req) {
	char			*name, *firstName, *lastName, *mail, *sID, *rMiles, query[150];
	struct kore_buf		*buf;
	u_int8_t		*data;
	size_t 			len;
	struct kore_pgsql 	sql;
	int			rows, i,  success = 0;


	buf = kore_buf_alloc(0);
	kore_pgsql_init(&sql);

	kore_buf_append(buf, asset_showOrderAdmin_html, asset_len_showOrderAdmin_html);

	if(req->method == HTTP_METHOD_GET){
		//Validate input
		http_populate_get(req);
		kore_log(1, "1");	
		//Check if last name is filled in.
		if (!http_argument_get_string(req, "lastName", &name)) {
			kore_buf_replace_string(buf, "<!--$searchName$-->", NULL, 0);
			kore_log(1, "2");
			kore_log(1, name);
		}
		//If filled in, continue.
		else {
			kore_log(1, "3");
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
						sID = kore_pgsql_getvalue(&sql, i, SQL_USERS_ID);
						lastName = kore_pgsql_getvalue(&sql, i, SQL_USERS_LAST_NAME);
						firstName = kore_pgsql_getvalue(&sql, i, SQL_USERS_FIRST_NAME);
						mail = kore_pgsql_getvalue(&sql, i, SQL_USERS_MAIL);
						//Put the results in a string in list.
						snprintf(list, sizeof(list), "<option value=\"%s\">%s %s %s</option><!--listentry-->", sID, firstName, lastName, mail);
						//Replace listenty with the new list, so it grows.
						kore_buf_replace_string(buf, "<!--listentry-->", list, strlen(list));
					}
				}
				//Release the database after use.
				kore_pgsql_cleanup(&sql);
			}
		}
	}	

	data = kore_buf_release(buf, &len);
	serve_page(req, data, len);
	kore_free(data);
	return (KORE_RESULT_OK);
}
