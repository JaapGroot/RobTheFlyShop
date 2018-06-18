#ifndef validators_h
#define validators_h

//validator functions
int v_admin_validate(struct http_request *, char *);
int v_user_validate(struct http_request *, char *);
int v_generic_validate(struct http_request *, char *);
int v_notLogedIn_validate(struct http_request *, char *);

#endif //validators_h
