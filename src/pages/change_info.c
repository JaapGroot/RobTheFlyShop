#include "includes.h"

//@description Back-end to change user data
//Input the http request of the site
//output none
int serve_change_info(struct http_request *req) {
	struct kore_buf		*buf;
	char			*mail, *fName, *lName, *passOld, *passNew, *passConf, query[150], add[25];
	u_int8_t		*data;
	size_t 			len;
	struct kore_pgsql	sql;
	int			rows, succes = 0, uID;
	struct hashsalt		hs;

	//Clear those vars Rob
	mail = NULL;		//var for the changed mail
	fName = NULL;		//var for the changed first name
	lName = NULL;		//var for the changed last name
	passOld = NULL;		//var for the old password
	passNew = NULL;		//var for the new password
	passConf = NULL;	//var for the confirmed password
	uID = NULL;		//var for the userID

	//Alloc the buffer and init the database.
	buf = kore_buf_alloc(0);
	kore_pgsql_init(&sql);
	
	//Fill the buff with the page
	kore_buf_append(buf, asset_editInfo_html, asset_len_editInfo_html);

	//TODO get ID from the cookie so know which data to change
	uID = getUIDFromCookie(req);
	//If it is a POST method. To add the RobMiles
	if(req->method == HTTP_METHOD_POST){
		//Get a post request and empty the searchName tag, because we don't need it.
		http_populate_post(req);
		
		//If the database connection failed. Stop.
		if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
			kore_pgsql_logerror(&sql);
		}			
		//Else database connection succeed and continue
		else{
			//If no old password is given, you can't change data so quit.
			if (!http_argument_get_string(req, "passwordOld", &passOld)) {
				kore_buf_replace_string(buf, "<!--$NoPassWarn$-->", "You must enter old password to change data.", strlen("You must enter old password to change data."));

			}
			//Old pw is given so continue.
			else {
				//Get the old pw of the database.
				snprintf(query, sizeof(query), "SELECT password FROM users WHERE user_id = \'%d\'",uID);
				//If the query failed, show a error.
				if(!kore_pgsql_query(&sql, query)){
					kore_pgsql_logerror(&sql);
				}
				//Else query succesfully executed. 
				else {
					//get the old password from the db
					strcpy(hs.HS, kore_pgsql_getvalue(&sql, 0, 0));
					//Check if the submitted pw and pw from the db are the same.
					if(!checkPass(hs, passOld)){
						kore_buf_replace_string(buf, "<!--$NoPassWarn$-->", "Invalid Password", strlen("Invalid Password"));
					}
					//If they are the same continue.
					else{
						//You can change data now, so it always succeed.
						succes = 1;

						//If an email argument is given change it.
						if (http_argument_get_string(req, "email", &mail)) {
							snprintf(query, sizeof(query), "UPDATE users SET mail = \'%s\' WHERE user_id = \'%d\'", mail, uID);
							//If the query failed, show a error.
							if(!kore_pgsql_query(&sql, query)){
								kore_pgsql_logerror(&sql);
							}
							//Else query succesfully executed. 
							else {
								strcat(add, "mail ");
							}
						}
						//If a name argument is given change it.
						if (http_argument_get_string(req, "fname", &fName)) {
							snprintf(query, sizeof(query), "UPDATE users SET first_name = \'%s\' WHERE user_id = \'%d\'", fName, uID);
							//If the query failed, show a error.
							if(!kore_pgsql_query(&sql, query)){
								kore_pgsql_logerror(&sql);
							}
							//Else query succesfully executed. 
							else {
								strcat(add, "First name ");
							}
						}
						//If a lastname argument is given change it.
						if (http_argument_get_string(req, "lname", &lName)) {
							snprintf(query, sizeof(query), "UPDATE users SET last_name = \'%s\' WHERE user_id = \'%d\'", lName, uID);
							//If the query failed, show a error.
							if(!kore_pgsql_query(&sql, query)){
								kore_pgsql_logerror(&sql);
							}
							//Else query succesfully executed. 
							else {
								strcat(add, "Last name ");
							}
						}
						//If new and conf password are given change it.
						if (http_argument_get_string(req, "passwordConfirm", &passConf)&&http_argument_get_string(req, "passwordnew", &passNew)) {
							//Hash the new pass
							//If new and conf are the same and new and old are not the same change it.
							if((strcmp(passNew, passConf) == 0) && !checkPass(hs, passNew)){
								hs = generateNewPass(passNew);
								snprintf(query, sizeof(query), "UPDATE users SET password = \'%s\' WHERE user_id = \'%d\'", hs.HS, uID);
								//If the query failed, show a error.
								if(!kore_pgsql_query(&sql, query)){
									kore_pgsql_logerror(&sql);
								}
								//Else query succesfully executed. 
								else {
									strcat(add, "password");
								}
							}
							else{
								//It failed, because passwords are the same
								kore_buf_replace_string(buf, "<!--$NoPassWarn$-->", "Old password is the same as new password", strlen("Old password is the same as new password"));
								strcat(add, "PASSWORD DID NOT CHANGE");
							}
						}
					}
				}
			}	
			//If succes == 1 show the succes page, otherwise the failed page.
			if (succes == 1){
				kore_buf_append(buf,asset_editInfoSucces_html,asset_len_editInfoSucces_html); 
				kore_buf_replace_string(buf, "$info$", add, strlen(add));
			}
			else {
				kore_buf_append(buf,asset_editInfoFailed_html,asset_len_editInfoFailed_html); 
			}
		}	
		//Clean the database.
		kore_pgsql_cleanup(&sql);
	}
	//Serve the page.		
	data = kore_buf_release(buf, &len);
	serve_page(req, data, len);
	kore_free(data);
	return (KORE_RESULT_OK);
}
