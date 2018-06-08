#ifndef SITE_CONTENT_STRUCT_INCLUDE
#define SITE_CONTENT_STRUCT_INCLUDE
#include "dobjects.h"
#include "post.h"
#include "misc_page.h"
#include "html_components.h"
#include "theme.h"
#include "tag_posts.h"

typedef struct site_content_struct {
	html_components_struct html_components;
	theme_struct dark_theme;
	theme_struct bright_theme;
	darray_struct misc_pages;
	darray_struct series;
	darray_struct posts;
	time_t current_time;
	darray_struct tags;
} site_content_struct;
void site_content_free(site_content_struct*);
void site_content_init(site_content_struct*);
tag_posts_struct* find_tag_posts_by_tag(site_content_struct*, const char*);
post_struct* find_post_by_folder_name(site_content_struct*, const char*);
series_struct* find_series_by_folder_name(site_content_struct*, const char*);
misc_page_struct* find_misc_page_by_filename(site_content_struct*, const char*);
int site_content_add_post_to_tag(site_content_struct*, post_struct*, const char*);
int site_content_add_post_to_tags(site_content_struct*, post_struct*);
int site_content_setup_tags(site_content_struct*);
int validate_posts(site_content_struct*);

#endif
