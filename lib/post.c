#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include "dobjects.h"
#include "post.h"

void post_free(post_struct* post) {
	dstring_free(&post->folder_name);
	dstring_free(&post->title);
	dstring_free(&post->content);
	dstring_free(&post->author);
	dstring_free(&post->raw_tags);
	// I do NOT need to free the strings in tags, because they are pointing to
	// strings inside of raw_tags. Same applies for suggested_next/prev_reading
	darray_free(&post->tags);
	dstring_free(&post->series_name);
	dstring_free(&post->short_description);
	dstring_free(&post->long_description);
	dstring_free(&post->raw_suggested_next_reading);
	dstring_free(&post->raw_suggested_prev_reading);
	darray_free(&post->suggested_next_reading);
	darray_free(&post->suggested_prev_reading);
	darray_free(&post->suggested_next_reading_names);
	darray_free(&post->suggested_prev_reading_names);
	dstring_free(&post->written_date);
	dstring_free(&post->publish_after);
	dstring_free(&post->updated_at);
}
post_struct* post_init(post_struct* post) {
	post->publish_when_ready = 0;
	post->can_publish = 0;
	post->has_code = 0;
	post->written_date_time = 0;
	post->publish_after_time = 0;
	post->updated_at_time = 0;
	post->series = NULL;
	dstring_lazy_init(&post->folder_name);
	dstring_lazy_init(&post->title);
	dstring_lazy_init(&post->content);
	dstring_lazy_init(&post->author);
	dstring_lazy_init(&post->raw_tags);
	dstring_lazy_init(&post->series_name);
	dstring_lazy_init(&post->short_description);
	dstring_lazy_init(&post->long_description);
	dstring_lazy_init(&post->raw_suggested_next_reading);
	dstring_lazy_init(&post->raw_suggested_prev_reading);
	dstring_lazy_init(&post->written_date);
	dstring_lazy_init(&post->publish_after);
	dstring_lazy_init(&post->updated_at);
	darray_lazy_init(&post->tags, sizeof(char*));
	darray_lazy_init(&post->suggested_next_reading_names, sizeof(char*));
	darray_lazy_init(&post->suggested_prev_reading_names, sizeof(char*));
	darray_lazy_init(&post->suggested_next_reading, sizeof(post_struct*));
	darray_lazy_init(&post->suggested_prev_reading, sizeof(post_struct*));
	return post;
}
void post_print_debug(post_struct* post) {
	printf("Folder name: %s\n", post->folder_name.str);
	printf("Title: %s\n", post->title.str);
	printf("Author: %s\n", post->author.str);
	printf("Series: %s\n", post->series_name.str);
	printf("Written date: %s\n", post->written_date.str);
	printf("Publish after: %s\n", post->publish_after.str);
	printf("Updated at: %s\n", post->updated_at.str);
}
// GENERAL TODO: Perhaps if series and misc_pages can't be loaded properly,
// that's an error?

// This function takes a third parameter, compared to most of the others,
// that is a pointer to an integer; this integer will be set to 1 if the
// post failed to be loaded because the generate flag was missing (which is
// not a critical failure, it should be ignored), and 0 if the post failed
// to be loaded due to a critical failure.
// base_dir, as always, shouldn't have a trailing slash.
post_struct* post_load(post_struct* post, dstring_struct* base_dir, const char* folder_name, int* generate_flag_missing) {
	(*generate_flag_missing) = 0;
	// First, check for the existence of the generate flag.
	if(!check_if_file_exists(base_dir, "/generate-post")) {
		// Post will not be loaded because it's not to be generated
		(*generate_flag_missing) = 1;
		return NULL;
	}
	if(base_dir->length == 0) {
		fprintf(stderr, "Can't load post, no base directory\n");
		return NULL;
	}
	if(!dstring_append(&post->folder_name, folder_name)) {
		fprintf(stderr, "Error loading post, folder_name dstring append error\n");
		return NULL;
	}
	if(!dstring_try_load_file(&post->title, base_dir, "/title", "post")
		|| !dstring_try_load_file(&post->content, base_dir, "/content.html", "post")
		|| !dstring_try_load_file(&post->author, base_dir, "/author", "post")
		|| !dstring_try_load_file(&post->raw_tags, base_dir, "/tags", "post")
		|| !dstring_try_load_file(&post->series_name, base_dir, "/series", "post")
		|| !dstring_try_load_file(&post->short_description, base_dir, "/short-description", "post")
		|| !dstring_try_load_file(&post->long_description, base_dir, "/long-description", "post")
		|| !dstring_try_load_file(&post->written_date, base_dir, "/written-date", "post")) {
		return NULL;
	}
	dstring_remove_trailing_newlines(&post->title);
	dstring_remove_trailing_newlines(&post->author);
	dstring_remove_trailing_newlines(&post->series_name);
	dstring_remove_trailing_newlines(&post->raw_tags);
	dstring_remove_trailing_newlines(&post->short_description);
	dstring_remove_trailing_newlines(&post->long_description);
	dstring_remove_trailing_newlines(&post->written_date);
	if(!dstring_split_to_darray(&post->raw_tags, &post->tags, ',')) {
		fprintf(stderr, "Error loading post, couldn't split tags\n");
		return NULL;
	}
	// Optional files now; if these fail to load, that is fine. Now, technically,
	// I should probably check to see if the files exist first, and then if there's
	// a file load error, then bail out because it would likely only fail if there was
	// a memory issue or something. TODO for later, I suppose.
	if(check_if_file_exists(base_dir, "/suggested-next-reading")) {
		if(!dstring_try_load_file(&post->raw_suggested_next_reading, base_dir, "/suggested-next-reading", "post optional")) {
			fprintf(stderr, "Error loading post, unable to read suggested-next-reading file\n");
			return NULL;
		}
		if(!dstring_split_to_darray(&post->raw_suggested_next_reading, &post->suggested_next_reading_names, '\n')) {
			fprintf(stderr, "Error loading post, couldn't split suggested next reading\n");
			return NULL;
		}
	}
	if(check_if_file_exists(base_dir, "/suggested-prev-reading")) {
		if(!dstring_try_load_file(&post->raw_suggested_prev_reading, base_dir, "/suggested-prev-reading", "post optional")) {
			fprintf(stderr, "Error loading post, unable to read suggested-prev-reading file\n");
			return NULL;
		}
		if(!dstring_split_to_darray(&post->raw_suggested_prev_reading, &post->suggested_prev_reading_names, '\n')) {
			fprintf(stderr, "Error loading post, couldn't split suggested next reading\n");
			return NULL;
		}

	}
	if(check_if_file_exists(base_dir, "/updated-at")) {
		if(!dstring_try_load_file(&post->updated_at, base_dir, "/updated-at", "post optional")) {
			fprintf(stderr, "Error loading post, unable to read updated-at file\n");
			return NULL;
		}
		dstring_remove_trailing_newlines(&post->updated_at);
	}
	if(check_if_file_exists(base_dir, "/publish-after")) {
		if(!dstring_try_load_file(&post->publish_after, base_dir, "/publish-after", "post optional")) {
			fprintf(stderr, "Error loading post, unable to read publish-after file\n");
			return NULL;
		}
		dstring_remove_trailing_newlines(&post->publish_after);
	}
	
	if(check_if_file_exists(base_dir, "/publish-when-ready")) {
		post->publish_when_ready = 1;
		
	}
	if(check_if_file_exists(base_dir, "/has-code")) {
		post->has_code = 1;
	}
	return post;	
}
