#ifndef pages_h
#define pages_h
int		serve_index(struct http_request *);
int		serve_login(struct http_request *);
int		serve_logedin(struct http_request *);
int		serve_logout(struct http_request *);
int		serve_register(struct http_request *);
int		serve_eula(struct http_request *);
int		serve_adminflight(struct http_request *);
int		serve_adminmiles(struct http_request *);
int		serve_adminorders(struct http_request *);
int 		serve_adminaccount(struct http_request *);

#endif //pages_h
