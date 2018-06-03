#ifndef TAG_POSTS_STRUCT_INCLUDE
#define TAG_POSTS_STRUCT_INCLUDE
#include "dobjects.h"
typedef struct tag_posts_struct {
	dstring_struct tag;
	darray_struct posts;
} tag_posts_struct;
void tag_posts_free(tag_posts_struct*);
tag_posts_struct* tag_posts_init(tag_posts_struct*);
void tag_posts_print_debug(tag_posts_struct*);
#endif
