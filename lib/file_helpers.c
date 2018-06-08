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
#include "file_helpers.h"



// Technically, I maybe should return -1 on error, but right now it doesn't.
int check_if_file_exists(dstring_struct* base_dir, const char* filename) {
	if(!dstring_append(base_dir, filename)) {
		fprintf(stderr, "Error checking if file %s%s exists, dstring append error\n", base_dir->str, filename);
		return 0;
	}
	int access_failure = access(base_dir->str, F_OK);
	dstring_remove_num_chars_in_text(base_dir, filename);
	return !access_failure;
}
int check_is_dir(const char* dir) {
	struct stat buffer;
	if(stat(dir, &buffer)) {
		return 0;
	}
	return (S_ISDIR(buffer.st_mode));
}
int try_check_dir_exists(dstring_struct* base_dir, const char* dir, const char* description) {
	if(!dstring_append(base_dir, dir)) {
		fprintf(stderr, "Unable to check if %s directory %s exists, dstring append error\n", description, dir);
		return 0;
	}
	int isdir = check_is_dir(base_dir->str);
	if(!isdir) {
		fprintf(stderr, "Error, missing directory %s in %s directory\n", dir, description);
	}
	dstring_remove_num_chars_in_text(base_dir, dir);
	return isdir;
}
int make_directory(dstring_struct* base_dir, const char* dir) {
	if(!dstring_append(base_dir, dir)) {
		fprintf(stderr, "Unable to create directory %s%s, dstring append error\n", base_dir->str, dir);
		return 0;
	}
	int mkdir_failed = mkdir(base_dir->str, 000755);
	if(mkdir_failed) {
		if(errno == EEXIST) {
			mkdir_failed = 0;
		} else {
			fprintf(stderr, "Unable to create directory %s\n", base_dir->str);
		}
	}
	dstring_remove_num_chars_in_text(base_dir, dir);
	return !mkdir_failed;
}
int is_html_filename(const char* filename) {
	size_t len = strlen(filename);
	return len >= 6
		&& filename[len-1] == 'l'
		&& filename[len-2] == 'm'
		&& filename[len-3] == 't'
		&& filename[len-4] == 'h'
		&& filename[len-5] == '.';
		
}
// Calling code must free the array when done.
darray_struct* get_html_filenames_in_directory(const char* directory) {
	darray_struct* filenames = malloc(sizeof(darray_struct));
	if(!filenames) {
		fprintf(stderr, "Error getting HTML filenames in %s, malloc error\n", directory);
		return NULL;
	}

	darray_lazy_init(filenames, sizeof(dstring_struct));

	DIR* dir = opendir(directory);
	if(dir == NULL) {
		fprintf(stderr, "Error getting HTML filenames, error opening directory %s\n", directory);
		darray_free(filenames);
		free(filenames);
		return NULL;
	}
	struct dirent* dir_ent;
	int had_error = 0;
	while(1) {
		dir_ent = readdir(dir);
		if(!dir_ent) break;
		// Need to remove '.' and '..'
		if(!strcmp(dir_ent->d_name, ".") || !strcmp(dir_ent->d_name, "..")) {
			continue;
		}
		if(!is_html_filename(dir_ent->d_name)) {
			continue;
		}
		// TODO: I should really check to make sure S_ISREG
		dstring_struct filename;
		dstring_lazy_init(&filename);

		if(!dstring_append(&filename, dir_ent->d_name)) {
			fprintf(stderr, "Error getting HTML filenames from directory %s, dstring append error\n", directory);
			had_error = 1;
			break;
		}
		if(!darray_append(filenames, &filename)) {
			fprintf(stderr, "Error getting HTML filenames from directory %s, darray append error\n", directory);
			had_error = 1;
			break;
		}
		
	}
	if(had_error) {
		for(size_t i = 0; i < filenames->length; i++) {
			dstring_free((dstring_struct*) darray_get_elem(filenames, i));
		}
		darray_free(filenames);
		free(filenames);
	}
	closedir(dir);
	if(had_error) {
		return NULL;
	}
	return filenames;
}
int remove_file_in_directory(dstring_struct* base_dir, const char* filename) {
	if(!dstring_append(base_dir, filename)) {
		fprintf(stderr, "Error removing file %s in directory %s, dstring append error\n", filename, base_dir->str);
		return 0;
	}
	int unlink_res = unlink(base_dir->str);
	dstring_remove_num_chars_in_text(base_dir, filename);
	if(unlink_res != 0) {
		fprintf(stderr, "Error removing file %s in directory %s, unlink error\n", filename, base_dir->str);
	} else {
		printf("Removed file %s%s\n", base_dir->str, filename);
	}
	return unlink_res == 0;
}
size_t get_file_extension_start(const char* filename) {
	size_t len = strlen(filename);
	if(len <= 2) {
		return 0;
	}
	size_t last_dot = len;
	for(size_t i = 1; i < len; i++) {
		if(filename[i] == '.') {
			last_dot = i;
		}
	}
	return last_dot;
}
