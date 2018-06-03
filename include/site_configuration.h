#ifndef SITE_CONFIGURATION_INCLUDE
#define SITE_CONFIGURATION_INCLUDE
#include "dobjects.h"
#include "param_parser.h"
// Configs from SITEGENVARS.conf
typedef struct configuration_struct {
	char* bright_host;
	char* dark_host;
	char* bright_name;
	char* dark_name;
	char* html_base_dir;
	char* code_base_dir;
	char* site_group;
	dstring_struct raw_config_file;
} configuration_struct;
int try_get_config_value(int, char**, const char*, void*);
int load_configuration(configuration_struct*, const char*);
int do_pre_validations(configuration_struct*);

#endif
