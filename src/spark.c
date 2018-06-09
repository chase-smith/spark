// TODO: Stop using base_dir where it's not a base directory but instead a filename.

#define _BSD_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <stdarg.h>
#include "dobjects.h"
#include "core_objects.h"
#include "post.h"
#include "param_parser.h"
#include "file_helpers.h"
#include "html_page_creators.h"
#include "site_configuration.h"
#include "site_generator.h"
#include "site_loader.h"

#define ERROR_BAD_PARAMETERS 1
#define ERROR_BAD_CONFIGURATION 2
#define ERROR_GENERATING_SITE 3
#define ERROR_OTHER 4


// GENERAL TODO: Fix includes across all files, some files include
// things they don't need.



typedef struct settings_struct {
	char* config_file;
	configuration_struct configuration;
	int show_help;
} settings_struct;





void show_help() {
	printf("--config <config file> [--only-generate-if-publishable-posts-changes]\n");
}

int get_parameters(settings_struct* settings, int argc, char* argv[]) {
	if(paramparser_get_flag(argc, argv, "--help", &settings->show_help)) {
		show_help();
		return 1;
	}
	
	if(!paramparser_get_string(argc, argv, "--config", &settings->config_file, PARAMPARSER_REQUIRED)) {
		fprintf(stderr, "Missing required parameter --config\n");
		return 0;
	}
	return 1;
}





int main(int argc, char* argv[]) {
	// TODO: Set proper permissions on all created directories and files.
	settings_struct settings;
	if(!get_parameters(&settings, argc-1, &argv[1])) {
		fprintf(stderr, "Error, bad parameters\n");
		return ERROR_BAD_PARAMETERS;
	}
	if(settings.show_help) {
		return 0;
	}
	if(!load_configuration(&settings.configuration, settings.config_file)) {
		fprintf(stderr, "Error, bad configuration\n");
		return ERROR_BAD_CONFIGURATION;
	}
	dstring_struct cbase_dir;
	dstring_lazy_init(&cbase_dir);
	if(!dstring_append(&cbase_dir, settings.configuration.code_base_dir)) {
		fprintf(stderr, "Error appending code base dir\n");
		return ERROR_OTHER;
	}
	if(!make_directory(&cbase_dir, "/generating")) {
		fprintf(stderr, "Error making /generating directory\n");
		return ERROR_OTHER;
	}

	if(!dstring_append(&cbase_dir, "/generating/gen.lock")) {
		fprintf(stderr, "Error with lock directory\n");
		return ERROR_OTHER;
	}
	int fd = open(cbase_dir.str, O_CREAT | O_EXCL);
	if(fd == -1) {
		fprintf(stderr, "Error getting lock!\n");
		return 1;
	} else {
		int res = generate_site(&settings.configuration);
		unlink(cbase_dir.str);
		if(!res) {
			fprintf(stderr, "Error generating site\n");
			return ERROR_GENERATING_SITE;
		}
	}
	dstring_free(&cbase_dir);
	dstring_free(&settings.configuration.raw_config_file);
	return 0;
}
