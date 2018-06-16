#ifndef DOBJECTS_INCLUDE
#define DOBJECTS_INCLUDE
#define _BSD_SOURCE
#define _GNU_SOURCE
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
#define DSTRING_SMALL_INCREMENT_SIZE 50
#define DSTRING_FILE_READ_BLOCK_SIZE 500

#define DARRAY_INITIAL_SIZE 50
#define DARRAY_INCREMENT_SIZE 100
#define DARRAY_SMALL_INCREMENT_SIZE 5



// dstring_struct is a dynamic string object. Its primary use case is for
// appending strings to it, as would be the case when constructing a file
// from other strings. The string is automatically resized if appending
// more data than the string can hold at its present size.
typedef struct dstring_struct {
	// Points to a null-terminated C string.
	char* str;

	// How much space is actually allocated for the string.
	size_t total_length;

	// How long the string is. Will always be the same as strlen(dstring->str),
	// if the dstring is only modified by dstring_* functions.
	size_t length;
} dstring_struct;

// darray_struct is a dynamic array object. Its primary use case is for
// appending elements to it. The array is automatically resized when appending
// an element if the array is full. All elements must be the same size,
// and the size must be known when the darray is initialized.
// It is very important to note that elements being appended are
// _copied_ (using memcpy). This means that a function local variable
// (ie a variable on the stack) can be, and must be, copied to a darray by passing
// a pointer to the variable. This does have some implications for how elements
// are accessed and cleaned up, but the goal is to make it easier on calling code.
typedef struct darray_struct {
	// Points to the space allocated for the array.
	void* array;

	// How much space is actually allocated for the array.
	size_t total_length;

	// How long the array is.
	size_t length;

	// How much space is needed for each element in the array.
	size_t elem_size;
} darray_struct;


// dstringbuilder_struct is an object for composing dstrings in a memory-efficient
// fashion. It is meant to delay the allocation of a large string until it is
// actually needed. There are special functions associated with it for comparing
// a dstringbuilder to a file that are memory-efficient and do not assemble
// the full string.
// A dstringbuilder is composed of an array of dstrings and dstringbuilders.
// There are helper functions that will automatically allocate new instances of
// dstrings and dstringbuilders (instead of just appending a pointer to a pre-existing
// dstring or dstringbuilder). When the dstringbuilder is cleaned up, it will
// automatically free up any such objects it automatically created.
// When a new dstring is automatically allocated and appended, the property
// current_dstring will refer to that dstring; if the most recent item
// appended was not an automatically-allocated dstring, its value will be NULL.
// The point of this property is to point to the dstring that's at the end of the
// dstringbuilder, if it was created by the dstringbuilder, for ease of use
// in append operations by both the dstringbuilder and calling code.
typedef struct dstringbuilder_struct {
	// The array of dstrings and dstringbuilders. Must never be modified
	// directly, only use the provided dstringbuilder functions.
	darray_struct array;

	// The most recent dstring that was automatically allocated and appended;
	// will be NULL if the most recent item added was not an automatically
	// allocated and appended dstring.
	dstring_struct* current_dstring;
} dstringbuilder_struct;

// =========================
// = darray_struct functions
// =========================

// Initializes a darray with enough space to hold the specified
// number of elements.
// Returns NULL if there's an error.
darray_struct* darray_init_with_size(darray_struct* darray, size_t elem_size, size_t initial_size);

// Initializes a darray with enough space to hold the default (DARRAY_INITIAL_SIZE)
// number of elements.
// Returns NULL if there's an error.
darray_struct* darray_init(darray_struct* darray, size_t elem_size);

// Sets up a darray that will be able to hold elements of the specified size,
// but does not allocate any memory for the array (ie the array is 0 length).
// Will never fail.
void darray_lazy_init(darray_struct* darray, size_t elem_size);

// Will free the space used for the darray. Note, though, that the elements
// themselves are not free()'d, if any cleanup must be done of the elements
// in the darray, it must be done prior to calling darray_free()
void darray_free(darray_struct* darray);

// Increases the array size (and the amount of memory allocated for the array)
// by precisely the number of elements specified.
// Returns NULL if there's an error.
darray_struct* darray_increase_size_specific_amount(darray_struct* darray, size_t additional_elems);

// Increases the array size by either DARRAY_SMALL_INCREMENT_SIZE or
// DARRAY_INCREMENT_SIZE, depending on how big the array is.
// Returns NULL if there's an error.
darray_struct* darray_increase_size(darray_struct* darray);

// Appends a COPY of the element to the array (resizing if necessary).
// Returns NULL if there's an error.
darray_struct* darray_append(darray_struct* darray, const void* elem);

// Returns a POINTER to the element at the specified position.
// No bounds checking is performed.
static inline void* darray_get_elem(darray_struct* darray, size_t index) {
	return darray->array + (index * darray->elem_size);
}

// Creates and returns a pointer to a shallow copy of the specified darray.
// Note, because this is a shallow copy, callers must be careful to not
// destroy or free elements from the original darray if elements are pointers.
// The returned darray must be cleaned up using darray_free(), AND
// the pointer itself must be free()'d.
// Returns NULL if there's an error.
darray_struct* darray_clone(darray_struct* darray);



// ==========================
// = dstring_struct functions
// ==========================

// Initializes a dstring with the enough space to hold a string of
// the specified size.
// Returns NULL on error.
dstring_struct* dstring_init_with_size(dstring_struct* dstring, size_t initial_size);

// Initializes a dstring with enough space to hold a string of
// the default (DSTRING_INITIAL_SIZE) size.
dstring_struct* dstring_init(dstring_struct* dstring);

// Sets up a dstring, but does not allocate any memory for it.
// The length and total_length are set to zero, and the string itself
// is pointed at a static const empty string "".
// Will never fail.
void dstring_lazy_init(dstring_struct* dstring);

// Frees the space used by the dstring. Note, it does not free
// the pointer itself.
void dstring_free(dstring_struct* dstring);

// Frees each dstring in the specified darray, as well as the
// darray itself, but it does not free the pointer itself.
void darray_of_dstrings_free(darray_struct* darray);

// Increases the size of the dstring (and the amount of memory allocated
// for the dstring) by the specified number of bytes, plus 1.
// The extra byte is for the null terminator, so that calling code does not
// have to worry about accounting for it.
// Returns NULL on error.
dstring_struct* dstring_resize_no_extra(dstring_struct* dstring, size_t additional_bytes);

// Increases the size of the dstring by at least the specified amount.
// Depending on the size of the string, an additional DSTRING_SMALL_INCREMENT_SIZE
// or DSTRING_INCREMENT_SIZE bytes will be added.
// Returns NULL on error.
dstring_struct* dstring_resize(dstring_struct* dstring, size_t min_additional_bytes);

// Appends the specified text to the dstring, resizing as necessary.
// Returns NULL on error.
dstring_struct* dstring_append(dstring_struct* dstring, const char* text);

// Appends to the dstring in a printf style fashion, resizing as necessary.
// Returns NULL on error.
dstring_struct* dstring_append_printf(dstring_struct* dstring, const char* format, ...);
dstring_struct* dstring_append_vaprintf(dstring_struct* dstring, const char* format, va_list va_args);

// Sets the length and null terminator in the dstring back by the number
// of characters in the specified text.
void dstring_remove_num_chars_in_text(dstring_struct* dstring, const char* text);

// Reads the specified file, in its entirety, into the specified dstring.
// Returns NULL on error.
dstring_struct* dstring_read_file(dstring_struct* dstring, const char* filename);

// Reads the specified process output file into a dstring.
// Note, because this calls pclose, it is an error to pass a non-process-file
// (ie a regular file).
// Returns NULL if there is an error; otherwise, check process_exit_code
// for the exit code from the process.
dstring_struct* dstring_read_process_output(dstring_struct* dstring, FILE* process_stdout, int* process_exit_code);

// Writes the specified dstring into the specified filename.
// Returns 0 on error, 1 on success.
int dstring_write_file(dstring_struct* dstring, const char* filename);

// If the file doesn't exist, or the contents are different than the dstring,
// writes the dstring to the specified file.
// Returns 0 on error; otherwise, check did_write to see if the file was
// actually written.
int dstring_write_file_if_different(dstring_struct* dstring, const char* filename, int* did_write);

// A helper function; will attempt to load the file in the specified base
// directory, with the given filename, into the dstring. If there is an error,
// prints out a helpful message including the file_type; this is useful for
// loading in different kinds of files (eg a CSS file or an HTML file)
// and printing out more useful messages if there's an issue.
// Returns 0 on error, 1 on success.
int dstring_try_load_file(dstring_struct* dstring, dstring_struct* base_dir, const char* filename, const char* file_type);

// Modifies the dstring, removing any trailing '\r' or '\n' characters.
void dstring_remove_trailing_newlines(dstring_struct* dstring);

// In the specified dstring, all instances of the delimiter will be replaced
// will null bytes. If the delimiter is '\n', '\r' characters will also
// be replaced.
// The darray must already have been initialized to store char*'s.
// Each C string, terminated by a null byte, that is encountered in the dstring
// will be appended to the darray.
// This DOES modify dstring.
// Returns NULL on error.
darray_struct* dstring_split_to_darray(dstring_struct* dstring, darray_struct* darray, char delimiter);

// =================================
// = dstringbuilder_struct functions
// =================================

// Sets up a dstringbuilder for use.
// Because of the nature of how the dstringbuilder works, initialization
// only needs to be lazy, and since there's only the one way to initialize,
// this function is just called dstringbuilder_init.
void dstringbuilder_init(dstringbuilder_struct* dstringbuilder);

// Frees all memory and objects that this dstringbuilder created.
void dstringbuilder_free(dstringbuilder_struct* dstringbuilder);

// Appends a pointer to an existing dstring to the dstringbuilder.
// This dstring will not be cleaned up when the dstringbuilder is.
// Sets dstringbuilder->current_dstring to NULL.
// Returns NULL on error, otherwise the pointer to the dstring that was appended.
dstring_struct* dstringbuilder_append_dstring(dstringbuilder_struct* dstringbuilder, dstring_struct* dstring);

// Creates and appends a brand new dstring to the dstringbuilder.
// This dstring will be cleaned up when the dstringbuilder is.
// Sets dstringbuilder->current_dstring to the created string.
// Returns NULL on error, otherwise the pointer to the dstring that was created and appended.
dstring_struct* dstringbuilder_new_dstring(dstringbuilder_struct* dstringbuilder);

// Appends text to the dstringbuilder, by appending it to the dstring pointed
// to by dstringbuilder->current_dstring (creating and appending a new dstring
// if the current_dstring is NULL).
// Returns NULL on error.
dstring_struct* dstringbuilder_append(dstringbuilder_struct* dstringbuilder, const char* text);

// Appends text in a printf fashion to the dstringbuilder, by appending it to
// the dstring pointed to by dstringbuilder->current_dstring (creating and
// appending a new dstring if the current_dstring is NULL).
// Returns NULL on error.
dstring_struct* dstringbuilder_append_printf(dstringbuilder_struct* dstringbuilder, const char* format, ...);

// Appends a pointer to an existing dstringbuilder to the dstringbuilder.
// The appended dstringbuilder will not be cleaned up when the dstringbuilder is.
// Sets dstringbuilder->current_dstring to NULL.
// Returns NULL on error, otherwise the pointer to the dstringbuilder that was appended.
dstringbuilder_struct* dstringbuilder_append_dstringbuilder(dstringbuilder_struct* dstringbuilder, dstringbuilder_struct* append_me);

// Creates and appends a brand new dstringbuilder to the dstringbuilder.
// The appended dstringbuilder will be cleaned up when the dstringbuilder is.
// Sets dstringbuilder->current_dstring to NULL.
// Returns NULL on error, otherwise the pointer to the dstringbuilder that was
// created and appended.
dstringbuilder_struct* dstringbuilder_new_dstringbuilder(dstringbuilder_struct* dstringbuilder);

// Calculates the total length of the string described by the dstringbuilder.
size_t dstringbuilder_get_length(dstringbuilder_struct* dstringbuilder);

// Creates and returns a pointer to a new dstring, the content of which is the
// string described by the dstringbuilder.
// Calling code is responsible for freeing both the dstring and the pointer
// itself.
// Returns NULL on error.
dstring_struct* dstringbuilder_form(dstringbuilder_struct* dstringbuilder);

// Efficiently checks to see if the file is different from the string
// described by the dstringbuilder, and will only form the dstring to write
// it out if the file is different.
// If there was an error, 0 is returned; if there is no error, 1 is returned,
// and the int pointed to by did_write should be checked to see if the
// file was written.
int dstringbuilder_write_file_if_different(dstringbuilder_struct* dstringbuilder, const char* filename, int* did_write);

#endif
