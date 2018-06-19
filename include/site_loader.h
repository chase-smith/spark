#ifndef SITE_LOADER_INCLUDE
#define SITE_LOADER_INCLUDE
#include "site_configuration.h"
#include "site_content.h"

// site_loader contains functions for loading a site's files into memory.
// Files are loaded based off of the CONTENT_BASE_DIR configuration variable.

// =======================
// = site_loader functions
// =======================

// -----------------------------------------------------------
// - Functions for loading specific parts of the site.
// - It is unlikely that you will need to call these directly,
// - instead look at load_site_content farther down.
// -----------------------------------------------------------

// Loads the dark and bright themes.
// Returns 0 on error.
int load_themes(configuration_struct* configuration, site_content_struct* site_content);

// Loads the HTML component files (header, footer, trailer).
// Returns 0 on error.
int load_html_components(configuration_struct* configuration, site_content_struct* site_content);

// Loads all series in.
// Returns 0 on error.
int load_series(configuration_struct* configuration, site_content_struct* site_content);

// Loads all misc pages in.
// Returns 0 on error.
int load_misc_pages(configuration_struct* configuration, site_content_struct* site_content);

// Calculates all post dates and determines which posts can be published.
// It does this by spawning a `date` process.
// Returns 0 on error.
int load_post_dates(configuration_struct* configuration, site_content_struct* site_content);

// Loads all posts in that have the generate-post flag file present.
// Returns 0 on error.
int load_posts(configuration_struct* configuration, site_content_struct* site_content);

// -------------------
// - load_site_content
// -------------------

// Loads all site content into memory, and links all objects
// (posts, series, etc) together appropriately.
// Returns 0 on error.
int load_site_content(configuration_struct* configuration, site_content_struct* site_content);

#endif
