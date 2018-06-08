#include "dobjects.h"
#include "site_content.h"

void site_content_free(site_content_struct* site_content) {
	html_components_free(&site_content->html_components);
	theme_free(&site_content->dark_theme);
	theme_free(&site_content->bright_theme);
	// I DO need to free these arrays
	for(size_t i = 0; i < site_content->misc_pages.length; i++) {
		misc_page_free(&((misc_page_struct*)site_content->misc_pages.array)[i]);
	}
	darray_free(&site_content->misc_pages);
	for(size_t i = 0; i < site_content->series.length; i++) {
		series_free(&((series_struct*)site_content->series.array)[i]);
	}
	darray_free(&site_content->series);
	for(size_t i = 0; i < site_content->posts.length; i++) {
		post_free(post_get_from_darray(&site_content->posts, i));
	}
	darray_free(&site_content->posts);
	for(size_t i = 0; i < site_content->tags.length; i++) {
		tag_posts_free(&((tag_posts_struct*)site_content->tags.array)[i]);
	}
	darray_free(&site_content->tags);
}
void site_content_init(site_content_struct* site_content) {
	site_content->current_time = 0;
	darray_lazy_init(&site_content->misc_pages, sizeof(misc_page_struct));
	darray_lazy_init(&site_content->series, sizeof(series_struct));
	darray_lazy_init(&site_content->posts, sizeof(post_struct));
	darray_lazy_init(&site_content->tags, sizeof(tag_posts_struct));
	html_components_init(&site_content->html_components);
	theme_init(&site_content->dark_theme);
	theme_init(&site_content->bright_theme);
}
int site_content_add_post_to_tag(site_content_struct* site_content, post_struct* post, const char* tag) {
	tag_posts_struct* tag_posts = find_tag_posts_by_tag(site_content, tag);
	if(tag_posts == NULL) {
		// Doing this saves a malloc and free. darray_append copies the
		// contents of what it's given, so there's no danger in giving it
		// a local variable.
		tag_posts_struct tag_posts_s;
		tag_posts = &tag_posts_s;
		tag_posts_init(tag_posts);

		if(!dstring_append(&tag_posts->tag, tag)) {
			fprintf(stderr, "Error adding post to tag, dstring append error\n");
			tag_posts_free(tag_posts);
			return 0;
		}
		if(!darray_append(&site_content->tags, tag_posts)) {
			fprintf(stderr, "Error adding post to tag, couldn't add tag_posts to tags\n");
			tag_posts_free(tag_posts);
			return 0;
		}

		tag_posts = find_tag_posts_by_tag(site_content, tag);
		if(tag_posts == NULL) {
			fprintf(stderr, "Error adding post to tag, couldn't find tag after it was created\n");
			return 0;
		}
	}
	if(!darray_append(&tag_posts->posts, &post)) {
		fprintf(stderr, "Error adding post to tag, darray append error\n");
		return 0;
	}
	return 1;
}
int site_content_add_post_to_tags(site_content_struct* site_content, post_struct* post) {
	if(!post->can_publish) {
		printf("Skipping tag adding for post %s, not publishing\n", post->title.str);
		return 1;
	}
	const char** tags = (const char**) post->tags.array;
	for(size_t i = 0; i < post->tags.length; i++) {
		if(!site_content_add_post_to_tag(site_content, post, tags[i])) {
			fprintf(stderr, "Error adding post %s to tag %s\n", post->folder_name.str, tags[i]);
			return 0;
		}
	}
	return 1;
}
int site_content_setup_tags(site_content_struct* site_content) {
	for(size_t i = 0; i < site_content->posts.length; i++) {
		post_struct* post = post_get_from_darray(&site_content->posts, i);
		if(!site_content_add_post_to_tags(site_content, post)) {
			// Error printing is done in above function
			return 0;
		}
	}
	return 1;
}
post_struct* find_post_by_folder_name(site_content_struct* site_content, const char* folder_name) {
	for(size_t i = 0; i < site_content->posts.length; i++) {
		post_struct* post = post_get_from_darray(&site_content->posts, i);
		if(!strcmp(folder_name, post->folder_name.str)) {
			return post;
		}
	}
	return NULL;
}
series_struct* find_series_by_folder_name(site_content_struct* site_content, const char* folder_name) {
	for(size_t i = 0; i < site_content->series.length; i++) {
		if(!strcmp(folder_name, ((series_struct*)site_content->series.array)[i].folder_name.str)) {
			return &((series_struct*)site_content->series.array)[i];
		}
	}
	return NULL;
}
misc_page_struct* find_misc_page_by_filename(site_content_struct* site_content, const char* filename) {
	for(size_t i = 0; i < site_content->misc_pages.length; i++) {
		if(!strcmp(filename, ((misc_page_struct*)site_content->misc_pages.array)[i].filename.str)) {
			return &((misc_page_struct*)site_content->misc_pages.array)[i];
		}
	}
	return NULL;
}
tag_posts_struct* find_tag_posts_by_tag(site_content_struct* site_content, const char* tag) {
	for(size_t i = 0; i < site_content->tags.length; i++) {
		if(!strcmp(tag, ((tag_posts_struct*)site_content->tags.array)[i].tag.str)) {
			return &((tag_posts_struct*)site_content->tags.array)[i];
		}
	}
	return NULL;
}
// Validate post names, series names, etc. Sets up links for series and posts
int validate_posts(site_content_struct* site_content) {
	for(size_t i = 0; i < site_content->posts.length; i++) {
		post_struct* post = post_get_from_darray(&site_content->posts, i);
		const char** suggested_prev_reading_names_strs = (const char**) post->suggested_prev_reading_names.array;
		for(size_t j = 0; j < post->suggested_prev_reading_names.length; j++) {
			const char* prev_post_name = suggested_prev_reading_names_strs[j];
			post_struct* prev_post = find_post_by_folder_name(site_content, prev_post_name);
			if(!prev_post) {
				fprintf(stderr, "Error validating posts, previous post %s not found for post %s\n", prev_post_name, post->folder_name.str);
				return 0;
			}
			if(!darray_append(&post->suggested_prev_reading, &prev_post)) {
				fprintf(stderr, "Error validating posts, suggested_prev_reading darray append error\n");
				return 0;
			}
		}
		// TODO: This is basically identical to suggested_prev_reading
		const char** suggested_next_reading_names_strs = (const char**) post->suggested_next_reading_names.array;
		for(size_t j = 0; j < post->suggested_next_reading_names.length; j++) {
			const char* next_post_name = suggested_next_reading_names_strs[j];
			post_struct* next_post = find_post_by_folder_name(site_content, next_post_name);
			if(!next_post) {
				fprintf(stderr, "Error validating posts, next post %s not found for post %s\n", next_post_name, post->folder_name.str);
				return 0;
			}
			if(!darray_append(&post->suggested_next_reading, &next_post)) {
				fprintf(stderr, "Error validating posts, suggested_next_reading darray append error\n");
				return 0;
			}
		}
		series_struct* series = find_series_by_folder_name(site_content, post->series_name.str);
		if(!series) {
			fprintf(stderr, "Error validating posts, unknown series %s for post %s\n", post->series_name.str, post->folder_name.str);
			return 0;
		}
		post->series = series;
		if(post->can_publish) {
			if(!darray_append(&series->posts, &post)) {
				fprintf(stderr, "Error validating posts, couldn't append to series\n");
				return 0;
			}
		}
	}
	return 1;
}

