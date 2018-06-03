#ifndef SITE_GENERATOR_INCLUDE
#define SITE_GENERATOR_INCLUDE
#include "dobjects.h"
#include "core_objects.h"
#include "file_helpers.h"
#include "html_page_creators.h"
#include "site_configuration.h"

int remove_old_tag_files(site_content_struct*, theme_struct*);
int generate_tags(site_content_struct*);
int generate_index_page(site_content_struct*, misc_page_struct*);
int generate_misc_pages(site_content_struct*);
int generate_posts(site_content_struct*);
int make_series_dir(series_struct*, theme_struct*);
int generate_series(site_content_struct*);
int generate_sitemap(site_content_struct*);
int write_rss_file_if_different(dstring_struct*, const char*, int*);
int generate_main_rss(configuration_struct*, site_content_struct*);

#endif
