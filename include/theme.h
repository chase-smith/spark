#ifndef THEME_STRUCT_INCLUDE
#define THEME_STRUCT_INCLUDE
#include "dobjects.h"


// theme_struct stores the CSS files for the theme, as well as configuration
// settings that are needed when generating pages with this theme.
typedef struct theme_struct theme_struct;
typedef struct theme_struct {
	// The main CSS, included on every page.
	dstring_struct main_css;

	// The syntax highlighting CSS, included on every post that is
	// marked as having code.
	dstring_struct syntax_highlighting_css;

	// The hostname to use for all pages generated for this theme.
	dstring_struct host;

	// Likely going to be either "Bright" or "Dark". This is what shows
	// as the text on the alt-themed page, for the link going to this-themed
	// page; on a dark page, the link to the bright-themed version of the page
	// will say "Bright", and vice versa.
	dstring_struct name;

	// The HTML output directory for all of the pages for this theme.
	dstring_struct html_base_dir;

	// A pointer to the alternate theme, so that links can be generated to
	// the alternate-theme pages.
	theme_struct* alt_theme;
} theme_struct;

// ========================
// = theme_struct functions
// ========================

// Cleans up resources used for the theme; does not free the pointer itself.
void theme_free(theme_struct* theme);

// Initializes the comopnents of the theme.
void theme_init(theme_struct* theme);

// Loads in the CSS files from theme_dir.
// Only loads the CSS files, the other properties are set elsewhere (site_loader).
// Returns NULL on error.
theme_struct* theme_load(theme_struct* theme, dstring_struct* theme_dir);

#endif
