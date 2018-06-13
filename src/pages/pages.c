#include "includes.h"

//check input from the register page, and give warnings where applicable
int check_register(struct http_request *req, struct kore_buf *b, char *checkstring, char *tag, char **returnstring);

