#include "dobjects.h"
#include "theme.h"
void theme_free(theme_struct* theme) {
	dstring_free(&theme->main_css);
	dstring_free(&theme->syntax_highlighting_css);
	dstring_free(&theme->host);
	dstring_free(&theme->name);
	dstring_free(&theme->html_base_dir);
}
void theme_init(theme_struct* theme) {
	theme->alt_theme = NULL;
	dstring_lazy_init(&theme->main_css);
	dstring_lazy_init(&theme->syntax_highlighting_css);
	dstring_lazy_init(&theme->host);
	dstring_lazy_init(&theme->name);
	dstring_lazy_init(&theme->html_base_dir);
}
theme_struct* theme_load(theme_struct* theme, dstring_struct* base_dir) {
	if(!dstring_try_load_file(&theme->main_css, base_dir, "/main.css", "theme")) {
		return NULL;
	}
	if(!dstring_try_load_file(&theme->syntax_highlighting_css, base_dir, "/syntax-highlighting.css", "theme")) {
		return NULL;
	}
	return theme;
}
