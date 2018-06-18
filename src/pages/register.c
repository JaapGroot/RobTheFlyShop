#include "includes.h" 

int
serve_eula(struct http_request *req){
	serve_page(req, asset_eula_txt, asset_len_eula_txt);
	return (KORE_RESULT_OK);
}

//Function for serving the register page, along with the logic of registering a user
int serve_register(struct http_request *req){
	char *fname, *lname, *mail, *password, *passwordConfirm;
	struct hashsalt 	hs;	
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

		kore_log(1, "[register page]");
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

		
		//if input wasn't valid the variable "inputvalid" will be zero, else it will be one
		//so if input was valid, we can try adding the user to the database, if the user already exists we'll know because the query will fail 
		if(inputvalid){
			//hash and salt the password
			hs = generateNewPass(password);
			//init the database
			kore_pgsql_init(&sql);
			//build the query to see if the user already exists
			char query[400];
			snprintf(query, sizeof(query), "INSERT INTO users (first_name, last_name, mail, password) VALUES(\'%s\', \'%s\', \'%s\', \'%s\')", fname, lname, mail, hs.HS);
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
		return 1;
	}else{
		kore_buf_replace_string(b, tag, asset_register_warning_html, asset_len_register_warning_html);
		returnstring = NULL;
		return 0;
	}	
}
