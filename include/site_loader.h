#ifndef SITE_LOADER_INCLUDE
#define SITE_LOADER_INCLUDE
#include "site_configuration.h"
#include "site_content.h"

int load_themes(configuration_struct*, site_content_struct*);
int load_html_components(configuration_struct*, site_content_struct*);
int load_series(configuration_struct*, site_content_struct*);
int load_misc_pages(configuration_struct*, site_content_struct*);
int load_post_dates(configuration_struct*, site_content_struct*);
int load_posts(configuration_struct*, site_content_struct*);
#endif
