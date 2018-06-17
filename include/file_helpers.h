#ifndef FILE_HELPERS_INCLUDE
#define FILE_HELPERS_INCLUDE
#include "dobjects.h"

// file_helpers contains a variety of useful helper functions
// for dealing with files and directories.
// Several functions take the base directory as a dstring,
// and the filename or directory name as a const char* string;
// this is to make it easier on calling code. In these functions,
// the dstring will be modified, but it will always be reverted
// back to its previous state before returning (though if more
// memory was allocated, that is not undone).

// ========================
// = file_helpers functions
// ========================

// Checks to see if the specified file exists in the given directory.
// Returns 1 if it does, 0 otherwise.
int check_if_file_exists(dstring_struct* base_dir, const char* filename);

// Checks to see if the item at the specified path is a directory.
// Returns 1 if it is, 0 otherwise.
int check_is_dir(const char* path);

// Checks to see if the specified dir exists in the given base_dir, printing
// out a helpful error message that includes the description if an error was
// encountered or the directory does not exist.
// Returns 1 if it exists, 0 otherwise.
int try_check_dir_exists(dstring_struct* base_dir, const char* dir, const char* description);

// Will make the specified directory if it does not exist.
// It is not an error if the directory already exists.
// Returns 0 if there was an error, 1 otherwise.
int make_directory(dstring_struct* base_dir, const char* dir);

// Returns 1 if the filename ends in ".html", 0 otherwise.
int is_html_filename(const char* filename);

// Creates a darray of dstrings of all of the HTML files in the specified
// directory. Calling code must free the dstrings, as well as the darray,
// as well as the pointer itself (this is where darray_of_dstrings_free
// comes in).
// Returns NULL on error, the darray otherwise.
darray_struct* get_html_filenames_in_directory(const char* directory);

// Removes the specified file in the given base directory.
// Returns 0 on error.
int remove_file_in_directory(dstring_struct* base_dir, const char* file);

// Returns the start of the file extension, or 0 or the length of the string
// if it wasn't found.
size_t get_file_extension_start(const char*);

#endif
