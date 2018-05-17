#include <kore/kore.h>
#include <kore/http.h>

#include "assets.h"

//function prototype for serving the index
int		serve_index(struct http_request *);

//
int
serve_index(struct http_request *req)
{
	http_response(req, 200, asset_index_html, 2000);
	return (KORE_RESULT_OK);
}
