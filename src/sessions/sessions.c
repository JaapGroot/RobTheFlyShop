#include "includes.h"

//Description:	Get session ID from user and request the User ID from DB
//@input:	http_request struct
//@output	unsigned int User ID
unsigned int getUIDFromCookie(struct http_request *req){
	struct		kore_pgsql sql;
	char		*sessionId;
	unsigned int	uid = NULL;
	char		query[150];
	int 		rows;
	
	own_log("LOG_NOTICE", "%s", "getting uid from cookie");
	
	http_populate_cookies(req);
	kore_pgsql_init(&sql);

	if (http_request_cookie(req, "session_id", &sessionId))
	{
		own_log("LOG_DEBUG", "SessionID: %s", sessionId);
	}else{
		own_log("LOG_DEBUG", "%s", "No session cookie found");
		kore_pgsql_cleanup(&sql);
		return NULL;
	}
	
	if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
		kore_pgsql_logerror(&sql);
		kore_pgsql_cleanup(&sql);
		return NULL;
	}

	own_log("LOG_NOTICE", "%s", "make connection with DB");	
	snprintf(query, sizeof(query), "SELECT usersuser_id FROM session WHERE session_id = \'%s\'", sessionId);
	own_log("LOG_NOTICE", "%s", query);
	
	if(!kore_pgsql_query(&sql, query)){
		kore_pgsql_logerror(&sql);
		kore_pgsql_cleanup(&sql);
		return NULL;
	}

	rows = kore_pgsql_ntuples(&sql);
	if(rows == 1){
		uid = atoi(kore_pgsql_getvalue(&sql, 0, 0));
		own_log("LOG_NOTICE", "uid: %d", uid);
	}
	kore_pgsql_cleanup(&sql);
	own_log("LOG_NOTICE", "returing uid: %d", uid); 
	return uid;
}

//Description:	Function to serve sessioncookie, with a session_id within the cookie and has a lifespan
//		of one hour. 
//@input:	http_request pointer value of the cookie (salt) and the userid
//@output	return succes integer
int serveCookie(struct http_request *req, char *value, int uid){
	struct 		kore_pgsql sql;
	char		query[300];
	time_t		timeString = time(NULL) + (1*60*60);
	
	http_response_cookie(req, "session_id", value, "/", timeString, 0, NULL);
	kore_pgsql_init(&sql);
	own_log("LOG_NOTICE", "%s", "push cookie to user");
	if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
		kore_pgsql_logerror(&sql);
		kore_pgsql_cleanup(&sql);
		return 0;
	}
	own_log("LOG_NOTICE", "%s", "make connection with DB");	
	snprintf(query, sizeof(query), "INSERT INTO session(usersuser_id, session_id, expire_date, login_tries) VALUES(\'%d\', \'%s\', to_timestamp(%lu), 0)", uid, value, timeString);
	own_log("LOG_DEBUG", "%s", query); 
	
	if(!kore_pgsql_query(&sql, query)){
		kore_pgsql_logerror(&sql);
		kore_pgsql_cleanup(&sql);
		return 0;
	}
	own_log("LOG_NOTICE", "%s", "push salt to DB");
	kore_pgsql_cleanup(&sql);
	return 1;
}

//Description
//@input:
//@output:
int deleteSession(struct http_request *req)
{
	struct 		kore_pgsql sql;
	time_t		oldTime = time(NULL) - (1*60*60*24*365);
	unsigned int		uid = NULL;

	http_response_cookie(req, "session_id", "", "/", oldTime, 0, NULL);
	kore_pgsql_init(&sql);

	if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
		kore_pgsql_logerror(&sql);
		kore_pgsql_cleanup(&sql);
		return 0;
	}
	
	uid = getUIDFromCookie(req);
	if(uid != NULL){
		char query[150];
		snprintf(query, sizeof(query), "DELETE FROM session WHERE usersuser_id = \'%d\'", uid);
		own_log("LOG_NOTICE", "delete query = %s", query);
		if(!kore_pgsql_query(&sql, "DELETE FROM session WHERE")){
			kore_pgsql_logerror(&sql);
			kore_pgsql_cleanup(&sql);
			return 0;
		}
	}
	kore_pgsql_cleanup(&sql);
	return 1;
}



//Description:
//@input:
//@output:
int getRoleFromUID(unsigned int uid){

	struct		kore_pgsql sql;
	char		query[300];
	int 		i, role, rows;
	kore_pgsql_init(&sql);

	own_log("LOG_NOTICE", "%s", "Getting role from UID"); 	

	if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
		kore_pgsql_logerror(&sql);
		kore_pgsql_cleanup(&sql);
		return 0;
	}

	own_log("LOG_NOTICE", "%s", "make conncection with DB"); 	
	snprintf(query, sizeof(query), "SELECT user_role FROM users WHERE user_id = \'%d\'", uid);
	own_log("LOG_NOTICE", "%s", query); 

	if(!kore_pgsql_query(&sql, query)){
		kore_pgsql_logerror(&sql);
		kore_pgsql_cleanup(&sql);
		return 0;
	}

	rows = kore_pgsql_ntuples(&sql);

	for(i = 0; i < rows; i++)
	{
		role = atoi(kore_pgsql_getvalue(&sql, i, 0));
		own_log("LOG_NOTICE", "role_id: %d", role);
	}
	kore_pgsql_cleanup(&sql);
	return role;
}
