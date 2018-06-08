#include "dobjects.h"
#include "html_components.h"

void html_components_free(html_components_struct* html_components) {
	dstring_free(&html_components->header);
	dstring_free(&html_components->footer);
	dstring_free(&html_components->trailer);
}
void html_components_init(html_components_struct* html_components) {
	dstring_lazy_init(&html_components->header);
	dstring_lazy_init(&html_components->footer);
	dstring_lazy_init(&html_components->trailer);
}
html_components_struct* html_components_load(html_components_struct* html_components, dstring_struct* base_dir) {
	if(!dstring_try_load_file(&html_components->header, base_dir, "/header.html", "HTML component")
		|| !dstring_try_load_file(&html_components->footer, base_dir, "/footer.html", "HTML component")
		|| !dstring_try_load_file(&html_components->trailer, base_dir, "/trailer.html", "HTML component")) {
		return NULL;
	}
	return html_components;
}
