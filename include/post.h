#ifndef POST_STRUCT_INCLUDE
#define POST_STRUCT_INCLUDE
#include "dobjects.h"
#include "series.h"
#include "file_helpers.h"

// post_struct contains all of the information about a post. Some fields are
// calculated after loading in all of the post files. Not all fields will have
// values (though they will be initialized), depending on whether or not
// certain post files exist.
// No field is needed for generate_post, because posts that won't be generated
// won't be loaded in.
typedef struct post_struct {
	// The folder name (used as ID and in the URL) for the post.
	dstring_struct folder_name;

	// The title of the post, as it will appear throughout the site.
	dstring_struct title;

	// The content.html file; the body of the post.
	dstring_struct content;

	// Who wrote the article.
	dstring_struct author;

	// The raw text from the comma-delimited tags file. Do not use
	// directly, it is modified after loading.
	dstring_struct raw_tags;

	// The list of tags for the post; it is a darray of char*'s.
	// Points to null-terminated strings within raw_tags.
	darray_struct tags;

	// The series ID that this post belongs to.
	dstring_struct series_name;

	// The short description of the post, as will appear in the HTML meta
	// tag in the header.
	dstring_struct short_description;

	// The long description of the post, as it will appear on listing pages
	// on the site.
	dstring_struct long_description;

	// The raw newline-delimited list of IDs of suggested next posts to
	// read. Will be an empty string if there are none. Do not use
	// directly, it is modified after loading.
	dstring_struct raw_suggested_next_reading;

	// The raw newline-delimited list of IDs of suggested previous posts to
	// read. Will be an empty string if there are none. Do not use
	// directly, it is modified after loading.
	dstring_struct raw_suggested_prev_reading;

	// The list of IDs of suggested next posts to read. It is a darray of
	// char*'s. Points to null-terminated strings within
	// raw_suggested_next_reading.
	darray_struct suggested_next_reading_names;

	// The list of IDs of suggested previous posts to read. It is a darray
	// of char*'s. Points to null-terminated strings within
	// raw_suggested_prev_reading.
	darray_struct suggested_prev_reading_names;

	// The list of posts that are suggested to read next. It is a darray of
	// post_struct*'s.
	darray_struct suggested_next_reading;

	// The list of posts that are suggested to read prior to this post.
	// It is a darray of post_struct*'s.
	darray_struct suggested_prev_reading;

	// The raw string with the date/timestamp of when the post was written.
	dstring_struct written_date;

	// The unix timestamp of when the post was written.
	time_t written_date_time;

	// Whether or not the post should be published if the publish_after
	// timestamp is in the past (if it exists).
	int publish_when_ready;

	// Whether or not the syntax-highlighting.css styling should be
	// included for this post.
	int has_code;

	// The raw string with the date/timestamp of the earliest the post can
	// be published.
	dstring_struct publish_after;

	// The unix timestamp of the earliest the post can be published.
	time_t publish_after_time;

	// The raw string with when the post was last updated at.
	dstring_struct updated_at;

	// The unix timestamp of when the post was last updated at.
	time_t updated_at_time;

	// Whether or not the post can be published.
	int can_publish;

	// The series the post belongs in.
	series_struct* series;
} post_struct;

// =======================
// = post_struct functions
// =======================

// Cleans up the resources used by a post.
void post_free(post_struct* post);

// Initializes all of the components of a post.
void post_init(post_struct* post);

// A function useful for debugging purposes, it prints out some basic
// information about the post.
void post_print_debug(post_struct* post);

// Returns a post_struct* from a darray of post_struct's at the given index.
static inline post_struct* post_get_from_darray(darray_struct* darray, size_t index) {
	return (post_struct*) darray_get_elem(darray, index);
}

// Returns a post_struct* from a darray of post_struct*'s at the given index.
static inline post_struct* post_get_from_darray_of_post_pointers(darray_struct* darray, size_t index) {
	return *((post_struct**) darray_get_elem(darray, index));
}

// Loads a post in from the given post directory. The folder name is passed in
// as well, to save from having to recalculate it.
// Sets (*generate_flag_missing) to 1 if the generate-post flag file is
// missing, 0 otherwise.
// Returns NULL on error, or if the generate-post flag file is missing; check
// generate_flag_missing to see if this is the case; it is not a critical
// error if the flag file is missing.
post_struct* post_load(post_struct* post, dstring_struct* post_dir, const char* folder_name, int* generate_flag_missing);

#endif
