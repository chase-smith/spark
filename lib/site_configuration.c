#include "file_helpers.h"
#include "site_configuration.h"
int try_get_config_value(int argc, char* argv[], const char* config_name, void* destination) {
	if(!paramparser_get_string(argc, argv, config_name, destination, PARAMPARSER_REQUIRED)) {
		fprintf(stderr, "Error, missing configuration setting %s\n", config_name);
		return 0;
	}
	return 1;
}
int load_configuration(configuration_struct* configuration, const char* config_file) {
	if(!dstring_init(&configuration->raw_config_file)) {
		fprintf(stderr, "Unable to initialize dstring for reading config file");
	}
	if(!dstring_read_file(&configuration->raw_config_file, config_file)) {
		fprintf(stderr, "Error loading config file\n");
		return 0;
	}
	if(configuration->raw_config_file.length == 0) {
		fprintf(stderr, "Loaded empty config file\n");
		return 0;
	}
	darray_struct lines;
	if(!darray_init(&lines, sizeof(char*))) {
		fprintf(stderr, "Error loading config file, darray init error\n");
		return 0;
	}
	if(!dstring_split_to_darray(&configuration->raw_config_file, &lines, '\n')) {
		fprintf(stderr, "Error reading config, dstring split error\n");
		darray_free(&lines);
		return 0;	
	}
	char** configv = (char**) lines.array;
	int did_load = try_get_config_value(lines.length, configv, "BRIGHT_HOST", &configuration->bright_host)
		&& try_get_config_value(lines.length, configv, "DARK_HOST", &configuration->dark_host)
		&& try_get_config_value(lines.length, configv, "BRIGHT_NAME", &configuration->bright_name)
		&& try_get_config_value(lines.length, configv, "DARK_NAME", &configuration->dark_name)
		&& try_get_config_value(lines.length, configv, "HTML_BASE_DIR", &configuration->html_base_dir)
		&& try_get_config_value(lines.length, configv, "CODE_BASE_DIR", &configuration->code_base_dir)
		&& try_get_config_value(lines.length, configv, "SITE_GROUP", &configuration->site_group)
		&& try_get_config_value(lines.length, configv, "RSS_DESCRIPTION", &configuration->rss_description);

	
	darray_free(&lines);
	return did_load;
}
int do_pre_validations(configuration_struct* configuration) {
	dstring_struct base_dir;
	dstring_init(&base_dir);
	dstring_append(&base_dir, configuration->code_base_dir);
	int code_base_dirs_exist = try_check_dir_exists(&base_dir, "/posts", "code base")
		&& try_check_dir_exists(&base_dir, "/components", "code base")
		&& try_check_dir_exists(&base_dir, "/series", "code base")
		&& try_check_dir_exists(&base_dir, "/misc_pages", "code base")
		&& try_check_dir_exists(&base_dir, "/themes", "code base")
		&& try_check_dir_exists(&base_dir, "/themes/dark", "code base")
		&& try_check_dir_exists(&base_dir, "/themes/bright", "code base");
	if(!code_base_dirs_exist) {
		dstring_free(&base_dir);
		return 0;
	}
	// Reusing the dstring for efficiency
	dstring_remove_num_chars_in_text(&base_dir, configuration->code_base_dir);
	dstring_append(&base_dir, configuration->html_base_dir);
	int html_base_dirs_exist = try_check_dir_exists(&base_dir, "/bright", "HTML base")
		&& try_check_dir_exists(&base_dir, "/dark", "HTML base");
	
	if(!html_base_dirs_exist) {
		dstring_free(&base_dir);
		return 0;
	}
	
	int make_html_dirs_res = make_directory(&base_dir, "/bright/posts")
		&& make_directory(&base_dir, "/bright/series")
		&& make_directory(&base_dir, "/bright/tags")
		&& make_directory(&base_dir, "/dark/posts")
		&& make_directory(&base_dir, "/dark/series")
		&& make_directory(&base_dir, "/dark/tags");
	if(!make_html_dirs_res) {
		dstring_free(&base_dir);
		return 0;
	}	
	dstring_free(&base_dir);

	
	return 1;
}

