#ifndef DOBJECTS_INCLUDE
#define DOBJECTS_INCLUDE
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

#define DSTRING_INITIAL_SIZE 1000
#define DSTRING_INCREMENT_SIZE 2000
#define DSTRING_FILE_READ_BLOCK_SIZE 500

#define DARRAY_INITIAL_SIZE 50
#define DARRAY_INCREMENT_SIZE 100



// Dynamic string
typedef struct dstring_struct {
	char* str;
	size_t total_length;
	size_t length;
} dstring_struct;

// Dynamic array.
// This is very similar in nature to dstring, but I wanted a dedicated
// data structure, and a set of dedicated function names, to dynamic
// strings, because it makes it a lot easier. Also, they grow at
// different rates, and darray doesn't need a NULL at the end.
typedef struct darray_struct {
	void* array;
	size_t total_length;
	size_t length;
	size_t elem_size;
} darray_struct;



darray_struct* darray_init_with_size(darray_struct* darray, size_t elem_size, size_t initial_size);
darray_struct* darray_init(darray_struct* darray, size_t elem_size);
void darray_free(darray_struct* darray);
darray_struct* darray_increase_size_specific_amount(darray_struct* darray, size_t additional_elems);
darray_struct* darray_increase_size(darray_struct* darray);
darray_struct* darray_append(darray_struct* darray, const void* elem);
void* darray_get_elem(darray_struct* darray, size_t index);
darray_struct* darray_clone(darray_struct* darray);
dstring_struct* dstring_init_with_size(dstring_struct* dstring, size_t initial_size);
dstring_struct* dstring_init(dstring_struct* dstring);
void dstring_free(dstring_struct* dstring);
void darray_of_dstrings_free(darray_struct* darray);
dstring_struct* dstring_resize(dstring_struct* dstring, size_t min_additional_bytes);
dstring_struct* dstring_append(dstring_struct* dstring, const char* text);
dstring_struct* dstring_append_printf(dstring_struct* dstring, const char* format, ...);
void dstring_remove_num_chars_in_text(dstring_struct* dstring, const char* text);
dstring_struct* dstring_read_file(dstring_struct* dstring, const char* file);
dstring_struct* dstring_read_process_output(dstring_struct* dstring, FILE* process_output, int* process_exit_code);
int dstring_write_file(dstring_struct* dstring, const char* file);
int dstring_write_file_if_different(dstring_struct* dstring, const char* filename, int* did_write);
int dstring_test();
int darray_test();
int dstring_try_load_file(dstring_struct* destination, dstring_struct* base_dir, const char* file, const char* filetype);
void dstring_remove_trailing_newlines(dstring_struct* dstring);
darray_struct* dstring_split_to_darray(dstring_struct* dstring, darray_struct* darray, char delimiter);
#endif
