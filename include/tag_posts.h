#ifndef TAG_POSTS_STRUCT_INCLUDE
#define TAG_POSTS_STRUCT_INCLUDE
#include "dobjects.h"

// tag_posts_struct maps a tag to a list of posts with that tag.
typedef struct tag_posts_struct {
	// The tag
	dstring_struct tag;

	// The list of post_struct*'s that contain the tag
	darray_struct posts;
} tag_posts_struct;

// ============================
// = tag_posts_struct functions
// ============================

// Cleans up the memory used by tag_posts. Does not free()
// the pointer itself.
void tag_posts_free(tag_posts_struct* tag_posts);

// Initializes the contents of tag_posts.
// Will never fail.
void tag_posts_init(tag_posts_struct* tag_posts);

// A debug function that prints out the tag and the post titles.
void tag_posts_print_debug(tag_posts_struct* tag_posts);
#endif
