#include "site_generator.h"

int remove_nonexistent_post_single(dstring_struct* base_dir, struct dirent* dir_ent, void* site_content_void_ptr) {
	// Skip processing of index.html file
	if(!strcmp(dir_ent->d_name, "index.html")) {
		return 1;
	}

	// Skip non-HTML files
	if(!is_html_filename(dir_ent->d_name)) {
		return 1;
	}
	site_content_struct* site_content = site_content_void_ptr;

	// -5 for the .html ending
	char* post_name = strndup(dir_ent->d_name, strlen(dir_ent->d_name) - 5);
	if(!post_name) {
		fprintf(stderr, "Error removing single nonexistent post, strndup error\n");
		return 0;
	}

	// Try to find the post
	post_struct* post = find_post_by_folder_name(site_content, post_name);
	free(post_name);

	// If we found it, we aren't going to remove the file
	if(post) {
		return 1;
	}

	// Didn't find the post, remove the file
	int res = remove_file_in_directory(base_dir, dir_ent->d_name);
	if(!res) {
		fprintf(stderr, "Error removing single nonexistent post, unlink error\n");
	}
	return res;
}
int remove_nonexistent_posts_for_theme(site_content_struct* site_content, theme_struct* theme) {
	dstring_struct base_dir;
	dstring_lazy_init(&base_dir);
	if(!dstring_append_printf(&base_dir, "%s/posts/", theme->html_base_dir.str)) {
		fprintf(stderr, "Error removing nonexistent posts for %s theme, dstring append error\n", theme->name.str);
		dstring_free(&base_dir);
		return 0;
	}

	int res = apply_function_to_directory_entries(&base_dir, 0, DT_REG, remove_nonexistent_post_single, site_content);
	dstring_free(&base_dir);

	if(!res) {
		fprintf(stderr, "Error removing nonexistent posts for %s theme\n", theme->name.str);
	}
	return res;
}
int remove_old_tag_files(site_content_struct* site_content, theme_struct* theme) {
	dstring_struct tag_dir;

	dstring_lazy_init(&tag_dir);

	if(!dstring_append_printf(&tag_dir, "%s/tags/", theme->html_base_dir.str)) {
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
		misc_page_init(&tag_page);
		// Just a small note: The description is copied from the title.
		// I'm calling this out because I had tried moving the description append
		// to the top, to make things line up better, and was initially confused why
		// the description was blank.
                if(!dstring_append_printf(&tag_page.filename, "tags/%s.html", tag_posts->tag.str)
			|| !dstring_append_printf(&tag_page.title, "%s tag listing", tag_posts->tag.str)
			|| !dstring_append(&tag_page.description, tag_page.title.str)
			|| !dstring_append_printf(&tag_page.content, "<header><h1>Tag: %s</h1></header>\n<section>\n", tag_posts->tag.str)) {
			fprintf(stderr, "Error generating tags, dstring append error\n");
			misc_page_free(&tag_page);
			return 0;
		}
		for(size_t j = 0; j < tag_posts->posts.length; j++) {
			post_struct* post = post_get_from_darray_of_post_pointers(&tag_posts->posts, j);
			// Split this one up onto multiple lines to make it more readable because
			// it is so long.
			if(!dstring_append_printf(&tag_page.content,
						"<div><h3><a href='/posts/%s'>%s</a></h3>\n<p>%s</p>\n</div>\n",
						post->folder_name.str,
						post->title.str,
						post->long_description.str)) {
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
	misc_page_init(&tags_page);

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
		if(!dstring_append_printf(&tags_page.content,
					"<div><a href='/tags/%s'>%s</a> (%d)</div>\n",
					tag_posts->tag.str,
					tag_posts->tag.str,
					tag_posts->posts.length)) {
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
// This is the sort function for the 'new and updated posts' list.
int updated_post_sort_compare(const void* post_a, const void* post_b) {
	time_t post_a_time = ((post_struct*)post_a)->written_date_time > ((post_struct*)post_a)->updated_at_time ? ((post_struct*)post_a)->written_date_time : ((post_struct*)post_a)->updated_at_time;
	time_t post_b_time = ((post_struct*)post_b)->written_date_time > ((post_struct*)post_b)->updated_at_time ? ((post_struct*)post_b)->written_date_time : ((post_struct*)post_b)->updated_at_time;
	return post_b_time - post_a_time;
}
int generate_index_page(site_content_struct* site_content, misc_page_struct* index_page_original) {
	misc_page_struct index_page;
	misc_page_init(&index_page);

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
		|| !dstring_append_printf(&index_page.content, "<article>\n%s<section>\n<h2>New and updated posts</h2>\n", index_page_original->content.str)) {
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
		post_struct* post = post_get_from_darray(new_posts, i);
		if(!post->can_publish) continue;
		if(num_shown >= SHOW_X_NEW_POSTS) break;
		if(!dstring_append_printf(&index_page.content,
					"<div><h3><a href=\"/posts/%s\">%s</a></h3>\n<p>%s</p>\n<ul>\n<li>Written: %s</li>\n",
					post->folder_name.str,
					post->title.str,
					post->long_description.str,
					post->written_date.str)) {
			fprintf(stderr, "Error generating index page, dstring append error\n");
			misc_page_free(&index_page);
			darray_free(new_posts);	
			free(new_posts);
			return 0;
		}
		if(post->updated_at.length > 0) {
			if(!dstring_append_printf(&index_page.content,
						"<li>Updated at: %s</li>\n",
						post->updated_at.str)) {
				fprintf(stderr, "Error generating index page, dstring append error\n");
				misc_page_free(&index_page);
				darray_free(new_posts);
				free(new_posts);
				return 0;
			}
		}
		if(!dstring_append_printf(&index_page.content,
					"<li>Series: <a href='/series/%s'>%s</a></li>\n</ul>\n</div>\n",
					post->series_name.str,
					post->series->title.str)) {
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
	// Remove nonexistent posts
	if(!remove_nonexistent_posts_for_theme(site_content, &site_content->dark_theme)
		|| !remove_nonexistent_posts_for_theme(site_content, &site_content->bright_theme)) {
		fprintf(stderr, "Error generating posts, couldn't remove nonexistent posts\n");
		return 0;
	}

	for(size_t i = 0; i < site_content->posts.length; i++) {
		post_struct* post = post_get_from_darray(&site_content->posts, i);
		if(!create_post_page(site_content, post)) {
			fprintf(stderr, "Error generating post %s\n", post->title.str);
			return 0;
		}
	}

	// TODO: Generate a page /posts/index.html
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
	misc_page_init(&series_listing_page);

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
		misc_page_init(&series_page);

		if(!dstring_append_printf(&series_listing_page.content,
					"<section>\n<h3><a href=\"/series/%s\">%s</a></h3>\n<p>%s</p>\n</section>\n",
					series->folder_name.str,
					series->title.str,
					series->short_description.str)) {
			fprintf(stderr, "Error generating series, error appending to series listing page\n");
			misc_page_free(&series_listing_page);
			misc_page_free(&series_page);
			return 0;
		}
		if(!dstring_append_printf(&series_page.filename, "series/%s/index.html", series->folder_name.str)
			|| !dstring_append_printf(&series_page.description, "Landing page for %s", series->title.str)
			|| !dstring_append_printf(&series_page.title, "%s listing", series->title.str)
			|| !dstring_append_printf(&series_page.content, "<header><h1>%s</h1></header>\n<p>%s</p><br />\n<section>\n", series->title.str, series->landing_desc_html.str)) {
			fprintf(stderr, "Error generating series, dstring_append error\n");
			misc_page_free(&series_page);
			misc_page_free(&series_listing_page);
			return 0;
		}
		for(size_t j = 0; j < series->posts.length; j++) {
			post_struct* post = post_get_from_darray_of_post_pointers(&series->posts, j);
			if(!dstring_append_printf(&series_page.content,
						"<div><h3><a href=\"/posts/%s\">%s</a></h3>\n<p>\n%s</p></div>\n",
						post->folder_name.str,
						post->title.str,
						post->long_description.str)) {
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
	misc_page_init(&sitemap);

	if(!dstring_append(&sitemap.title, "Sitemap")
		|| !dstring_append(&sitemap.description, "Sitemap")
		|| !dstring_append(&sitemap.filename, "sitemap.html")
		|| !dstring_append(&sitemap.content, "<header><h1>All posts</h1></header>\n")) {
		fprintf(stderr, "Error generating sitemap, dstring append error\n");
		misc_page_free(&sitemap);
		return 0;
	}
	// TODO: Practically identical to generate_series
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
			post_struct* post = post_get_from_darray_of_post_pointers(&series->posts, j);
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
	dstring_struct file_contents;

	dstring_lazy_init(&file_contents);

	char* lastBuildDate_start = strstr(rss_dstring->str, "lastBuildDate");
	if(lastBuildDate_start == NULL) {
		fprintf(stderr, "Error with RSS file, couldn't find lastBuildDate\n");
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
int append_rss_time(dstring_struct* rss, time_t time) {
	struct tm* time_struct = gmtime(&time);
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
	if(!dstring_append(rss, buff)) {
		fprintf(stderr, "Error generating RSS, couldn't append time\n");
		return 0;
	}
	return 1;
}
int generate_main_rss(configuration_struct* configuration, site_content_struct* site_content) {
	dstring_struct rss_feed;
	dstring_struct rss_filename;

	dstring_lazy_init(&rss_feed);
	dstring_lazy_init(&rss_filename);

	if(!dstring_append(&rss_feed, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<rss version=\"2.0\">\n<channel>\n<lastBuildDate>")) {
		fprintf(stderr, "Error generating RSS, dstring append error\n");
		dstring_free(&rss_feed);
		return 0;
	}
	if(!append_rss_time(&rss_feed, site_content->current_time)) {
		fprintf(stderr, "Error generating RSS, error appending time\n");
		dstring_free(&rss_feed);
		return 0;
	}
	if(!dstring_append_printf(&rss_feed,
				"</lastBuildDate>\n<title>%s posts</title>\n<link>https://%s/</link>\n<description>%s</description>\n",
				configuration->bright_host,
				configuration->bright_host,
				configuration->rss_description)) {
		fprintf(stderr, "Error generating RSS, dstring append error\n");
		dstring_free(&rss_feed);
		return 0;
	}
	for(size_t i = 0; i < site_content->posts.length; i++) {
		post_struct* post = post_get_from_darray(&site_content->posts, i);
		if(!post->can_publish) continue;

		if(!dstring_append_printf(&rss_feed,
					"<item>\n<title>%s</title>\n<category>%s</category>\n<pubDate>",
					post->title.str,
					post->series->title.str)
			|| !append_rss_time(&rss_feed, post->written_date_time)
			|| !dstring_append_printf(&rss_feed,
					"</pubDate>\n<link>https://%s/posts/%s</link>\n<description>%s</description>\n</item>\n",
					configuration->bright_host,
					post->folder_name.str,
					post->long_description.str)) {
			fprintf(stderr, "Error generating RSS, dstring append error\n");
			dstring_free(&rss_feed);
			return 0;
		}
	}
	if(!dstring_append(&rss_feed, "</channel>\n</rss>\n")) {
		fprintf(stderr, "Error generating RSS, dstring append error\n");
		dstring_free(&rss_feed);
		return 0;
	}
	if(!dstring_append_printf(&rss_filename, "%s/feed.rss", site_content->bright_theme.html_base_dir.str)) {
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
	if(did_write) {
		printf("Updated RSS feed\n");
	}
	dstring_free(&rss_feed);
	dstring_free(&rss_filename);
	return 1;
		
}

int generate_site_internal(configuration_struct* configuration) {
	site_content_struct site_content;
	site_content_init(&site_content);
	if(!load_site_content(configuration, &site_content)) {
		fprintf(stderr, "Error loading site content\n");
		site_content_free(&site_content);
		return 0;
	}
	if(!generate_tags(&site_content)) {
		fprintf(stderr, "Error generating tags\n");
		site_content_free(&site_content);
		return 0;
	}
	if(!generate_misc_pages(&site_content)) {
		fprintf(stderr, "Error generating misc_pages\n");
		site_content_free(&site_content);
		return 0;
	}
	if(!generate_posts(&site_content)) {
		fprintf(stderr, "Error generating posts\n");
		site_content_free(&site_content);
		return 0;
	}
	if(!generate_series(&site_content)) {
		fprintf(stderr, "Error generating series\n");
		site_content_free(&site_content);
		return 0;
	}
	if(!generate_sitemap(&site_content)) {
		fprintf(stderr, "Error generating sitemap\n");
		site_content_free(&site_content);
		return 0;
	}
	if(!generate_main_rss(configuration, &site_content)) {
		fprintf(stderr, "Error generating RSS\n");
		site_content_free(&site_content);
		return 0;
	}
	site_content_free(&site_content);
	return 1;
}
// TODO: I don't like how the site generator is also responsible for
// loading in the site. Ideally, I'd have two public functions for
// generating a site: one for where all the files are on disk,
// and you present it a configuration and it loads everything in,
// and the other where you present it a configuration and a
// site_content_struct that's already got data loaded in, such as
// if it were to be generated completely programmatically.
// Now, in the latter case, you still need a place on disk for
// the /generating/ folder, so that the lock file and the post dates
// file can go in there; however, I may be able to get around that
// requirement by piping the dates to `date`, if possible,
// and by just not having a lock. Perhaps I'll wait to allow
// the latter case until I figure that part out.
int generate_site(configuration_struct* configuration) {
	dstring_struct cbase_dir;
	dstring_lazy_init(&cbase_dir);
	if(!dstring_append(&cbase_dir, configuration->content_base_dir)) {
		fprintf(stderr, "Error appending content base dir\n");
		return 0;
	}
	if(!make_directory(&cbase_dir, "/generating")) {
		fprintf(stderr, "Error making /generating directory\n");
		return 0;
	}

	if(!dstring_append(&cbase_dir, "/generating/gen.lock")) {
		fprintf(stderr, "Error with lock directory\n");
		return 0;
	}
	int fd = open(cbase_dir.str, O_CREAT | O_EXCL);
	if(fd == -1) {
		fprintf(stderr, "Error getting lock!\n");
		dstring_free(&cbase_dir);
		return 0;
	} else {
		int res = generate_site_internal(configuration);
		unlink(cbase_dir.str);
		if(!res) {
			fprintf(stderr, "Error generating site\n");
			dstring_free(&cbase_dir);
			return 0;
		}
	}
	dstring_free(&cbase_dir);
	return 1;
}
