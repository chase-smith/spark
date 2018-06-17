#ifndef MISC_PAGE_STRUCT_INCLUDE
#define MISC_PAGE_STRUCT_INCLUDE
#include "dobjects.h"

// misc_page_struct describes a "miscellaneous" page to be generated.
// Its primary purpose is to describe the pages in the misc_pages/
// directory, but it is also used in site_generator to describe
// and generate purely dynamic pages as well (such as the series
// landing pages).
typedef struct misc_page_struct {
	// The page content.
	dstring_struct content;

	// The title of the page, as will be used in the <title> HTML tag.
	dstring_struct title;

	// The output filename (eg "index.html" or "tags/meta/index.html").
	dstring_struct filename;

	// The description of the page, as will be used in the description meta
	// HTML tag in the header.
	dstring_struct description;
} misc_page_struct;

// ============================
// = misc_page_struct functions
// ============================

// Cleans up the resources for the misc_page; does not free
// the pointer itself.
void misc_page_free(misc_page_struct* misc_page);

// Initializes the misc_page so that it can be used.
void misc_page_init(misc_page_struct* misc_page);

// Loads a misc_page from its misc_page_dir (eg misc_pages/index/).
// Returns NULL on error.
misc_page_struct* misc_page_load(misc_page_struct* misc_page, dstring_struct* misc_page_dir);

#endif
