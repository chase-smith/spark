#include "dobjects.h"
#include "misc_page.h"

void misc_page_free(misc_page_struct* misc_page) {
	dstring_free(&misc_page->content);
	dstring_free(&misc_page->title);
	dstring_free(&misc_page->filename);
	dstring_free(&misc_page->description);
}
misc_page_struct* misc_page_load(misc_page_struct* misc_page, dstring_struct* base_dir) {
	// TODO: Check for existence of generate flag file
	if(!dstring_try_load_file(&misc_page->content, base_dir, "/content.html", "misc_page")
		|| !dstring_try_load_file(&misc_page->title, base_dir, "/title", "misc_page")
		|| !dstring_try_load_file(&misc_page->filename, base_dir, "/filename", "misc_page")
		|| !dstring_try_load_file(&misc_page->description, base_dir, "/description", "misc_page")) {
		return NULL;
	}
	dstring_remove_trailing_newlines(&misc_page->title);
	dstring_remove_trailing_newlines(&misc_page->filename);
	dstring_remove_trailing_newlines(&misc_page->description);
	return misc_page;
}
misc_page_struct* misc_page_init(misc_page_struct* misc_page) {
	misc_page->generate = 0;
	int success = dstring_init(&misc_page->content)
		&& dstring_init_with_size(&misc_page->title, 100)
		&& dstring_init_with_size(&misc_page->filename, 100)
		&& dstring_init_with_size(&misc_page->description, 200);
	if(!success) {
		misc_page_free(misc_page);
		return NULL;
	}
	return misc_page;
}

