#include "dobjects.h"
#include "html_page_creators.h"
int create_page(site_content_struct* site_content, dstringbuilder_struct* page_content, theme_struct* theme, page_generation_settings_struct* page_generation_settings) {
	dstring_struct dest_filename;
	dstringbuilder_struct page_builder;

	dstring_lazy_init(&dest_filename);
	dstringbuilder_init(&page_builder);

#define CREATE_PAGE_APPEND(appending, err_message) if(!dstringbuilder_append(&page_builder, appending)) { fprintf(stderr, "Error creating page, couldn't append %s\n", err_message); dstringbuilder_free(&page_builder); return PAGE_GENERATION_FAILURE; }
#define CREATE_PAGE_APPEND_DSTRING(appending, err_message) if(!dstringbuilder_append_dstring(&page_builder, appending)) { fprintf(stderr, "Error creating page, couldn't append %s\n", err_message); dstringbuilder_free(&page_builder); return PAGE_GENERATION_FAILURE; }
#define CREATE_PAGE_APPEND_DSTRINGBUILDER(appending, err_message) if(!dstringbuilder_append_dstringbuilder(&page_builder, appending)) { fprintf(stderr, "Error creating page, couldn't append %s\n", err_message); dstringbuilder_free(&page_builder); return PAGE_GENERATION_FAILURE; }
#define CREATE_PAGE_PRINTF_APPEND(err_message, format, args...) if(!dstringbuilder_append_printf(&page_builder, format, args)) { fprintf(stderr, "Error creating page, couldn't append %s\n", err_message); dstringbuilder_free(&page_builder); return PAGE_GENERATION_FAILURE; }
	CREATE_PAGE_APPEND(site_content->html_components.header.str, "header")
	if(page_generation_settings->author != NULL) {
		CREATE_PAGE_PRINTF_APPEND("author header", "<meta name=\"author\" content=\"%s\">\n", page_generation_settings->author)
	}
	if(page_generation_settings->keywords != NULL) {
		CREATE_PAGE_PRINTF_APPEND("keywords header", "<meta name=\"keywords\" content=\"%s\">\n", page_generation_settings->keywords)
	}
	if(page_generation_settings->description != NULL) {
		CREATE_PAGE_PRINTF_APPEND("description header", "<meta name=\"description\" content=\"%s\">\n", page_generation_settings->description)
	}
	CREATE_PAGE_PRINTF_APPEND("title header", "<title>%s</title>\n", page_generation_settings->title)
	
	CREATE_PAGE_PRINTF_APPEND("canonical header", "<link rel='canonical' href='%s'>\n", page_generation_settings->canonical_url)

	CREATE_PAGE_APPEND("</head>", "end head")

	CREATE_PAGE_PRINTF_APPEND("style section", "<style>%s%s</style>",
				theme->main_css.str,
				page_generation_settings->has_code ? theme->syntax_highlighting_css.str : "")
	
	CREATE_PAGE_APPEND("<body>", "body begin")

	// Page header
	// TODO: I should probably have a setting that controls this, instead of just using the hostname.
	// I only have it using the hostname so that it's no longer hard-coded to my site name.
	// This will require passing in the configuration as parameter
	// to every creation function, something that I removed previously... Oh well, it needs to be done.
	CREATE_PAGE_PRINTF_APPEND("page header", "<header class='nheader'>\n<a class='leftfloat' href='/'>%s</a> <a class='rightfloat' href='https://%s/%s'>[%s]</a>\n</header>\n", theme->host.str, theme->alt_theme->host.str, page_generation_settings->url_path, theme->alt_theme->name.str)

	CREATE_PAGE_APPEND_DSTRINGBUILDER(page_content, "page content")
	CREATE_PAGE_APPEND(site_content->html_components.footer.str, "page footer")
	CREATE_PAGE_APPEND("</body>", "end body")
	CREATE_PAGE_APPEND(site_content->html_components.trailer.str, "page trailer")

#undef CREATE_PAGE_APPEND
#undef CREATE_PAGE_APPEND_DSTRING
#undef CREATE_PAGE_APPEND_DSTRINGBUILDER
#undef CREATE_PAGE_PRINTF_APPEND

	if(!dstring_append_printf(&dest_filename, "%s/%s", theme->html_base_dir.str, page_generation_settings->filename)) {
		fprintf(stderr, "Error generating page, dstring append error\n");
		dstringbuilder_free(&page_builder);
		dstring_free(&dest_filename);
		return PAGE_GENERATION_FAILURE;
	}
	int did_write;
	int write_res = dstringbuilder_write_file_if_different(&page_builder, dest_filename.str, &did_write);
	if(!write_res) {
		fprintf(stderr, "Error generating page, couldn't write file %s\n", dest_filename.str);
	}
	dstringbuilder_free(&page_builder);
	dstring_free(&dest_filename);
	
	if(!write_res) {
		return PAGE_GENERATION_FAILURE;
	} else {
		if(did_write) {
			return PAGE_GENERATION_UPDATED;
		} else {
			return PAGE_GENERATION_NO_UPDATE;
		}
	}
}
int create_page_wrapper(site_content_struct* site_content, dstringbuilder_struct* page_content, page_generation_settings_struct* page_generation_settings, int is_post) {
	dstring_struct canonical_url;
	dstringbuilder_struct page_builder;

	dstring_lazy_init(&canonical_url);
	dstringbuilder_init(&page_builder);

	if(!dstringbuilder_append(&page_builder, "<main")
		|| !dstringbuilder_append(&page_builder, is_post ? " class='post'>\n" : ">\n")
		|| !dstringbuilder_append_dstringbuilder(&page_builder, page_content)
		|| !dstringbuilder_append(&page_builder, "</main>\n")) {
		fprintf(stderr, "Error creating page, dstring append error\n");
		dstringbuilder_free(&page_builder);
		dstring_free(&canonical_url);
		return PAGE_GENERATION_FAILURE;
	}
	if(!dstring_append_printf(&canonical_url, "https://%s/%s", site_content->bright_theme.host.str, page_generation_settings->url_path)) {
		fprintf(stderr, "Error creating page, canonical url dstring append error\n");
		dstringbuilder_free(&page_builder);
		dstring_free(&canonical_url);
		return PAGE_GENERATION_FAILURE;
	}
	page_generation_settings->canonical_url = canonical_url.str;
	int bright_res = create_page(site_content, &page_builder, &site_content->bright_theme, page_generation_settings);
	if(!bright_res) {
		fprintf(stderr, "Error creating bright version of page %s\n", canonical_url.str);
		dstringbuilder_free(&page_builder);
		dstring_free(&canonical_url);
		return PAGE_GENERATION_FAILURE;
	}
	if(bright_res == PAGE_GENERATION_UPDATED) {
		printf("Updated bright page %s\n", page_generation_settings->filename);
	}
	int dark_res = create_page(site_content, &page_builder, &site_content->dark_theme, page_generation_settings);
	if(!dark_res) {
		fprintf(stderr, "Error creating dark version of page %s\n", canonical_url.str);
		dstringbuilder_free(&page_builder);
		dstring_free(&canonical_url);
		return PAGE_GENERATION_FAILURE;
	}
	if(dark_res == PAGE_GENERATION_UPDATED) {
		printf("Updated dark page %s\n", page_generation_settings->filename);
	}
	page_generation_settings->canonical_url = NULL;
	dstring_free(&canonical_url);
	dstringbuilder_free(&page_builder);
	if(bright_res > dark_res) {
		return bright_res;
	} else {
		return dark_res;
	}
}
int create_misc_page(site_content_struct* site_content, misc_page_struct* misc_page) {
	// The URL path is derived from the filename by stripping out the file extension
	char* url_path = strdup(misc_page->filename.str);
	if(url_path == NULL) {
		fprintf(stderr, "Error mallocing space for url_path\n");
		return PAGE_GENERATION_FAILURE;
	}
	size_t file_extension_start = get_file_extension_start(url_path);
	url_path[file_extension_start] = '\0';
	page_generation_settings_struct page_generation_settings;
	page_generation_settings.keywords = NULL;
	page_generation_settings.author = NULL;
	page_generation_settings.title = misc_page->title.str;
	page_generation_settings.filename = misc_page->filename.str;
	page_generation_settings.url_path = url_path;
	page_generation_settings.has_code = 0;
	page_generation_settings.description = misc_page->description.str;

	dstringbuilder_struct page_builder;
	dstringbuilder_init(&page_builder);
	if(!dstringbuilder_append_dstring(&page_builder, &misc_page->content)) {
		fprintf(stderr, "Error creating page %s, dstringbuilder append dstring error\n", url_path);
		dstringbuilder_free(&page_builder);
		return PAGE_GENERATION_FAILURE;
	}
	int create_page_res = create_page_wrapper(site_content, &page_builder, &page_generation_settings, 0);
	if(!create_page_res) {
		fprintf(stderr, "Error creating page %s, page create error\n", url_path);
	}
	dstringbuilder_free(&page_builder);
	free(url_path);
	return create_page_res;
}
int create_post_page_append_recommended_readings(dstring_struct* page, darray_struct* recommendations, const char* recommendation_type) {
	int num_posts_added = 0;
	for(size_t i = 0; i < recommendations->length; i++) {
		post_struct* recommended_post = post_get_from_darray_of_post_pointers(recommendations, i);
		if(!recommended_post->can_publish) continue;
		if(num_posts_added == 0) {
			if(!dstring_append_printf(page, "<div class=\"s_p_reading\">Suggested %s reading: ", recommendation_type)) {
				fprintf(stderr, "Error appending post recommendations\n");
				return 0;
			}
		}
		if(num_posts_added > 0) {
			if(!dstring_append(page, ", ")) {
				fprintf(stderr, "Error appending post recommendations\n");
				return 0;
			}
		}
		if(!dstring_append_printf(page, "<a href=\"/posts/%s\">%s</a>",
					recommended_post->folder_name.str,
					recommended_post->title.str)) {
			fprintf(stderr, "Error appending post recommendations\n");
			return 0;
		}
		num_posts_added++;
	}
	if(num_posts_added > 0) {
		if(!dstring_append(page, "</div>")) {
			fprintf(stderr, "Error appending post recommendations\n");
			return 0;
		}
	}
	return 1;
}
int create_post_page(site_content_struct* site_content, post_struct* post) {
	dstringbuilder_struct page_builder;
	dstring_struct tags;
	dstring_struct url_path;
	dstring_struct filename;

	dstringbuilder_init(&page_builder);
	dstring_lazy_init(&tags);
	dstring_lazy_init(&url_path);
	dstring_lazy_init(&filename);

	if(!dstring_append_printf(&url_path, "posts/%s", post->folder_name.str)) {
		fprintf(stderr, "Error creating post page, dstring append error\n");
		dstringbuilder_free(&page_builder);
		dstring_free(&tags);
		dstring_free(&url_path);
		dstring_free(&filename);
		return PAGE_GENERATION_FAILURE;
	}
	if(!dstring_append_printf(&filename, "%s.html", url_path.str)) {
		fprintf(stderr, "Error creating post page, dstring append error\n");
		dstringbuilder_free(&page_builder);
		dstring_free(&tags);
		dstring_free(&url_path);
		dstring_free(&filename);
		return PAGE_GENERATION_FAILURE;
	}
#define CREATE_POST_PAGE_APPEND(appending, err_message) if(!dstringbuilder_append(&page_builder, appending)) { fprintf(stderr, "Error creating page, couldn't append %s\n", err_message); dstringbuilder_free(&page_builder); dstring_free(&tags); dstring_free(&url_path); dstring_free(&filename); return PAGE_GENERATION_FAILURE; }
#define CREATE_POST_PAGE_APPEND_DSTRING(appending, err_message) if(!dstringbuilder_append_dstring(&page_builder, appending)) { fprintf(stderr, "Error creating page, couldn't append %s\n", err_message); dstringbuilder_free(&page_builder); dstring_free(&tags); dstring_free(&url_path); dstring_free(&filename); return PAGE_GENERATION_FAILURE; }
#define CREATE_POST_PAGE_PRINTF_APPEND(err_message, format, args...) if(!dstringbuilder_append_printf(&page_builder, format, args)) { fprintf(stderr, "Error creating page, couldn't append %s\n", err_message); dstringbuilder_free(&page_builder); dstring_free(&tags); dstring_free(&url_path); dstring_free(&filename); return PAGE_GENERATION_FAILURE; }
	CREATE_POST_PAGE_PRINTF_APPEND("article begin", "<article>\n<header>\n<h1>%s</h1>\n", post->title.str)
	if(!create_post_page_append_recommended_readings(page_builder.current_dstring, &post->suggested_prev_reading, "previous")) {
		fprintf(stderr, "Error creating post page, couldn't append suggested previous readings\n");
		dstringbuilder_free(&page_builder);
		dstring_free(&tags);
		dstring_free(&url_path);
		dstring_free(&filename);
		return PAGE_GENERATION_FAILURE;
	}
	CREATE_POST_PAGE_APPEND("</header>\n", "post header")
	CREATE_POST_PAGE_APPEND_DSTRING(&post->content, "post content")
	if(!dstringbuilder_new_dstring(&page_builder)) {
		fprintf(stderr, "Error creating post page, couldn't make a new dstring\n");
		dstringbuilder_free(&page_builder);
		dstring_free(&tags);
		dstring_free(&url_path);
		dstring_free(&filename);
		return PAGE_GENERATION_FAILURE;
	}
	if(!create_post_page_append_recommended_readings(page_builder.current_dstring, &post->suggested_next_reading, "next")) {
		fprintf(stderr, "Error creating post page, couldn't append suggested next readings\n");
		dstringbuilder_free(&page_builder);
		dstring_free(&tags);
		dstring_free(&url_path);
		dstring_free(&filename);
		return PAGE_GENERATION_FAILURE;
	}
	CREATE_POST_PAGE_APPEND("<footer class=\"flexcontainer postfooter\">\n", "post footer")
	CREATE_POST_PAGE_APPEND("<div>\n", "post footer")
	CREATE_POST_PAGE_PRINTF_APPEND("post footer series title", "<div>Series: <a href=\"/series/%s\">%s</a></div>\n", post->series_name.str, post->series->title.str)

	CREATE_POST_PAGE_PRINTF_APPEND("post footer author", "<div>Author: @%s</div>\n", post->author.str)
	
	CREATE_POST_PAGE_APPEND("<div>Tags: ", "post footer tags")
	for(size_t i = 0; i < post->tags.length; i++) {
		const char* tag = *((const char**) darray_get_elem(&post->tags, i));
		if(i > 0) {
			CREATE_POST_PAGE_APPEND(", ", "post footer tags")
			if(!dstring_append(&tags, ",")) {
				fprintf(stderr, "Error generating post, dstring append error\n");
				dstring_free(&tags);
				dstringbuilder_free(&page_builder);
				dstring_free(&url_path);
				dstring_free(&filename);
				return PAGE_GENERATION_FAILURE;
			}
		}
		if(!dstring_append(&tags, tag)) {
			fprintf(stderr, "Error generating post, dstring append error\n");
			dstring_free(&tags);
			dstringbuilder_free(&page_builder);
			dstring_free(&url_path);
			dstring_free(&filename);
			return PAGE_GENERATION_FAILURE;
		}
		CREATE_POST_PAGE_PRINTF_APPEND("post footer tags", "<a href=\"/tags/%s\">%s</a>", tag, tag)
	}
	CREATE_POST_PAGE_APPEND("</div>\n</div>\n", "close post footer")

	CREATE_POST_PAGE_PRINTF_APPEND("Post right footer", "<div>\n<div>Written: %s</div>\n", post->written_date.str)
	
	if(post->updated_at.length >0) {
		CREATE_POST_PAGE_PRINTF_APPEND("post updated at", "<div>Updated: %s</div>\n", post->updated_at.str)
	}
	CREATE_POST_PAGE_APPEND("</div>\n</footer>\n</article>\n</main>\n", "post close")
	

#undef CREATE_POST_PAGE_APPEND
#undef CREATE_POST_PAGE_PRINTF_APPEND
	page_generation_settings_struct page_generation_settings;
	page_generation_settings.filename = filename.str;
	page_generation_settings.keywords = tags.str;
	page_generation_settings.description = post->short_description.str;
	page_generation_settings.title = post->title.str;
	page_generation_settings.author = post->author.str;
	page_generation_settings.url_path = url_path.str;
	page_generation_settings.has_code = post->has_code;
	int create_page_res = create_page_wrapper(site_content, &page_builder, &page_generation_settings, 1);
	dstring_free(&url_path);
	dstring_free(&filename);
	dstringbuilder_free(&page_builder);
	dstring_free(&tags);
	if(!create_page_res) {
		fprintf(stderr, "Error generating post %s, creation error\n", post->title.str);
	}
	return create_page_res;
}
