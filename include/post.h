#ifndef POST_STRUCT_INCLUDE
#define POST_STRUCT_INCLUDE
#include "dobjects.h"
#include "series.h"
#include "file_helpers.h"

// No field is needed for generate_post, because posts that won't be generated
// won't be loaded in.
typedef struct post_struct {
	dstring_struct folder_name;
	dstring_struct title;
	dstring_struct content;
	dstring_struct author;
	dstring_struct raw_tags;
	darray_struct tags; 
	dstring_struct series_name;
	dstring_struct short_description;
	dstring_struct long_description;
	dstring_struct raw_suggested_next_reading;
	dstring_struct raw_suggested_prev_reading;
	darray_struct suggested_next_reading_names;
	darray_struct suggested_prev_reading_names;
	darray_struct suggested_next_reading;
	darray_struct suggested_prev_reading;
	dstring_struct written_date;
	time_t written_date_time;
	int publish_when_ready;
	int has_code;
	dstring_struct publish_after;
	time_t publish_after_time;
	dstring_struct updated_at;
	time_t updated_at_time;
	int can_publish;
	series_struct* series;
} post_struct;

void post_free(post_struct*);
void post_init(post_struct*);
void post_print_debug(post_struct*);
post_struct* post_load(post_struct*, dstring_struct*, const char*, int*);
#endif
