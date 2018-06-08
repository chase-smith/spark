#ifndef SERIES_STRUCT_INCLUDE
#define SERIES_STRUCT_INCLUDE
#include "dobjects.h"
typedef struct series_struct {
	dstring_struct folder_name;
	dstring_struct landing_desc_html;
	int order;
	dstring_struct short_description;
	dstring_struct title;
	darray_struct posts;
} series_struct;
void series_free(series_struct*);
series_struct* series_load(series_struct*, dstring_struct*, const char*);
series_struct* series_init(series_struct*);
#endif
