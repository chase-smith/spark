#include "site_generator.h"
int remove_old_tag_files(site_content_struct* site_content, theme_struct* theme) {
	dstring_struct tag_dir;
	if(!dstring_init(&tag_dir)) {
		fprintf(stderr, "Error generating tags, tag_dir dstring init error\n");
		return 0;
	}
	if(!dstring_append(&tag_dir, theme->html_base_dir.str)
		|| !dstring_append(&tag_dir, "/tags/")) {
		fprintf(stderr, "Error generating tags, tag_dir dstring append error\n");
		dstring_free(&tag_dir);
		return 0;
	}
	darray_struct* html_files = get_html_filenames_in_directory(tag_dir.str);
	if(html_files == NULL) {
		fprintf(stderr, "Error generating tags, couldn't get HTML filenames in tag directory\n");
		dstring_free(&tag_dir);
		return 0;
	}
	for(size_t i = 0; i < html_files->length; i++) {
		// Index file won't be found in tags list, so skip it.
		if(!strcmp("index.html", ((dstring_struct*) darray_get_elem(html_files, i))->str)) {
			continue;
		}
		// Technically I could find the tag using strncmp, but I don't think
		// that's a necessary optimization at this point.
		dstring_remove_num_chars_in_text(((dstring_struct*) darray_get_elem(html_files, i)), ".html");
		tag_posts_struct* tag = find_tag_posts_by_tag(site_content, ((dstring_struct*) darray_get_elem(html_files, i))->str);
		if(tag == NULL) {
			// Not found. Remove file.
			if(!dstring_append((dstring_struct*) darray_get_elem(html_files, i), ".html")) {
				fprintf(stderr, "Error generating tags, dstring append error\n");
				dstring_free(&tag_dir);
				darray_of_dstrings_free(html_files);
				free(html_files);
				return 0;
			}
			if(!remove_file_in_directory(&tag_dir, ((dstring_struct*) darray_get_elem(html_files, i))->str)) {
				fprintf(stderr, "Error generating tags, couldn't remove old file\n");
				dstring_free(&tag_dir);
				darray_of_dstrings_free(html_files);
				free(html_files);
				return 0;
			}
		}
		
	}
	darray_of_dstrings_free(html_files);
	free(html_files);
	dstring_free(&tag_dir);
	return 1;
}


int generate_tags(site_content_struct* site_content) {
	// Remove files that won't be generated
	if(!remove_old_tag_files(site_content, &site_content->bright_theme)
		|| !remove_old_tag_files(site_content, &site_content->dark_theme)) {
		fprintf(stderr, "Error removing old tag files\n");
		return 0;
	}
	// Generate each tag listing
	for(size_t i = 0; i < site_content->tags.length; i++) {
		tag_posts_struct* tag_posts = (tag_posts_struct*) darray_get_elem(&site_content->tags, i);
		misc_page_struct tag_page;
		if(!misc_page_init(&tag_page)) {
			fprintf(stderr, "Error generating tags, misc_page init error\n");
			return 0;
		}
		if(!dstring_append(&tag_page.filename, "tags/")
			|| !dstring_append(&tag_page.filename, tag_posts->tag.str)
			|| !dstring_append(&tag_page.filename, ".html")
			|| !dstring_append(&tag_page.title, tag_posts->tag.str)
			|| !dstring_append(&tag_page.title, " tag listing")
			|| !dstring_append(&tag_page.description, tag_page.title.str)
			|| !dstring_append(&tag_page.content, "<header><h1>Tag: ")
			|| !dstring_append(&tag_page.content, tag_posts->tag.str)
			|| !dstring_append(&tag_page.content, "</h1></header>\n<section>\n")) {
			fprintf(stderr, "Error generating tags, dstring append error\n");
			misc_page_free(&tag_page);
			return 0;
		}
		for(size_t j = 0; j < tag_posts->posts.length; j++) {
			
			post_struct* post = *(((post_struct**) darray_get_elem(&tag_posts->posts, j)));
			if(!dstring_append(&tag_page.content, "<div><h3><a href='/posts/")
				|| !dstring_append(&tag_page.content, post->folder_name.str)
				|| !dstring_append(&tag_page.content, "'>")
				|| !dstring_append(&tag_page.content, post->title.str)
				|| !dstring_append(&tag_page.content, "</a></h3>\n")
				|| !dstring_append(&tag_page.content, "<p>")
				|| !dstring_append(&tag_page.content, post->long_description.str)
				|| !dstring_append(&tag_page.content, "</p>\n")
				|| !dstring_append(&tag_page.content, "</div>\n")) {
				fprintf(stderr, "Error generating tags, post dstring append error\n");
				misc_page_free(&tag_page);
				return 0;
			}
		}
		if(!dstring_append(&tag_page.content, "</section>\n")) {
			fprintf(stderr, "Error generating tags, dstring append error\n");
			misc_page_free(&tag_page);
			return 0;
		}
		if(!create_misc_page(site_content, &tag_page)) {
			fprintf(stderr, "Error generating page for %s\n", tag_posts->tag.str);
			misc_page_free(&tag_page);
			return 0;
		}
		misc_page_free(&tag_page);
		
	}
	// Generate the index page last; this is so that if anything fails
	// with generating the individual tag pages, the index page won't have
	// invalid links.
	misc_page_struct tags_page;
	if(!misc_page_init(&tags_page)) {
		fprintf(stderr, "Error generating tags, misc_page init error\n");
		return 0;
	}
	if(!dstring_append(&tags_page.filename, "tags/index.html")
		|| !dstring_append(&tags_page.title, "All tags")
		|| !dstring_append(&tags_page.description, "All tags")
		|| !dstring_append(&tags_page.content, "<header><h1>All tags</h1></header>\n<section>\n")) {
		fprintf(stderr, "Error generating tags, dstring append error\n");
		misc_page_free(&tags_page);
		return 0;
	}
	for(size_t i = 0; i < site_content->tags.length; i++) {
		tag_posts_struct* tag_posts = (tag_posts_struct*) darray_get_elem(&site_content->tags, i);
		if(!dstring_append(&tags_page.content, "<div><a href='/tags/")
			|| !dstring_append(&tags_page.content, tag_posts->tag.str)
			|| !dstring_append(&tags_page.content, "'>")
			|| !dstring_append(&tags_page.content, tag_posts->tag.str)
			|| !dstring_append(&tags_page.content, "</a> (")
			|| !dstring_append_printf(&tags_page.content, "%d", tag_posts->posts.length)
			|| !dstring_append(&tags_page.content, ")</div>\n")) {
			fprintf(stderr, "Error generating tags, dstring append error\n");
			misc_page_free(&tags_page);
			return 0;
		}
	}
	if(!dstring_append(&tags_page.content, "</section>\n")) {
		fprintf(stderr, "Error generating tags, dstring append error\n");
		misc_page_free(&tags_page);
		return 0;
	}
	if(!create_misc_page(site_content, &tags_page)) {
		fprintf(stderr, "Error generating tags, couldn't create tags listing page\n");
		misc_page_free(&tags_page);
		return 0;
	}
	misc_page_free(&tags_page);
	return 1;
	
}
int updated_post_sort_compare(const void* post_a, const void* post_b) {
	time_t post_a_time = ((post_struct*)post_a)->written_date_time > ((post_struct*)post_a)->updated_at_time ? ((post_struct*)post_a)->written_date_time : ((post_struct*)post_a)->updated_at_time;
	time_t post_b_time = ((post_struct*)post_b)->written_date_time > ((post_struct*)post_b)->updated_at_time ? ((post_struct*)post_b)->written_date_time : ((post_struct*)post_b)->updated_at_time;
	return post_b_time - post_a_time;
}
int generate_index_page(site_content_struct* site_content, misc_page_struct* index_page_original) {
	misc_page_struct index_page;
	if(!misc_page_init(&index_page)) {
		fprintf(stderr, "Error generating index page, misc_page_init error\n");
		return 0;
	}
	darray_struct* new_posts = darray_clone(&site_content->posts);
	if(new_posts == NULL) {
		fprintf(stderr, "Error generating index page, couldn't clone posts\n");
		misc_page_free(&index_page);
		return 0;
	}
	qsort(new_posts->array, new_posts->length, new_posts->elem_size, &updated_post_sort_compare);
	if(!dstring_append(&index_page.filename, "index.html")
		|| !dstring_append(&index_page.title, index_page_original->title.str)
		|| !dstring_append(&index_page.description, index_page_original->description.str)
		|| !dstring_append(&index_page.content, "<article>\n")
		|| !dstring_append(&index_page.content, index_page_original->content.str)
		|| !dstring_append(&index_page.content, "<section>\n")
		|| !dstring_append(&index_page.content, "<h2>New and updated posts</h2>\n")) {
		fprintf(stderr, "Error generating index page, dstring append error\n");
		misc_page_free(&index_page);
		darray_free(new_posts);
		free(new_posts);
		return 0;
	}
	// TODO: This should be a configuration setting
	const int SHOW_X_NEW_POSTS = 5;
	int num_shown = 0;
	for(size_t i = 0; i < new_posts->length; i++) {
		post_struct* post = (post_struct*) darray_get_elem(new_posts, i);
		if(!post->can_publish) continue;
		if(num_shown >= SHOW_X_NEW_POSTS) break;
		if(!dstring_append(&index_page.content, "<div><h3><a href=\"/posts/")
			|| !dstring_append(&index_page.content, post->folder_name.str)
			|| !dstring_append(&index_page.content, "\">")
			|| !dstring_append(&index_page.content, post->title.str)
			|| !dstring_append(&index_page.content, "</a></h3>\n<p>")
			|| !dstring_append(&index_page.content, post->long_description.str)
			|| !dstring_append(&index_page.content, "</p>\n<ul>\n")
			|| !dstring_append(&index_page.content, "<li>Written: ")
			|| !dstring_append(&index_page.content, post->written_date.str)
			|| !dstring_append(&index_page.content, "</li>\n")) {
			fprintf(stderr, "Error generating index page, dstring append error\n");
			misc_page_free(&index_page);
			darray_free(new_posts);	
			free(new_posts);
			return 0;
		}
		if(post->updated_at.length > 0) {
			if(!dstring_append(&index_page.content, "<li>Updated at: ")
				|| !dstring_append(&index_page.content, post->updated_at.str)
				|| !dstring_append(&index_page.content, "</li>\n")) {
				fprintf(stderr, "Error generating index page, dstring append error\n");
				misc_page_free(&index_page);
				darray_free(new_posts);
				free(new_posts);
				return 0;
			}
		}
		if(!dstring_append(&index_page.content, "<li>Series: <a href='/series/")
			|| !dstring_append(&index_page.content, post->series_name.str)
			|| !dstring_append(&index_page.content, "'>")
			|| !dstring_append(&index_page.content, post->series->title.str)
			|| !dstring_append(&index_page.content, "</a></li>\n")
			|| !dstring_append(&index_page.content, "</ul>\n</div>\n")) {
			fprintf(stderr, "Error generating index page, dstring append error\n");
			misc_page_free(&index_page);
			darray_free(new_posts);
			free(new_posts);
			return 0;
		}
		num_shown++;
	}
	if(!dstring_append(&index_page.content, "</section>\n</article>\n")) {
		fprintf(stderr, "Error generating index page, dstring append error\n");
		misc_page_free(&index_page);
		darray_free(new_posts);
		free(new_posts);
		return 0;
	}

	if(!create_misc_page(site_content, &index_page)) {
		fprintf(stderr, "Error generating index page\n");
		misc_page_free(&index_page);
		darray_free(new_posts);
		free(new_posts);
		return 0;
	}			
	misc_page_free(&index_page);
	darray_free(new_posts);
	free(new_posts);
	return 1;
}
int generate_misc_pages(site_content_struct* site_content) {
	for(size_t i = 0; i < site_content->misc_pages.length; i++) {
		misc_page_struct* misc_page = (misc_page_struct*) darray_get_elem(&site_content->misc_pages, i);
		if(!strcmp(misc_page->filename.str, "index.html")) {
			if(!generate_index_page(site_content, misc_page)) {
				fprintf(stderr, "Error generating index page\n");
				return 0;
			}
		} else {
			if(!create_misc_page(site_content, misc_page)) {
				fprintf(stderr, "Error generating misc_page %s\n", misc_page->filename.str);
				return 0;
			}
		}
	}
	return 1;
}
int generate_posts(site_content_struct* site_content) {
	// TODO: Remove old posts
	for(size_t i = 0; i < site_content->posts.length; i++) {
		post_struct* post = (post_struct*) darray_get_elem(&site_content->posts, i);
		if(!create_post_page(site_content, post)) {
			fprintf(stderr, "Error generating post %s\n", post->title.str);
			return 0;
		}
	}
	return 1;
}
int make_series_dir(series_struct* series, theme_struct* theme) {
	if(!dstring_append(&theme->html_base_dir, "/series/")) {
		fprintf(stderr, "Error making series dir, dstring append error\n");
		return 0;
	}
	if(!make_directory(&theme->html_base_dir, series->folder_name.str)) {
		fprintf(stderr, "Error making series dir for %s\n", series->folder_name.str);
		dstring_remove_num_chars_in_text(&theme->html_base_dir, "/series/");
		return 0;
	}
	dstring_remove_num_chars_in_text(&theme->html_base_dir, "/series/");
	return 1;
}
int generate_series(site_content_struct* site_content) {
	// TODO: Remove old series pages
	misc_page_struct series_listing_page;
	if(!misc_page_init(&series_listing_page)) {
		fprintf(stderr, "Error generating series, misc_page init error\n");
		return 0;
	}
	if(!dstring_append(&series_listing_page.filename, "series/index.html")
		|| !dstring_append(&series_listing_page.description, "List of all series")
		|| !dstring_append(&series_listing_page.title, "All series")
		|| !dstring_append(&series_listing_page.content, "<header><h1>Post series</h1></header>\n")) {
		fprintf(stderr, "Error generating series, series_listing_page append error\n");
		misc_page_free(&series_listing_page);
		return 0;
	}
	for(size_t i = 0; i < site_content->series.length; i++) {
		series_struct* series = (series_struct*) darray_get_elem(&site_content->series, i);
		misc_page_struct series_page;
		if(!misc_page_init(&series_page)) {
			fprintf(stderr, "Error generating series, misc_page_init error\n");
			misc_page_free(&series_listing_page);
			return 0;
		}
		if(!dstring_append(&series_listing_page.content, "<section>\n")
			|| !dstring_append(&series_listing_page.content, "<h3><a href=\"/series/")
			|| !dstring_append(&series_listing_page.content, series->folder_name.str)
			|| !dstring_append(&series_listing_page.content, "\">")
			|| !dstring_append(&series_listing_page.content, series->title.str)
			|| !dstring_append(&series_listing_page.content, "</a></h3>\n")
			|| !dstring_append(&series_listing_page.content, "<p>")
			|| !dstring_append(&series_listing_page.content, series->short_description.str)
			|| !dstring_append(&series_listing_page.content, "</p>\n")
			|| !dstring_append(&series_listing_page.content, "</section>\n")) {
			fprintf(stderr, "Error generating series, error appending to series listing page\n");
			misc_page_free(&series_listing_page);
			misc_page_free(&series_page);
			return 0;
		}
		if(!dstring_append(&series_page.filename, "series/")
			|| !dstring_append(&series_page.filename, series->folder_name.str)
			|| !dstring_append(&series_page.filename, "/index.html")
			|| !dstring_append(&series_page.description, "Landing page for ")
			|| !dstring_append(&series_page.description, series->title.str)
			|| !dstring_append(&series_page.title, series->title.str)
			|| !dstring_append(&series_page.title, " listing")
			|| !dstring_append(&series_page.content, "<header><h1>")
			|| !dstring_append(&series_page.content, series->title.str)
			|| !dstring_append(&series_page.content, "</h1></header>\n")
			|| !dstring_append(&series_page.content, "<p>")
			|| !dstring_append(&series_page.content, series->landing_desc_html.str)
			|| !dstring_append(&series_page.content, "</p><br />\n")
			|| !dstring_append(&series_page.content, "<section>\n")) {
			fprintf(stderr, "Error generating series, dstring_append error\n");
			misc_page_free(&series_page);
			misc_page_free(&series_listing_page);
			return 0;
		}
		for(size_t j = 0; j < series->posts.length; j++) {
			post_struct* post = *(post_struct**) darray_get_elem(&series->posts, j);
			if(!dstring_append(&series_page.content, "<div><h3><a href=\"/posts/")
				|| !dstring_append(&series_page.content, post->folder_name.str)
				|| !dstring_append(&series_page.content, "\">")
				|| !dstring_append(&series_page.content, post->title.str)
				|| !dstring_append(&series_page.content, "</a></h3>\n")
				|| !dstring_append(&series_page.content, "<p>\n")
				|| !dstring_append(&series_page.content, post->long_description.str)
				|| !dstring_append(&series_page.content, "</p></div>\n")) {
				fprintf(stderr, "Error generating series, post dstring_append error\n");
				misc_page_free(&series_page);
				misc_page_free(&series_listing_page);
				return 0;
			}
		}
		if(!dstring_append(&series_page.content, "</section>\n")) {
			fprintf(stderr, "Error generating series, dstring append error\n");
			misc_page_free(&series_page);
			misc_page_free(&series_listing_page);
			return 0;
		}
		if(!make_series_dir(series, &site_content->bright_theme)
			|| !make_series_dir(series, &site_content->dark_theme)) {
			fprintf(stderr, "Error generating series, couldn't make series directories\n");
			misc_page_free(&series_page);
			misc_page_free(&series_listing_page);
			return 0;
		}
		if(!create_misc_page(site_content, &series_page)) {
			fprintf(stderr, "Error generating series, couldn't generate pages\n");
			misc_page_free(&series_page);
			misc_page_free(&series_listing_page);
			return 0;
		}
		misc_page_free(&series_page);
	}
	if(!create_misc_page(site_content, &series_listing_page)) {
		fprintf(stderr, "Error generating series, couldn't generate series_listing_page\n");
		misc_page_free(&series_listing_page);
		return 0;
	}
	misc_page_free(&series_listing_page);

	
	return 1;
}

// Generates the sitemap.html page, which lists all posts on a single page
int generate_sitemap(site_content_struct* site_content) {
	misc_page_struct sitemap;
	if(!misc_page_init(&sitemap)) {
		fprintf(stderr, "Error generating sitemap, misc_page_init error\n");
		return 0;
	}
	if(!dstring_append(&sitemap.title, "Sitemap")
		|| !dstring_append(&sitemap.description, "Sitemap")
		|| !dstring_append(&sitemap.filename, "sitemap.html")
		|| !dstring_append(&sitemap.content, "<header><h1>All posts</h1></header>\n")) {
		fprintf(stderr, "Error generating sitemap, dstring append error\n");
		misc_page_free(&sitemap);
		return 0;
	}
	
	for(size_t i = 0; i < site_content->series.length; i++) {
		series_struct* series = (series_struct*) darray_get_elem(&site_content->series, i);
		if(!dstring_append(&sitemap.content, "<header><h2>")
			|| !dstring_append(&sitemap.content, series->title.str)
			|| !dstring_append(&sitemap.content, "</h2></header>\n")
			|| !dstring_append(&sitemap.content, "<p>")
			|| !dstring_append(&sitemap.content, series->landing_desc_html.str)
			|| !dstring_append(&sitemap.content, "</p><br />\n")
			|| !dstring_append(&sitemap.content, "<section>\n")) {
			fprintf(stderr, "Error generating sitemap, dstring_append error\n");
			misc_page_free(&sitemap);
			return 0;
		}
		for(size_t j = 0; j < series->posts.length; j++) {
			post_struct* post = *(post_struct**) darray_get_elem(&series->posts, j);
			if(!dstring_append(&sitemap.content, "<div><h3><a href=\"/posts/")
				|| !dstring_append(&sitemap.content, post->folder_name.str)
				|| !dstring_append(&sitemap.content, "\">")
				|| !dstring_append(&sitemap.content, post->title.str)
				|| !dstring_append(&sitemap.content, "</a></h3>\n")
				|| !dstring_append(&sitemap.content, "<p>\n")
				|| !dstring_append(&sitemap.content, post->long_description.str)
				|| !dstring_append(&sitemap.content, "</p></div>\n")) {
				fprintf(stderr, "Error generating series, post dstring_append error\n");
				misc_page_free(&sitemap);
				return 0;
			}
		}
		if(!dstring_append(&sitemap.content, "</section>\n")) {
			fprintf(stderr, "Error generating sitemap, dstring append error\n");
			misc_page_free(&sitemap);
			return 0;
		}
	}
	if(!create_misc_page(site_content, &sitemap)) {
		fprintf(stderr, "Error generating sitemap, couldn't create page\n");
		misc_page_free(&sitemap);
		return 0;
	}
	misc_page_free(&sitemap);
	return 1;
}
// TODO FIXME: This assumes that rss_dstring is a fully formed RSS object.
int write_rss_file_if_different(dstring_struct* rss_dstring, const char* filename, int* did_write) {
	(*did_write) = 0;
	char* lastBuildDate_start = strstr(rss_dstring->str, "lastBuildDate");
	if(lastBuildDate_start == NULL) {
		fprintf(stderr, "Error with RSS file, couldn't find lastBuildDate\n");
		return 0;
	}
	dstring_struct file_contents;
	if(!dstring_init(&file_contents)) {
		fprintf(stderr, "Error writing file, dstring init error\n");
		return 0;
	}
	int need_to_write = 1;
	if(!access(filename, F_OK)) {
		if(!dstring_read_file(&file_contents, filename)) {
			fprintf(stderr, "Error writing file, couldn't read existing file\n");
			dstring_free(&file_contents);
			return 0;
		}
		char* new_ptr = rss_dstring->str;
		char* old_ptr = file_contents.str;
		while(new_ptr != lastBuildDate_start) {
			if(*(new_ptr++) != *(old_ptr++)) {
				break;
			}
		}
		while(*new_ptr != '<') {
			++new_ptr;
			++old_ptr;
		}
		if(!strcmp(new_ptr, old_ptr)) {
			need_to_write = 0;
		}
	} else {
		printf("Creating file %s as it doesn't exist\n", filename);
	}
	if(need_to_write) {
		if(!dstring_write_file(rss_dstring, filename)) {
			fprintf(stderr, "Error writing file %s\n", filename);
			dstring_free(&file_contents);
			return 0;
		}
		(*did_write) = 1;
	}
	dstring_free(&file_contents);
	return 1;
}

int generate_main_rss(configuration_struct* configuration, site_content_struct* site_content) {
	time_t now_time;
	time(&now_time);
	struct tm* time_struct = gmtime(&now_time);
	if(time_struct == NULL) {
		fprintf(stderr, "Error generating RSS, couldn't get time\n");
		return 0;
	}
	char buff[51];
	size_t strftime_res = strftime(buff, 50, "%a, %d %b %Y %T %z", time_struct);
	if(strftime_res == 0) {
		fprintf(stderr, "Error generating RSS, couldn't strftime\n");
		return 0;
	}
	dstring_struct rss_feed;
	if(!dstring_init(&rss_feed)) {
		fprintf(stderr, "Error generating RSS, couldn't init rss dstring\n");
		return 0;
	}
	if(!dstring_append(&rss_feed, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n")
		|| !dstring_append(&rss_feed, "<rss version=\"2.0\">\n")
		|| !dstring_append(&rss_feed, "<channel>\n")
		|| !dstring_append(&rss_feed, "<lastBuildDate>")
		|| !dstring_append(&rss_feed, buff)
		|| !dstring_append(&rss_feed, "</lastBuildDate>\n")
		|| !dstring_append(&rss_feed, "<title>")
		|| !dstring_append(&rss_feed, configuration->bright_host)
		|| !dstring_append(&rss_feed, " posts</title>\n")
		|| !dstring_append(&rss_feed, "<link>https://")
		|| !dstring_append(&rss_feed, configuration->bright_host)
		|| !dstring_append(&rss_feed, "/</link>\n")
		|| !dstring_append(&rss_feed, "<description>")
		|| !dstring_append(&rss_feed, configuration->rss_description)
		|| !dstring_append(&rss_feed, "</description>\n")) {
		fprintf(stderr, "Error generating RSS, dstring append error\n");
		dstring_free(&rss_feed);
		return 0;
	}
	for(size_t i = 0; i < site_content->posts.length; i++) {
		post_struct* post = (post_struct*) darray_get_elem(&site_content->posts, i);
		struct tm* post_time_struct = gmtime(&post->written_date_time);
		if(post_time_struct == NULL) {
			fprintf(stderr, "Error generating RSS, couldn't get post time\n");
			dstring_free(&rss_feed);
			return 0;
		}
		char post_buff[51];
		size_t strftime_res2 = strftime(post_buff, 50, "%a, %d %b %Y %T %z", post_time_struct);
		if(strftime_res2 == 0) {
			fprintf(stderr, "Error generating RSS, couldn't strftime for post\n");
			dstring_free(&rss_feed);
			return 0;
		}
		if(!post->can_publish) continue;
		if(!dstring_append(&rss_feed, "<item>\n")
			|| !dstring_append(&rss_feed, "<title>")
			|| !dstring_append(&rss_feed, post->title.str)
			|| !dstring_append(&rss_feed, "</title>\n")
			|| !dstring_append(&rss_feed, "<category>")
			|| !dstring_append(&rss_feed, post->series->title.str)
			|| !dstring_append(&rss_feed, "</category>\n")
			|| !dstring_append(&rss_feed, "<pubDate>")
			|| !dstring_append(&rss_feed, post_buff)
			|| !dstring_append(&rss_feed, "</pubDate>\n")
			|| !dstring_append(&rss_feed, "<link>https://")
			|| !dstring_append(&rss_feed, configuration->bright_host)
			|| !dstring_append(&rss_feed, "/posts/")
			|| !dstring_append(&rss_feed, post->folder_name.str)
			|| !dstring_append(&rss_feed, "</link>\n")
			|| !dstring_append(&rss_feed, "<description>")
			|| !dstring_append(&rss_feed, post->long_description.str)
			|| !dstring_append(&rss_feed, "</description>\n")
			|| !dstring_append(&rss_feed, "</item>\n")) {
			fprintf(stderr, "Error generating RSS, dstring append error\n");
			dstring_free(&rss_feed);
			return 0;
		}
	}
	if(!dstring_append(&rss_feed, "</channel>\n")
		|| !dstring_append(&rss_feed, "</rss>\n")) {
		fprintf(stderr, "Error generating RSS, dstring append error\n");
		dstring_free(&rss_feed);
		return 0;
	}
	dstring_struct rss_filename;
	if(!dstring_init(&rss_filename)) {
		fprintf(stderr, "Error generating RSS, filename dstring init error\n");
		dstring_free(&rss_feed);
		return 0;
	}
	if(!dstring_append(&rss_filename, site_content->bright_theme.html_base_dir.str)
		|| !dstring_append(&rss_filename, "/feed.rss")) {
		fprintf(stderr, "Error generating RSS, filename dstring append error\n");
		dstring_free(&rss_feed);
		dstring_free(&rss_filename);
		return 0;
	}
	int did_write;
	if(!write_rss_file_if_different(&rss_feed, rss_filename.str, &did_write)) {
		fprintf(stderr, "Error generating RSS, couldn't write file\n");
		dstring_free(&rss_feed);
		dstring_free(&rss_filename);
		return 0;
	}
	dstring_free(&rss_feed);
	dstring_free(&rss_filename);
	return 1;
		
}

