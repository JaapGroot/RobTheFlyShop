#include "includes.h"

//@description Back-end function to ADD RobMiles to a user.
//Input http request from the site
//Output none
int serve_admin_add_miles(struct http_request *req) {
	char			*name, *firstName, *lastName, *mail, *sID, *rMiles, query[150];
	struct kore_buf		*buf;
	u_int8_t		*data;
	size_t 			len;
	struct kore_pgsql 	sql;
	int			rows, i,  success = 0;
	
	//Empty all the values, just be sure.
	name = NULL;		//Name searched with the get request.
	firstName = NULL;	//First name from the searched user
	lastName = NULL;	//Last name from the searched user
	mail = NULL;		//Mail from the searched user
	sID = NULL;		//sID(stringID) from the searched user
	rMiles = NULL;		//rMiles(Rob Miles) Miles to add to the user

	//Buffer to store the HTML code and init the database.
	buf = kore_buf_alloc(0);
	kore_pgsql_init(&sql);

	//Add the html to the buffer.
	kore_buf_append(buf, asset_addMiles_html, asset_len_addMiles_html);

	//If the site gets a get request do this. To find a user in the database.
	if(req->method == HTTP_METHOD_GET){
		//Validate input
		http_populate_get(req);
	
		//Check if last name is filled in.
		if (!http_argument_get_string(req, "lastName", &name)) {
			kore_buf_replace_string(buf, "$searchName$", NULL, 0);
		}
		//If filled in, continue.
		else {
			kore_buf_replace_string(buf, "$searchName$", name, strlen(name));
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
	//If it is a POST method. To add the RobMiles
	if(req->method == HTTP_METHOD_POST){
		//Get a post request and empty the searchName tag, because we don't need it.
		http_populate_post(req);
		
		//If there is data in both the selectUser and robMiles go on.
		if (!(http_argument_get_string(req, "selectUser", &sID) && http_argument_get_string(req, "robMiles", &rMiles))) {
		kore_buf_replace_string(buf, "$searchName$", NULL, 0);
		}
		else{
			//If the database connection is not succesfull.
			if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
				kore_pgsql_logerror(&sql);
			}
			//Else it is succesfull.
			else{
				//Put the new SQL statement in the query. And print the query to check it.
				snprintf(query, sizeof(query), "UPDATE users SET rob_miles = rob_miles + \'%s\' WHERE user_id = \'%s\'",rMiles, sID);
				kore_log(LOG_NOTICE, "%s", query);
				//If the query did not execute succesfull, show a error.
				if(!kore_pgsql_query(&sql, query)){
					kore_pgsql_logerror(&sql);
				}
				//Rob Miles succesfull added.
				else {	
					success = 1;
				}
			}
			//Close db connection
			kore_pgsql_cleanup(&sql);
		}
		//If succeed, show at the page, otherwise show a fail.
		if (success == 1){
			kore_buf_append(buf,asset_milesSucces_html,asset_len_milesSucces_html); 
		}
		else {
			kore_buf_append(buf,asset_milesFailed_html,asset_len_milesFailed_html); 
		}	
	}
	//Put the buf in data and serve the page.
	data = kore_buf_release(buf, &len);
	serve_page(req, data, len);
	kore_free(data);
	return (KORE_RESULT_OK);
}
