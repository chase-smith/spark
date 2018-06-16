#ifndef HTML_COMPONENTS_STRUCT_INCLUDE
#define HTML_COMPONENTS_STRUCT_INCLUDE
#include "dobjects.h"

// html_components_struct holds the contents of the files defined in the
// components/ directory of a site.
typedef struct html_components_struct {
	// The HTML header (with opening <html> and <head> tags)
	dstring_struct header;

	// The page footer (likely has site navigation links)
	dstring_struct footer;

	// The HTML trailer (with closing </html> tag)
	dstring_struct trailer;
} html_components_struct;

// ==================================
// = html_components_struct functions
// ==================================

// Cleans up the resources used by html_components; does not free
// the pointer itself.
void html_components_free(html_components_struct* html_components);

// Initializes the specified html_components object.
void html_components_init(html_components_struct* html_components);

// Loads the component files in components_dir into html_components.
// Returns NULL on error.
html_components_struct* html_components_load(html_components_struct* html_components, dstring_struct* components_dir);

#endif
