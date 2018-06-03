#ifndef THEME_STRUCT_INCLUDE
#define THEME_STRUCT_INCLUDE
#include "dobjects.h"
typedef struct theme_struct theme_struct;
typedef struct theme_struct {
	dstring_struct main_css;
	dstring_struct syntax_highlighting_css;
	dstring_struct host;
	dstring_struct name;
	dstring_struct html_base_dir;
	theme_struct* alt_theme;
} theme_struct;
void theme_free(theme_struct*);
theme_struct* theme_init(theme_struct*);
theme_struct* theme_load(theme_struct*, dstring_struct*);
#endif
