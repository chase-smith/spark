#include "dobjects.h"
#include "series.h"
#include "post.h"
void series_free(series_struct* series) {
	dstring_free(&series->folder_name);
	dstring_free(&series->landing_desc_html);
	dstring_free(&series->short_description);
	dstring_free(&series->title);
	darray_free(&series->posts);
}
series_struct* series_load(series_struct* series, dstring_struct* base_dir, const char* folder_name) {
	if(!dstring_append(&series->folder_name, folder_name)) {
		fprintf(stderr, "Error loading series, folder_name dstring append error\n");
		return NULL;
	}
	dstring_struct order_string;
	if(!dstring_init(&order_string)) {
		fprintf(stderr, "Error loading series, dstring init error\n");
		return NULL;
	}
	
	if(!dstring_try_load_file(&series->landing_desc_html, base_dir, "/landing-desc.html", "series")
		|| !dstring_try_load_file(&series->short_description, base_dir, "/short-description", "series")
		|| !dstring_try_load_file(&series->title, base_dir, "/title", "series")
		|| !dstring_try_load_file(&order_string, base_dir, "/order", "series")) {
		dstring_free(&order_string);
		return NULL;
	}
	dstring_remove_trailing_newlines(&series->short_description);
	dstring_remove_trailing_newlines(&series->title);

	series->order = atoi(order_string.str);
	dstring_free(&order_string);

	return series;
}
series_struct* series_init(series_struct* series) {
	series->order = 0;
	int success = dstring_init(&series->landing_desc_html)
		&& dstring_init_with_size(&series->short_description, 200)
		&& dstring_init_with_size(&series->title, 100)
		&& dstring_init_with_size(&series->folder_name, 100)
		&& darray_init_with_size(&series->posts, sizeof(post_struct*), 20);
	if(!success) {
		series_free(series);
		return NULL;
	}
	return series;
}
