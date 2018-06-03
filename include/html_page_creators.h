#ifndef HTML_PAGE_CREATORS_INCLUDE
#define HTML_PAGE_CREATORS_INCLUDE
#include "dobjects.h"
#include "core_objects.h"

#define GENMODE_POST 1
#define GENMODE_STATIC 2
#define PAGE_GENERATION_FAILURE 0
#define PAGE_GENERATION_NO_UPDATE 1
#define PAGE_GENERATION_UPDATED 2
typedef struct page_generation_settings_struct {
	char* filename;
	char* keywords;
	char* description;
	char* title;
	char* author;
	char* canonical_url;
	char* url_path;
	int has_code;
} page_generation_settings_struct;


int create_page(site_content_struct*, dstring_struct*, theme_struct*, page_generation_settings_struct*);
int create_page_wrapper(site_content_struct*, dstring_struct*, page_generation_settings_struct*, int);
int create_misc_page(site_content_struct*, misc_page_struct*);
int create_post_page_append_recommended_readings(dstring_struct*, darray_struct*, const char*);
int create_post_page(site_content_struct*, post_struct*);
#endif
