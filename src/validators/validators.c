#include "includes.h"

//Description:	Validator
//@input:	
//@output:	
int v_admin_validate(struct http_request *req, char *data) {
	unsigned int 	uid, role;

	uid = getUIDFromCookie(req);
	if(uid == NULL){
		return (KORE_RESULT_ERROR);
	}

	role = getRoleFromUID(uid);
	if(role == 1)
	{
		return (KORE_RESULT_OK);
	}
	else
	{
		return (KORE_RESULT_ERROR);
	}
}

//Description:	Validator
//@input:	
//@output:	
int v_user_validate(struct http_request *req, char *data) {
	unsigned int 	uid, role;

	uid = getUIDFromCookie(req);
	if(uid == NULL){
		return (KORE_RESULT_ERROR);
	}

	role = getRoleFromUID(uid);
	
	if(role == 0)
	{
		return (KORE_RESULT_OK);
	}
	else
	{
		return (KORE_RESULT_ERROR);
	}
}


//Description:	Validator
//@input:	
//@output:	
int v_generic_validate(struct http_request *req, char *data) {
	if(v_user_validate(req, data) || v_admin_validate(req, data))
	{
		return (KORE_RESULT_OK);
	}
	else
	{
		return (KORE_RESULT_ERROR);
	}
}

//Description:	Validator
//@input:	
//@output:	
int v_notLogedIn_validate(struct http_request *req, char *data) {
	if(!v_generic_validate(req, data))
	{
		return (KORE_RESULT_OK);
	}
	else
	{
		return (KORE_RESULT_ERROR);
	}
}
