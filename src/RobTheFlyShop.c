#include <kore/kore.h>
#include <kore/http.h>

#include "assets.h"

//function prototypes
//serving pages
int		serve_index(struct http_request *);
int		serve_linked(struct http_request *);

//actual functions
int
serve_index(struct http_request *req)
{
//	size_t		len;
//	struct kore_buf	*res;
//	u_int8_t	*data;

//	res = kore_buf_alloc(0);
//	kore_buf_append(res, asset_index_html, asset_len_index_html);

	http_response(req, 200, asset_index_html, asset_len_index_html);

//	kore_buf_free(res);
	return (KORE_RESULT_OK);
}
	
int
serve_linked(struct http_request *req)
{
	http_response(req, 200, asset_linked_html, asset_len_linked_html);
	return (KORE_RESULT_OK);
}
