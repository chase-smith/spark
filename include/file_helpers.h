#ifndef FILE_HELPERS_INCLUDE
#define FILE_HELPERS_INCLUDE
#include "dobjects.h"

int check_if_file_exists(dstring_struct*, const char*);
int check_is_dir(const char*);
int try_check_dir_exists(dstring_struct*, const char*, const char*);
int make_directory(dstring_struct*, const char*);
int is_html_filename(const char*);
darray_struct* get_html_filenames_in_directory(const char*);
int remove_file_in_directory(dstring_struct*, const char*);
size_t get_file_extension_start(const char*);
#endif
