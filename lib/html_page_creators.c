#include "dobjects.h"
#include "html_page_creators.h"
int create_page(site_content_struct* site_content, dstring_struct* page_content, theme_struct* theme, page_generation_settings_struct* page_generation_settings) {
	dstring_struct page;
	if(!dstring_init_with_size(&page, 10000)) {
		fprintf(stderr, "Error creating page, dstring init error\n");
		return PAGE_GENERATION_FAILURE;
	}
#define CREATE_PAGE_APPEND(appending, err_message) if(!dstring_append(&page, appending)) { fprintf(stderr, "Error creating page, couldn't append %s\n", err_message); dstring_free(&page); return PAGE_GENERATION_FAILURE; }
	CREATE_PAGE_APPEND(site_content->html_components.header.str, "header")
	if(page_generation_settings->author != NULL) {
		CREATE_PAGE_APPEND("<meta name=\"author\" content=\"", "author header")
		CREATE_PAGE_APPEND(page_generation_settings->author, "author header")
		CREATE_PAGE_APPEND("\">\n", "author header")
	}
	if(page_generation_settings->keywords != NULL) {
		CREATE_PAGE_APPEND("<meta name=\"keywords\" content=\"", "keywords header")
		CREATE_PAGE_APPEND(page_generation_settings->keywords, "keywords header")
		CREATE_PAGE_APPEND("\">\n", "keywords header")
	}
	if(page_generation_settings->description != NULL) {
		CREATE_PAGE_APPEND("<meta name=\"description\" content=\"", "description header")
		CREATE_PAGE_APPEND(page_generation_settings->description, "description header")
		CREATE_PAGE_APPEND("\">\n", "description header")
	}
	CREATE_PAGE_APPEND("<title>", "title header")
	CREATE_PAGE_APPEND(page_generation_settings->title, "title header")
	CREATE_PAGE_APPEND("</title>\n", "title header")
	
	CREATE_PAGE_APPEND("<link rel='canonical' href='", "canonical header")
	CREATE_PAGE_APPEND(page_generation_settings->canonical_url, "canonical header")
	CREATE_PAGE_APPEND("'>\n", "canonical header")

	CREATE_PAGE_APPEND("</head>", "end head")

	CREATE_PAGE_APPEND("<style>", "style section")
	CREATE_PAGE_APPEND(theme->main_css.str, "style section")
	if(page_generation_settings->has_code) {
		CREATE_PAGE_APPEND(theme->syntax_highlighting_css.str, "style section")
	}
	CREATE_PAGE_APPEND("</style>", "style section")
	
	CREATE_PAGE_APPEND("<body>", "body begin")

	// Page header
	CREATE_PAGE_APPEND("<header class='nheader'>\n", "page header")
	// TODO: This should be passed in as a site setting or something...
	CREATE_PAGE_APPEND("<a class='leftfloat' href='/'>thinkingincode.ninja</a> <a class='rightfloat' href='https://", "page header")
	CREATE_PAGE_APPEND(theme->alt_theme->host.str, "page header")
	CREATE_PAGE_APPEND("/", "page header")
	CREATE_PAGE_APPEND(page_generation_settings->url_path, "page header")
	CREATE_PAGE_APPEND("'>[", "page header")
	CREATE_PAGE_APPEND(theme->alt_theme->name.str, "page header")
	CREATE_PAGE_APPEND("]</a>\n", "page header")
	CREATE_PAGE_APPEND("</header>\n", "page header")

	CREATE_PAGE_APPEND(page_content->str, "page content")
	CREATE_PAGE_APPEND(site_content->html_components.footer.str, "page footer")
	CREATE_PAGE_APPEND("</body>", "end body")
	CREATE_PAGE_APPEND(site_content->html_components.trailer.str, "page trailer")

#undef CREATE_PAGE_APPEND
	dstring_struct dest_filename;
	if(!dstring_init(&dest_filename)) {
		fprintf(stderr, "Error generating page, dstring init error\n");
		dstring_free(&page);
		return PAGE_GENERATION_FAILURE;
	}
	if(!dstring_append(&dest_filename, theme->html_base_dir.str)
		|| !dstring_append(&dest_filename, "/")
		|| !dstring_append(&dest_filename, page_generation_settings->filename)) {
		fprintf(stderr, "Error generating page, dstring append error\n");
		dstring_free(&page);
		dstring_free(&dest_filename);
		return PAGE_GENERATION_FAILURE;
	}
	int did_write;
	int write_res = dstring_write_file_if_different(&page, dest_filename.str, &did_write);
	dstring_free(&page);
	if(!write_res) {
		fprintf(stderr, "Error generating page, couldn't write file %s\n", dest_filename.str);
	}
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
int create_page_wrapper(site_content_struct* site_content, dstring_struct* page_content, page_generation_settings_struct* page_generation_settings, int is_post) {
	dstring_struct wrapped_page;
	if(!dstring_init_with_size(&wrapped_page, page_content->length + 50)) {
		fprintf(stderr, "Error creating page, dstring init error\n");
		return PAGE_GENERATION_FAILURE;
	}
	if(!dstring_append(&wrapped_page, "<main")
		|| !dstring_append(&wrapped_page, is_post ? " class='post'>\n" : ">\n")
		|| !dstring_append(&wrapped_page, page_content->str)
		|| !dstring_append(&wrapped_page, "</main>\n")) {
		fprintf(stderr, "Error creating page, dstring append error\n");
		dstring_free(&wrapped_page);
		return PAGE_GENERATION_FAILURE;
	}
	dstring_struct canonical_url;
	if(!dstring_init(&canonical_url)) {
		fprintf(stderr, "Error creating page, canonical url dstring init error\n");
		dstring_free(&wrapped_page);
		return PAGE_GENERATION_FAILURE;
	}
	if(!dstring_append(&canonical_url, "https://")
		|| !dstring_append(&canonical_url, site_content->bright_theme.host.str)
		|| !dstring_append(&canonical_url, "/")
		|| !dstring_append(&canonical_url, page_generation_settings->url_path)) {
		fprintf(stderr, "Error creating page, canonical url dstring append error\n");
		dstring_free(&wrapped_page);
		dstring_free(&canonical_url);
		return PAGE_GENERATION_FAILURE;
	}
	page_generation_settings->canonical_url = canonical_url.str;
	int bright_res = create_page(site_content, &wrapped_page, &site_content->bright_theme, page_generation_settings);
	if(!bright_res) {
		fprintf(stderr, "Error creating bright version of page %s\n", canonical_url.str);
		dstring_free(&wrapped_page);
		dstring_free(&canonical_url);
		return PAGE_GENERATION_FAILURE;
	}
	if(bright_res == PAGE_GENERATION_UPDATED) {
		printf("Updated bright page %s\n", page_generation_settings->filename);
	}
	int dark_res = create_page(site_content, &wrapped_page, &site_content->dark_theme, page_generation_settings);
	if(!dark_res) {
		fprintf(stderr, "Error creating dark version of page %s\n", canonical_url.str);
		dstring_free(&wrapped_page);
		dstring_free(&canonical_url);
		return PAGE_GENERATION_FAILURE;
	}
	if(dark_res == PAGE_GENERATION_UPDATED) {
		printf("Updated dark page %s\n", page_generation_settings->filename);
	}
	page_generation_settings->canonical_url = NULL;
	dstring_free(&wrapped_page);
	dstring_free(&canonical_url);
	if(bright_res > dark_res) {
		return bright_res;
	} else {
		return dark_res;
	}
}
int create_misc_page(site_content_struct* site_content, misc_page_struct* misc_page) {
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

	int create_page_res = create_page_wrapper(site_content,  &misc_page->content, &page_generation_settings, 0);
	if(!create_page_res) {
		fprintf(stderr, "Error creating page %s, page create error\n", url_path);
	}
	free(url_path);
	return create_page_res;
}
int create_post_page_append_recommended_readings(dstring_struct* page, darray_struct* recommendations, const char* recommendation_type) {
	int num_posts_added = 0;
	for(size_t i = 0; i < recommendations->length; i++) {
		post_struct* recommended_post = *((post_struct**) darray_get_elem(recommendations, i));
		if(!recommended_post->can_publish) continue;
		if(num_posts_added == 0) {
			if(!dstring_append(page, "<div class=\"s_p_reading\">Suggested ")
				|| !dstring_append(page, recommendation_type)
				|| !dstring_append(page, " reading: ")) {
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
		if(!dstring_append(page, "<a href=\"/posts/")
			|| !dstring_append(page, recommended_post->folder_name.str)
			|| !dstring_append(page, "\">")
			|| !dstring_append(page, recommended_post->title.str)
			|| !dstring_append(page, "</a>")) {
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
	dstring_struct page;
	dstring_struct tags;
	dstring_struct url_path;
	dstring_struct filename;
	if(!dstring_init_with_size(&page, 10000)) {
		fprintf(stderr, "Error creating post page, dstring init error\n");
		return PAGE_GENERATION_FAILURE;
	}
	if(!dstring_init(&tags)) {
		fprintf(stderr, "Error creating post page, dstring init error\n");
		dstring_free(&page);
		return PAGE_GENERATION_FAILURE;
	}
	if(!dstring_init(&url_path)) {
		fprintf(stderr, "Error creating post page, dstring init error\n");
		dstring_free(&page);
		dstring_free(&tags);
		return PAGE_GENERATION_FAILURE;
	}
	if(!dstring_init(&filename)) {
		fprintf(stderr, "Error creating post page, dstring init error\n");
		dstring_free(&page);
		dstring_free(&tags);
		dstring_free(&url_path);
		return PAGE_GENERATION_FAILURE;
	}
	if(!dstring_append(&url_path, "posts/")
		|| !dstring_append(&url_path, post->folder_name.str)) {
		fprintf(stderr, "Error creating post page, dstring append error\n");
		dstring_free(&page);
		dstring_free(&tags);
		dstring_free(&url_path);
		dstring_free(&filename);
		return PAGE_GENERATION_FAILURE;
	}
	if(!dstring_append(&filename, url_path.str)
		|| !dstring_append(&filename, ".html")) {
		fprintf(stderr, "Error creating post page, dstring append error\n");
		dstring_free(&page);
		dstring_free(&tags);
		dstring_free(&url_path);
		dstring_free(&filename);
		return PAGE_GENERATION_FAILURE;
	}
#define CREATE_POST_PAGE_APPEND(appending, err_message) if(!dstring_append(&page, appending)) { fprintf(stderr, "Error creating page, couldn't append %s\n", err_message); dstring_free(&page); dstring_free(&tags); dstring_free(&url_path); dstring_free(&filename); return PAGE_GENERATION_FAILURE; }
	CREATE_POST_PAGE_APPEND("<article>\n", "article begin")
	CREATE_POST_PAGE_APPEND("<header>\n", "header begin")
	CREATE_POST_PAGE_APPEND("<h1>", "post header")
	CREATE_POST_PAGE_APPEND(post->title.str, "post header")
	CREATE_POST_PAGE_APPEND("</h1>\n", "post header")
	if(!create_post_page_append_recommended_readings(&page, &post->suggested_prev_reading, "previous")) {
		fprintf(stderr, "Error creating post page, couldn't append suggested previous readings\n");
		dstring_free(&page);
		dstring_free(&tags);
		dstring_free(&url_path);
		dstring_free(&filename);
		return PAGE_GENERATION_FAILURE;
	}
	CREATE_POST_PAGE_APPEND("</header>\n", "post header")
	CREATE_POST_PAGE_APPEND(post->content.str, "post content")
	if(!create_post_page_append_recommended_readings(&page, &post->suggested_next_reading, "next")) {
		fprintf(stderr, "Error creating post page, couldn't append suggested next readings\n");
		dstring_free(&page);
		dstring_free(&tags);
		dstring_free(&url_path);
		dstring_free(&filename);
		return PAGE_GENERATION_FAILURE;
	}
	CREATE_POST_PAGE_APPEND("<footer class=\"flexcontainer postfooter\">\n", "post footer")
	CREATE_POST_PAGE_APPEND("<div>\n", "post footer")
	CREATE_POST_PAGE_APPEND("<div>Series: <a href=\"/series/", "post footer series title")
	CREATE_POST_PAGE_APPEND(post->series_name.str, "post footer series title")
	CREATE_POST_PAGE_APPEND("\">", "post footer series title")
	CREATE_POST_PAGE_APPEND(post->series->title.str, "post footer series title")
	CREATE_POST_PAGE_APPEND("</a></div>\n", "post footer series title")
	
	CREATE_POST_PAGE_APPEND("<div>Author: @", "post footer author")
	CREATE_POST_PAGE_APPEND(post->author.str, "post footer author")
	CREATE_POST_PAGE_APPEND("</div>\n", "post footer author")
	
	CREATE_POST_PAGE_APPEND("<div>Tags: ", "post footer tags")
	for(size_t i = 0; i < post->tags.length; i++) {
		const char* tag = *((const char**) darray_get_elem(&post->tags, i));
		if(i > 0) {
			CREATE_POST_PAGE_APPEND(", ", "post footer tags")
			if(!dstring_append(&tags, ",")) {
				fprintf(stderr, "Error generating post, dstring append error\n");
				dstring_free(&tags);
				dstring_free(&page);
				dstring_free(&url_path);
				dstring_free(&filename);
				return PAGE_GENERATION_FAILURE;
			}
		}
		if(!dstring_append(&tags, tag)) {
			fprintf(stderr, "Error generating post, dstring append error\n");
			dstring_free(&tags);
			dstring_free(&page);
			dstring_free(&url_path);
			dstring_free(&filename);
			return PAGE_GENERATION_FAILURE;
		}
		CREATE_POST_PAGE_APPEND("<a href=\"/tags/", "post footer tags")
		CREATE_POST_PAGE_APPEND(tag, "post footer tags")
		CREATE_POST_PAGE_APPEND("\">", "post footer tags")
		CREATE_POST_PAGE_APPEND(tag, "post footer tags")
		CREATE_POST_PAGE_APPEND("</a>", "post footer tags")
	}
	CREATE_POST_PAGE_APPEND("</div>\n", "post footer tags")
	CREATE_POST_PAGE_APPEND("</div>\n", "post footer left side")

	CREATE_POST_PAGE_APPEND("<div>\n", "post footer right")
	CREATE_POST_PAGE_APPEND("<div>Written: ", "post written date")
	CREATE_POST_PAGE_APPEND(post->written_date.str, "post written date")
	CREATE_POST_PAGE_APPEND("</div>\n", "post written date")
	
	if(post->updated_at.length >0) {
		CREATE_POST_PAGE_APPEND("<div>Updated: ", "post updated at")
		CREATE_POST_PAGE_APPEND(post->updated_at.str, "post updated at")
		CREATE_POST_PAGE_APPEND("</div>\n", "post updated at")
	}
	CREATE_POST_PAGE_APPEND("</div>\n", "post footer left side close")
	CREATE_POST_PAGE_APPEND("</footer>\n", "post footer close")
	CREATE_POST_PAGE_APPEND("</article>\n", "post article close")
	CREATE_POST_PAGE_APPEND("</main>\n", "post main close")
	

#undef CREATE_POST_PAGE_APPEND
	page_generation_settings_struct page_generation_settings;
	page_generation_settings.filename = filename.str;
	page_generation_settings.keywords = tags.str;
	page_generation_settings.description = post->short_description.str;
	page_generation_settings.title = post->title.str;
	page_generation_settings.author = post->author.str;
	page_generation_settings.url_path = url_path.str;
	page_generation_settings.has_code = post->has_code;
	int create_page_res = create_page_wrapper(site_content, &page, &page_generation_settings, 1);
	dstring_free(&url_path);
	dstring_free(&filename);
	dstring_free(&page);
	dstring_free(&tags);
	if(!create_page_res) {
		fprintf(stderr, "Error generating post %s, creation error\n", post->title.str);
	}
	return create_page_res;
}