#ifndef SERIES_STRUCT_INCLUDE
#define SERIES_STRUCT_INCLUDE
#include "dobjects.h"

// series_struct holds the contents of a series.
typedef struct series_struct {
	// The folder name (used as ID and in the URL) for the series.
	dstring_struct folder_name;

	// The HTML used in the series landing page.
	dstring_struct landing_desc_html;

	// The order the series will appear in, relative to the other series
	// in the site.
	int order;

	// The short description for the series, used in the HTML description
	// meta tag.
	dstring_struct short_description;

	// The title of the series, as it will appear throughout the site
	// (such as on post pages).
	dstring_struct title;

	// All the posts in this series; it is a darray of post_struct*'s.
	darray_struct posts;
} series_struct;

// =========================
// = series_struct functions
// =========================

// Cleans up resources used by the series.
void series_free(series_struct* series);

// Initializes the components of a series.
void series_init(series_struct* series);

// Loads a series from its directory. The folder name is pased
// so that it does not need to be recomputed.
// The list of posts is not calculated in this function, see site_loader.
// Returns NULL on error.
series_struct* series_load(series_struct* series, dstring_struct* series_dir, const char* folder_name);

#endif
