#ifndef HTML_COMPONENTS_STRUCT_INCLUDE
#define HTML_COMPONENTS_STRUCT_INCLUDE
#include "dobjects.h"
typedef struct html_components_struct {
	dstring_struct header;
	dstring_struct footer;
	dstring_struct trailer;
} html_components_struct;
void html_components_free(html_components_struct*);
html_components_struct* html_components_init(html_components_struct*);
html_components_struct* html_components_load(html_components_struct*, dstring_struct*);
#endif
