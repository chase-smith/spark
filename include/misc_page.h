#ifndef MISC_PAGE_STRUCT_INCLUDE
#define MISC_PAGE_STRUCT_INCLUDE
#include "dobjects.h"
typedef struct misc_page_struct {
	dstring_struct content;
	dstring_struct title;
	dstring_struct filename;
	dstring_struct description;
	int generate;
} misc_page_struct;
void misc_page_free(misc_page_struct*);
misc_page_struct* misc_page_load(misc_page_struct*, dstring_struct*);
void misc_page_init(misc_page_struct*);
#endif
