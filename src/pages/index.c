#include "includes.h"

int
serve_index(struct http_request *req)
{
	//to serve a page from a buffer, like with more dynamic content do the following:
	//create a buffer
	size_t			len;
	struct kore_buf		*buff;
	u_int8_t		*data;
	
	//Implement buffer for the index
	buff = kore_buf_alloc(0);

	//add the welcome message to the page
	kore_buf_append(buff, asset_index_html, asset_len_index_html);

	//add the content you want to the buffer
	//in this case we want the list of flights
	//create a database query to get all rows with flights
	struct kore_pgsql sql;
	char *destination, *date, *price, *number;
	int rows;
	//connect to db
	kore_pgsql_init(&sql);
	if(!kore_pgsql_setup(&sql, "DB", KORE_PGSQL_SYNC)){
		kore_pgsql_logerror(&sql);
	}

	//query a list of flights
	if(!kore_pgsql_query(&sql, "SELECT * FROM flight")){
		kore_pgsql_logerror(&sql);
	}else{

		//get the amount of rows returned
		rows = kore_pgsql_ntuples(&sql);

		//run through each row and add the entry to the buffer
		for(int i = 0; i < rows; i++){
			//get the values
			destination = kore_pgsql_getvalue(&sql, i, SQL_FLIGHT_DESTINATION);
			date = kore_pgsql_getvalue(&sql, i, SQL_FLIGHT_DATE);
			price = kore_pgsql_getvalue(&sql, i, SQL_FLIGHT_PRICE);
			number = kore_pgsql_getvalue(&sql, i, SQL_FLIGHT_NUMBER);

			//add an empty template to the buffer
			kore_buf_append(buff, asset_flight_listview_html, asset_len_flight_listview_html);
	
			//add the values to the placeholders
			kore_buf_replace_string(buff, "$location$", destination, strlen(destination));
			kore_buf_replace_string(buff, "$price$", price, strlen(price));
			kore_buf_replace_string(buff, "$date$", date, strlen(date));
			kore_buf_replace_string(buff, "$flightno$", number, strlen(number));
		}
	}	
	kore_pgsql_cleanup(&sql);
	//release the buffer, and get the length
	data = kore_buf_release(buff, &len);
	
	//call serve page to get a pretty header and footer
	serve_page(req ,data, len);

	//free the buffer
	kore_free(data);
	return (KORE_RESULT_OK);
}
