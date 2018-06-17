#ifndef SITE_CONFIGURATION_INCLUDE
#define SITE_CONFIGURATION_INCLUDE
#include "dobjects.h"
#include "param_parser.h"

// configuration_struct houses the site configuration variables that are
// defined in the site config file. If loaded via load_configuration(),
// then all strings are pointing to strings within
// configuration->raw_config_file.
typedef struct configuration_struct {
	// The hostname for the bright-themed site.
	char* bright_host;

	// The hostname for the dark-themed site.
	char* dark_host;

	// The text that will appear when you're on a dark page and looking
	// at the link that will take you to the bright-themed version of
	// the same page. Likely going to be "Bright".
	char* bright_name;

	// The text that will appear when you're on a bright page and looking
	// at the link that will take you to the dark-themed version of
	// the same page. Likely going to be "Dark".
	char* dark_name;

	// The root HTML output directory (no trailing slash).
	char* html_base_dir;

	// The base content directory with all of the site files.
	char* content_base_dir;

	// The group that all created files will be owned by. Not used yet.
	char* site_group;

	// The description to put in the RSS file.
	char* rss_description;

	// The loaded configuration file; by default, all configuration strings
	// will point to strings in this dstring (the dstring itself will
	// be modified, and shouldn't be used directly).
	dstring_struct raw_config_file;
} configuration_struct;

// ================================
// = configuration_struct functions
// ================================

// Loads configuration with the config defined in config_file.
// Returns 0 on error.
int load_configuration(configuration_struct* configuration, const char* config_file);

// Does some validations with the content directories and verifies that the
// HTML directories exist.
// Returns 0 on error.
int do_pre_validations(configuration_struct*);

#endif
