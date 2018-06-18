#ifndef HTML_PAGE_CREATORS_INCLUDE
#define HTML_PAGE_CREATORS_INCLUDE
#include "dobjects.h"
#include "core_objects.h"

#define PAGE_GENERATION_FAILURE 0
#define PAGE_GENERATION_NO_UPDATE 1
#define PAGE_GENERATION_UPDATED 2

// html_page_creators defines functions for actually creating individual HTML
// pages on a site. These functions create both the bright and dark variants
// of a page. HTML files are not actually written if there is no difference
// between the existing file and the 'new' one; this is not just to save on
// disk writes, but also (and primarily) to allow the webserver to properly
// cache pages that don't change (nginx uses 'date last modified' in its
// caching behavior, and I imagine other webservers do as well).

// ==============================
// = html_page_creators functions
// ==============================

// Creates a misc page. Note, the misc page can either be a 'static' one
// (one that was defined in the folder/file structure of the site), or
// a 'dynamic' one (one created by Spark, such as a tag page), it
// does not matter, there is no distinction between the two.
// Returns one of the PAGE_GENERATION_* values.
int create_misc_page(site_content_struct* site_content, misc_page_struct* misc_page);

// Creates a post page.
// Returns one of the PAGE_GENERATION_* values.
int create_post_page(site_content_struct* site_content, post_struct* post);

#endif
