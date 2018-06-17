#ifndef SITE_GENERATOR_INCLUDE
#define SITE_GENERATOR_INCLUDE
#include "dobjects.h"
#include "core_objects.h"
#include "file_helpers.h"
#include "html_page_creators.h"
#include "site_configuration.h"
#include "site_loader.h"

// The functions in site_generator are responsible for taking the site_content
// and creating the HTML files for the site.

// ==========================
// = site_generator functions
// ==========================

// -----------------------------------------------------------
// - Functions for generating specific parts of the site.
// - It is unlikely that you will need to call these directly,
// - instead look at the entry functions farther down.
// -----------------------------------------------------------

// Generates a page for each tag, as well as the tag listing page.
// Returns 0 on error.
int generate_tags(site_content_struct* site_content);

// Generates all misc_pages on the site, including the index page (home page).
// Returns 0 on error.
int generate_misc_pages(site_content_struct* site_content);

// Generates all posts on the site.
// Returns 0 on error.
int generate_posts(site_content_struct* site_content);

// Generates each series landing page, as well as the page which lists
// all series.
// Returns 0 on error.
int generate_series(site_content_struct* site_content);

// Generates an HTML page with links to every (publishable) post.
// Note, this function may be deprecated or changed significantly
// in the future.
// Returns 0 on error.
int generate_sitemap(site_content_struct* site_content);

// Generates the main RSS feed file for the site.
// Returns 0 on error.
int generate_main_rss(configuration_struct* configuration, site_content_struct* site_content);

// --------------------------------------------------
// - Entry functions; these generate the entire site.
// --------------------------------------------------

// Loads and generates the entire site.
// Note, this function is going to change significantly in the future.
// A better API will be created for generating sites.
// Returns 0 on error.
int generate_site(configuration_struct* configuration);

#endif
