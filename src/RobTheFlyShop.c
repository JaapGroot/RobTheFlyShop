//includes
#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assets.h"
#include "openssl/sha.h"

//macros
#define SQL_FLIGHT_DESTINATION (3)
#define SQL_FLIGHT_PRICE (2)
#define SQL_FLIGHT_DATE (1)
#define SQL_FLIGHT_NUMBER (0)

#define SQL_USERS_ID (0)
#define SQL_USERS_FIRST_NAME (1)
#define SQL_USERS_LAST_NAME (2)
#define SQL_USERS_MAIL (3)
#define SQL_USERS_PASSWORD (4)
#define SQL_USERS_USER_ROLE (5)
#define SQL_USERS_ROB_MILES (6)

//function prototypes
//initialization
int init(int);

//serving pages
int		serve_index(struct http_request *);
int		serve_login(struct http_request *);
int		serve_logedin(struct http_request *);
int		serve_register(struct http_request *);
int		serve_eula(struct http_request *);
int		serve_adminflight(struct http_request *);
int		serve_adminmiles(struct http_request *);
int		serve_adminorders(struct http_request *);
int 		serve_adminaccount(struct http_request *);

//validator functions
int v_admin_validate(struct http_request *, char *);

//serve full page
int serve_page(struct http_request *, u_int8_t *, size_t len);

//check input from the register page, and give warnings where applicable
int check_register(struct http_request *req, struct kore_buf *b, char *checkstring, char *tag, char **returnstring);

//functions for generating Salt and Hash
unsigned int 	randomNumber(void);
void 		generateSalt(void);
unsigned char* 	hashString(unsigned char* org);


//initializes stuff
int init(int state){
	//init database
	kore_pgsql_register("DB", "host=localhost user=pgadmin password=root dbname=rtfsdb"); 
	return (KORE_RESULT_OK);
}

//actual functions
int
serve_index(struct http_request *req)
{
	//to serve a page from a buffer, like with more dynamic content do the following:
	//create a buffer
	size_t			len;
	struct kore_buf		*buff;
	u_int8_t		*data;
	
	//Implement buffer for the index
	buff = kore_buf_alloc(0);

	//add the welcome message to the page
	kore_buf_append(buff, asset_index_html, asset_len_index_html);

	//add the content you want to the buffer
	//in this case we want the list of flights
	//create a database query to get all rows with flights
	struct kore_pgsql sql;
	char *destination, *date, *price, *number;
	int rows;

	//connect to db
	if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
		kore_pgsql_logerror(&sql);
	}

	//query a list of flights
	if(!kore_pgsql_query(&sql, "SELECT * FROM flight")){
		kore_pgsql_logerror(&sql);
	}

	//get the amount of rows returned
	rows = kore_pgsql_ntuples(&sql);

	//run through each row and add the entry to the buffer
	for(int i = 0; i < rows; i++){
		//get the values
		destination = kore_pgsql_getvalue(&sql, i, SQL_FLIGHT_DESTINATION);
		date = kore_pgsql_getvalue(&sql, i, SQL_FLIGHT_DATE);
		price = kore_pgsql_getvalue(&sql, i, SQL_FLIGHT_PRICE);
		number = kore_pgsql_getvalue(&sql, i, SQL_FLIGHT_NUMBER);

		//add an empty template to the buffer
		kore_buf_append(buff, asset_flight_listview_html, asset_len_flight_listview_html);

		//add the values to the placeholders
		kore_buf_replace_string(buff, "$location$", destination, strlen(destination));
		kore_buf_replace_string(buff, "$price$", price, strlen(price));
		kore_buf_replace_string(buff, "$date$", date, strlen(date));
		kore_buf_replace_string(buff, "$flightno$", number, strlen(number));
	}

	//release the buffer, and get the length
	data = kore_buf_release(buff, &len);
	
	//call serve page to get a pretty header and footer
	serve_page(req ,data, len);

	//free the buffer
	kore_free(data);
	return (KORE_RESULT_OK);
}

int
serve_eula(struct http_request *req){
	serve_page(req, asset_eula_txt, asset_len_eula_txt);
	return (KORE_RESULT_OK);
}
	
int
serve_login(struct http_request *req)
{	
	u_int8_t success = 0;
	
	struct kore_buf		*b;
	u_int8_t		*d;
	size_t			len;
	char			*mail, *pass;
	int			UserId = 0;

	//first allocate the buffer
	b = kore_buf_alloc(0);

	if (req->method == HTTP_METHOD_GET){
		http_populate_get(req);
	}

	else if (req->method == HTTP_METHOD_POST){
		//populate and do regex validation on post request
		http_populate_post(req);

		//check if the entry was correct
		if(http_argument_get_string(req, "Email", &mail) && http_argument_get_string(req, "Password", &pass)){
			//TODO: add pass salting and stuff
			//variables are stored at *mail and *pass
			//
			//reserve some variables
			struct kore_pgsql sql;
			int rows;
			
			//init the database
			kore_pgsql_init(&sql);

			//try to connect to the database we called DB for synchronous database searches.
			if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
				//if we couln't connect log the error,
				//youll be resent to the login page as if nothing happened
				success = 0;
				kore_pgsql_logerror(&sql);
			}else{
			//if we did connect you'll be sent to the page that tells you youre logged in
				//build query
				char query[100];
				snprintf(query, sizeof(query), "SELECT * FROM users WHERE mail=\'%s\' AND password=\'%s\'", mail, pass); 

				kore_log(LOG_NOTICE, "%s", query);
				//preform the query
				if(!kore_pgsql_query(&sql, query)){
					kore_pgsql_logerror(&sql);
				}

				//if there were no results the mail or password were incorrect, if there are more then 1 multiple users were selected wich shouldn't happen, so there should only be result if the login is succesful
				rows = kore_pgsql_ntuples(&sql);
				kore_log(LOG_NOTICE, "rows: %i", rows);
				if(rows == 1){
					//set the user id from the database
					UserId = atoi(kore_pgsql_getvalue(&sql, 0, 0));
					success = 1;
				}
			}

			//interfacing with the database is done, clean up...
			kore_pgsql_cleanup(&sql);
		}
		if(!success){
			//else let the user know they did it wrong
			kore_buf_append(b, asset_loginwarning_html, asset_len_loginwarning_html);
			success = 0;
		}
	}
	
	//if login was successful
	if(success){
		//TODO: give a cookie to the user
	
		//the user id should be stored in UserId
		kore_log(LOG_NOTICE, "UID of user: %i", UserId);

		//show the user the logedin page
		kore_buf_append(b, asset_logedin_html, asset_len_logedin_html);
	}else{
		//seve the normal page again
		kore_buf_append(b, asset_login_html, asset_len_login_html);
	}
	
	//serve the page.
	http_response_header(req, "content-type", "text/html");
	d = kore_buf_release(b, &len);
	serve_page(req, d, len);
	kore_free(d);
	
	return (KORE_RESULT_OK);
}

int serve_logedin(struct http_request *req){
	//to serve a page with static content, simply call serve page with the content you want
	serve_page(req, asset_logedin_html, asset_len_logedin_html);
	return (KORE_RESULT_OK);
}

int serve_adminflight(struct http_request *req) {
	char			*fID, *fLoc, *fDate, query[150];
	struct kore_buf		*buf;
	size_t 			len;
	u_int8_t 		*data;
	struct kore_pgsql 	sql;
	int			rows, i,  success = 0;

	//Empty the values
	fID = 0;
	fLoc = 0;
	fDate = 0;

	//Buffer to store the HTML code and init the database.
	buf = kore_buf_alloc(0);
	kore_pgsql_init(&sql);

	//Add the buffer to the page.
	kore_buf_append(buf, asset_cancelFlight_html, asset_len_cancelFlight_html);
	
	//If the site gets a get request do this. To find a user in the database.
	if(req->method == HTTP_METHOD_GET){
		//Validate input
		http_populate_get(req);
	
		//Get the flight location from the getrequest and change the label to the location.
		if (http_argument_get_string(req, "flightLoc", &fLoc)){
			kore_buf_replace_string(buf, "$searchFlightLoc$", fLoc, strlen(fLoc));
		}
		//If failed/no name, empty the label.
		else {
			kore_buf_replace_string(buf, "$searchFlightLoc$", NULL, 0);
		}
		//If the DB connection failed, show a error.
		if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
			kore_pgsql_logerror(&sql);
		}
		//Else check if location is filled in for the search
		else if (fLoc != NULL)	{
			//Save the query in a var. Limit 10, because we don't want more then 10 results and only the last 10 results.
			snprintf(query, sizeof(query), "SELECT * FROM flight WHERE location LIKE \'%%%s%%\' AND cancelled = 'f'  ORDER BY flight_date DESC LIMIT 10",fLoc);
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

	//If it is a POST method. To cancell the flight
	if(req->method == HTTP_METHOD_POST){
		//Get a post request and empty the searcFlightLoc tag, because we don't need it.
		http_populate_post(req);
		kore_buf_replace_string(buf, "$searchFlightLoc$", NULL, 0);
		//If there is data in the selectFlight go on.
		if (http_argument_get_string(req, "selectFlight", &fID)) {
			//If the database connection is not succesfull.
			if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
				kore_pgsql_logerror(&sql);
			}
			//Else it is succesfull.
			else{
				//Put the new SQL statement in the query. And print the query to check it.
				snprintf(query, sizeof(query), "UPDATE flight SET cancelled = 't' WHERE flight_nr = \'%s\'", fID);
				kore_log(LOG_NOTICE, "%s", query);
				//If the query did not execute succesfull, show a error.
				if(!kore_pgsql_query(&sql, query)){
					kore_pgsql_logerror(&sql);
				}
				//Flight succesfully cancelled
				else {	
					success = 1;
				}
				//Close db connection
				kore_pgsql_cleanup(&sql);
			}
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

//Function for serving the register page, along with the logic of registering a user
int serve_register(struct http_request *req){
	char *fname, *lname, *mail, *password, *passwordConfirm;
	//whatever the datatype is for a hash and salt
	struct kore_buf		*b;
	u_int8_t 		*d;
	size_t			len;
	struct kore_pgsql	sql;
	b = kore_buf_alloc(0);
	kore_buf_append(b, asset_register_html, asset_len_register_html);

	//initialize variables
	fname = NULL;
	lname = NULL;
	mail = NULL;
	password = NULL;
	passwordConfirm = NULL;
	
	//if the page was called with a get request
	if(req->method == HTTP_METHOD_GET){
		//take out all the tags
		kore_buf_replace_string(b, "$warning_mail$", NULL, 0);
		kore_buf_replace_string(b, "$warning_fname$", NULL, 0);
		kore_buf_replace_string(b, "$warning_lname$", NULL, 0);
		kore_buf_replace_string(b, "$warning_pass$", NULL, 0);
		kore_buf_replace_string(b, "$warning_box$", NULL, 0);
	}else if(req->method == HTTP_METHOD_POST){
		u_int8_t	inputvalid = 1;

		http_populate_post(req);
		if(!check_register(req, b, "email", "$warning_mail$", &mail)){
			inputvalid = 0;
		}
		if(!check_register(req, b, "fname", "$warning_fname$", &fname)){
			inputvalid = 0;
		}
		if(!check_register(req, b, "lname", "$warning_lname$", &lname)){
			inputvalid = 0;
		}
		if(!check_register(req, b, "agree", "$warning_box$", NULL));
		//check if passwords match
		if(!(http_argument_get_string(req, "password", &password) && http_argument_get_string(req, "passwordConfirm", &passwordConfirm))){
			inputvalid = 0;
			kore_buf_replace_string(b, "$warning_pass$", (void *)asset_register_warning_html, asset_len_register_warning_html);
		}else{
			if(strcmp(password, passwordConfirm)){
				//if the passwords don't match
				inputvalid = 0;
				kore_buf_replace_string(b, "$warning_pass$", (void *)asset_register_warning_html, asset_len_register_warning_html);
			}else{
				kore_buf_replace_string(b, "$warning_pass$", NULL ,0);
			}
		}

		kore_log(1, "checking done");
		//TODO: hash and salt the password
		//
		//

		//if input wasn't valid the variable "inputvalid" will be zero, else it will be one
		//so if input was valid, we can try adding the user to the database, if the user already exists we'll know because the query will fail 
		//init sql
		if(inputvalid){
			kore_pgsql_init(&sql);
			kore_log(1, "building query");
			//build the query to see if the user already exists
			char query[400];
			snprintf(query, sizeof(query), "INSERT INTO users (first_name, last_name, mail, password) VALUES(\'%s\', \'%s\', \'%s\', \'%s\')", fname, lname, mail, password);
			kore_log(1, "Registering user: %s", query);

			//connect to the database
			if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
				kore_pgsql_logerror(&sql);
			}

			//do the query
			if(!kore_pgsql_query(&sql, query)){
				kore_pgsql_logerror(&sql);
				kore_buf_append(b, asset_userexists_html, asset_len_userexists_html);
			}else{
				kore_buf_append(b, asset_register_success_html, asset_len_register_success_html);
			}

			//cleanup database
			kore_pgsql_cleanup(&sql);
		}
	}

	d = kore_buf_release(b, &len);
	serve_page(req, d, len);
	kore_free(d);
	return(KORE_RESULT_OK);
}

//Back-end function to ADD RobMiles to a user. Returns a OK if finished, request the site.
int serve_adminmiles(struct http_request *req) {
	char			*name, *firstName, *lastName, *mail, *sID, *rMiles, query[150];
	struct kore_buf		*buf;
	u_int8_t		*data;
	size_t 			len;
	struct kore_pgsql 	sql;
	int			rows, i,  success = 0;
	
	//Empty all the values, just be sure.
	name = NULL;
	firstName = NULL;
	lastName = NULL;
	mail = NULL;
	sID = NULL;
	rMiles = NULL;
	rows = 0;

	//Buffer to store the HTML code and init the database.
	buf = kore_buf_alloc(0);
	kore_pgsql_init(&sql);

	//Add the html to the buffer.
	kore_buf_append(buf, asset_addMiles_html, asset_len_addMiles_html);

	//If the site gets a get request do this. To find a user in the database.
	if(req->method == HTTP_METHOD_GET){
		//Validate input
		http_populate_get(req);
	
		//Get the lastname from the getrequest and change the label to the lastname.
		if (http_argument_get_string(req, "lastName", &name)) {
			kore_buf_replace_string(buf, "$searchName$", name, strlen(name));
		}
		//If failed/no name, empty the label.
		else {
			kore_buf_replace_string(buf, "$searchName$", NULL, 0);
		}
		//If the DB connection failed, show a error.
		if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
			kore_pgsql_logerror(&sql);
		}
		//Else check if a name is filled in for the search
		else if (name != NULL)	{
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
		}
		//Release the database after use.
		kore_pgsql_cleanup(&sql);
	}
	//If it is a POST method. To add the RobMiles
	if(req->method == HTTP_METHOD_POST){
		//Get a post request and empty the searchName tag, because we don't need it.
		http_populate_post(req);
		kore_buf_replace_string(buf, "$searchName$", NULL, 0);
		//If there is data in both the selectUser and robMiles go on.
		if (http_argument_get_string(req, "selectUser", &sID) && http_argument_get_string(req, "robMiles", &rMiles)) {
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
				//Close db connection
				kore_pgsql_cleanup(&sql);
			}
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

int serve_adminorders(struct http_request *req) {
	return (KORE_RESULT_OK);
}

int serve_adminaccount(struct http_request *req) {
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

//function for checking the input on the register page
int check_register(struct http_request *req, struct kore_buf *b, char *checkstring, char *tag, char **returnstring){
	//get the data
	//if the data could be gathered remove the tag from the page
	//else show a warning on the page and set the return string to NULL
	if(NULL == returnstring){
		char *dummy;
		returnstring = &dummy;
	}
	if(http_argument_get_string(req, checkstring, returnstring)){
		kore_buf_replace_string(b, tag, NULL, 0);
		kore_log(1, "%s", *returnstring);
		return 1;
	}else{
		kore_buf_replace_string(b, tag, asset_register_warning_html, asset_len_register_warning_html);
		returnstring = NULL;
		return 0;
	}	
}

//Validator functions
int v_admin_validate(struct http_request *req, char *data) {
	//TODO: Moet nog gemaakt worden, momenteel voor testen admin page
	return (KORE_RESULT_OK);
}


//Description: Function that opens /dev/urandom/ and get a random number from it
//@input: 	void
//@return: 	unsigned int of a random ten digit number 
unsigned int randomNumber(void)
{
	unsigned int 	randval;
	FILE 		*f;
	
	//open from file /dev/urandom/ and get a 10 digit random number from it.
	f = fopen("/dev/urandom", "r");
	fread(&randval, sizeof(randval), 1, f);
	fclose(f);
	kore_log(1, "Random Number is: %u",randval);
	return randval;
}


//Description: 	Function generating a random Salt. For now it doesn't do very much...
//		other than making a hash. The only problem I have is converting the 
//		"randomhash" and making it a readable hexstring from it.
//@input:	nothing, just make the salt for me please...
//@output:	For now nothing, in the near future a char* of the salt.
void generateSalt(void)
{
	unsigned char	*numberString;
	unsigned int 	randNumber;
	int		i;
	unsigned char	*salt;
	
	randNumber = randomNumber();
	sprintf(numberString, "%u", randNumber);
	
	salt = hashString(numberString);

	//return salt;
}

//Description: hash a string unsing the hashingmethod of SHA256
//@input: 	unsigned char* of the original string 
//@output:	unsigned char* of the hashed string
unsigned char* hashString(unsigned char* org)
{
	unsigned char	*d = SHA256((const unsigned char*)org, strlen(org), 0);
	return d;
}
