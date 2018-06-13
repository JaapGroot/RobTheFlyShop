#ifndef sessions_h
#define sessions_h


//functions for cookie chechink and generating
int 		deleteSession(struct http_request *req);
unsigned int	getUIDFromCookie(struct http_request *req);
int		serveCookie(struct http_request *req, char *value, int uid);
int		getRoleFromUID(unsigned int uid);

#endif
