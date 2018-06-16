// TODO: Stop using base_dir where it's not a base directory but instead a filename.

#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "dobjects.h"
#include "param_parser.h"
#include "site_configuration.h"
#include "site_generator.h"

#define ERROR_BAD_PARAMETERS 1
#define ERROR_BAD_CONFIGURATION 2
#define ERROR_GENERATING_SITE 3
#define ERROR_OTHER 4
#define ERROR_VALIDATING_SITE 5

// GENERAL TODO: Fix includes across all files, some files include
// things they don't need.

typedef struct settings_struct {
	char* config_file;
	int show_help;
	int generate_site;
	int validate_site;
} settings_struct;

void show_help() {
	printf("spark --config <config file> [--generate-site | --validate-site]\n\n");
	printf("Spark is a dual-themed static blog site generator.\n");
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
	paramparser_get_flag(argc, argv, "--generate-site", &settings->generate_site);
	paramparser_get_flag(argc, argv, "--validate-site", &settings->validate_site);
	
	// Presently this is the only action, so if it's not given,
	// then that's a problem
	if(!settings->generate_site && !settings->validate_site) {
		fprintf(stderr, "Need either --generate-site or --validate-site\n");
		return 0;
	}
	return 1;
}
int validate_site(configuration_struct* configuration) {
	site_content_struct site_content;
	site_content_init(&site_content);
	int res = load_site_content(configuration, &site_content);
	site_content_free(&site_content);
	if(!res) {
		fprintf(stderr, "Invalid site\n");
	} else {
		printf("Site is valid\n");
	}
	return res;
}

int main(int argc, char* argv[]) {
	// TODO: Set proper permissions on all created directories and files.
	settings_struct settings;

	// Offset by 1 because we don't want to pass the program name
	if(!get_parameters(&settings, argc-1, &argv[1])) {
		fprintf(stderr, "Error, bad parameters\n");
		return ERROR_BAD_PARAMETERS;
	}

	if(settings.show_help) {
		return 0;
	}

	configuration_struct configuration;
	if(!load_configuration(&configuration, settings.config_file)) {
		fprintf(stderr, "Error, bad configuration\n");
		return ERROR_BAD_CONFIGURATION;
	}
	int res = 0;
	if(settings.generate_site) {
		res = generate_site(&configuration);
	} else if(settings.validate_site) {
		res = validate_site(&configuration);
	}

	dstring_free(&configuration.raw_config_file);

	if(!res && settings.generate_site) {
		return ERROR_GENERATING_SITE;
	} else if(!res &&settings.validate_site) {
		return ERROR_VALIDATING_SITE;
	}
	return 0;
}
