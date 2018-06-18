#include "includes.h"

int serve_logout(struct http_request *req){
	//create buffer for logout message
	struct kore_buf *buff;
	u_int8_t	*data;
	size_t		len;

	buff = kore_buf_alloc(0);
	kore_buf_append(buff, asset_logout_html, asset_len_logout_html);
	
	kore_log(1, "[logout page]");	
	//delete the session
	if(deleteSession(req)){
		//if deletion succeded:
		//replace the success tag with successfully
		kore_buf_replace_string(buff, "$success$", "successfully", strlen("successfully"));
	}else{
		//else let the user know logging out didn't work
		kore_buf_replace_string(buff, "$success$", "not successfully", strlen("not successfully"));
	}

	//release the buffer, and get the length
	data = kore_buf_release(buff, &len);
	
	//call serve page to get a pretty header and footer
	serve_page(req ,data, len);

	//free the buffer
	kore_free(data);

	return (KORE_RESULT_OK);	
}
