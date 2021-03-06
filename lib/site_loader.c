#include "site_loader.h"
int load_themes(configuration_struct* configuration, site_content_struct* site_content) {
	dstring_struct base_dir;

	dstring_lazy_init(&base_dir);

	if(!dstring_append(&base_dir, configuration->content_base_dir) 
		|| !dstring_append(&base_dir, "/themes/bright")) {
		fprintf(stderr, "Error loading themes, dstring append error\n");
		dstring_free(&base_dir);
		return 0;
	}
	if(!theme_load(&site_content->bright_theme, &base_dir)) {
		fprintf(stderr, "Error loading bright theme\n");
		dstring_free(&base_dir);
		return 0;
	}
	dstring_remove_num_chars_in_text(&base_dir, "bright");
	if(!dstring_append(&base_dir, "dark")) {
		fprintf(stderr, "Error loading dark theme, dstring append error\n");
		dstring_free(&base_dir);
		return 0;
	}
	if(!theme_load(&site_content->dark_theme, &base_dir)) {
		fprintf(stderr, "Error loading dark theme\n");
		dstring_free(&base_dir);
		return 0;
	}
	dstring_free(&base_dir);
	site_content->dark_theme.alt_theme = &site_content->bright_theme;
	site_content->bright_theme.alt_theme = &site_content->dark_theme;

	if(!dstring_append(&site_content->dark_theme.host, configuration->dark_host)
		|| !dstring_append(&site_content->dark_theme.name, configuration->dark_name)
		|| !dstring_append(&site_content->dark_theme.html_base_dir, configuration->html_base_dir)
		|| !dstring_append(&site_content->dark_theme.html_base_dir, "/dark")) {
		fprintf(stderr, "Error configuring dark theme settings\n");
		return 0;
	}
	if(!dstring_append(&site_content->bright_theme.host, configuration->bright_host)
		|| !dstring_append(&site_content->bright_theme.name, configuration->bright_name)
		|| !dstring_append(&site_content->bright_theme.html_base_dir, configuration->html_base_dir)
		|| !dstring_append(&site_content->bright_theme.html_base_dir, "/bright")) {
		fprintf(stderr, "Error configuring bright theme settings\n");
		return 0;
	}
	return 1;
}
int load_html_components(configuration_struct* configuration, site_content_struct* site_content) {
	dstring_struct base_dir;

	dstring_lazy_init(&base_dir);

	if(!dstring_append(&base_dir, configuration->content_base_dir)
		|| !dstring_append(&base_dir, "/components")) {
		fprintf(stderr, "Error loading HTML components, dstring append error\n");
		dstring_free(&base_dir);
		return 0;
	}
	if(!html_components_load(&site_content->html_components, &base_dir)) {
		fprintf(stderr, "Error loading HTML components\n");
		dstring_free(&base_dir);
		return 0;
	}
	dstring_free(&base_dir);
	return 1;
}

int series_sort_compare(const void* series_a, const void* series_b) {
	return ((series_struct*)series_a)->order - ((series_struct*)series_b)->order;
}
int load_single_series(dstring_struct* base_dir, struct dirent* dir_ent, void* site_content_void_ptr) {
	site_content_struct* site_content = site_content_void_ptr;

	if(!dstring_append(base_dir, dir_ent->d_name)) {
		fprintf(stderr, "Error loading single series, dstring append error\n");
		return 0;
	}
	series_struct tmp_series_entry;
	series_init(&tmp_series_entry);
	
	if(!series_load(&tmp_series_entry, base_dir, dir_ent->d_name)) {
		fprintf(stderr, "Warning, series folder %s doesn't contain a valid series. Skipping\n", dir_ent->d_name);
		dstring_remove_num_chars_in_text(base_dir, dir_ent->d_name);
		series_free(&tmp_series_entry);
		return 1;
	}
	dstring_remove_num_chars_in_text(base_dir, dir_ent->d_name);
	// OK, so we loaded the series in. Store it in the array, we'll
	// sort it later.
	if(!darray_append(&site_content->series, &tmp_series_entry)) {
		fprintf(stderr, "Error reading series data, darray append error\n");
		series_free(&tmp_series_entry);
		return 0;
	}
	return 1;
}
int load_series(configuration_struct* configuration, site_content_struct* site_content) {
	dstring_struct base_dir;

	dstring_lazy_init(&base_dir);

	if(!dstring_append(&base_dir, configuration->content_base_dir)
		|| !dstring_append(&base_dir, "/series/")) {
		fprintf(stderr, "Error loading all series, dstring append error\n");
		dstring_free(&base_dir);
		return 0;
	}
	int res = apply_function_to_directory_entries(&base_dir, 0, DT_DIR, load_single_series, site_content);
	dstring_free(&base_dir);
	if(!res) {
		return 0;
	}
	// Series will be sorted using qsort, which sorts in-place.
	qsort(site_content->series.array, site_content->series.length, site_content->series.elem_size, &series_sort_compare);

	return 1;
	
}
int load_single_misc_page(dstring_struct* base_dir, struct dirent* dir_ent, void* site_content_void_ptr) {
	site_content_struct* site_content = site_content_void_ptr;
	if(!dstring_append(base_dir, dir_ent->d_name)) {
		fprintf(stderr, "Error loading single misc page, dstring append error\n");
		return 0;
	}
	misc_page_struct tmp_misc_page_entry;
	misc_page_init(&tmp_misc_page_entry);
	if(!misc_page_load(&tmp_misc_page_entry, base_dir)) {
		fprintf(stderr, "Error, misc_pages folder %s doesn't contain a valid misc page.\n", dir_ent->d_name);
		dstring_remove_num_chars_in_text(base_dir, dir_ent->d_name);
		misc_page_free(&tmp_misc_page_entry);
		return 0;
	}
	dstring_remove_num_chars_in_text(base_dir, dir_ent->d_name);
	// OK, so we loaded the misc page in. Store it.
	if(!darray_append(&site_content->misc_pages, &tmp_misc_page_entry)) {
		fprintf(stderr, "Error reading misc_pages data, darray append error\n");
		misc_page_free(&tmp_misc_page_entry);
		return 0;
	}
	return 1;
}
int load_misc_pages(configuration_struct* configuration, site_content_struct* site_content) {
	dstring_struct base_dir;

	dstring_lazy_init(&base_dir);

	if(!dstring_append(&base_dir, configuration->content_base_dir)
		|| !dstring_append(&base_dir, "/misc_pages/")) {
		fprintf(stderr, "Error loading all misc_pages, dstring append error\n");
		dstring_free(&base_dir);
		return 0;
	}

	int res = apply_function_to_directory_entries(&base_dir, 0, DT_DIR, load_single_misc_page, site_content);
	dstring_free(&base_dir);
	if(!res) {
		return 0;
	}
	return 1;
	
}

int post_sort_compare(const void* post_a, const void* post_b) {
	return ((post_struct*)post_b)->written_date_time - ((post_struct*)post_a)->written_date_time;
}

// Returns 0 on failure...
time_t read_time_from_str(const char* str) {
	long long scanned_time = 0;
	int num_read = sscanf(str, "%lld", &scanned_time);
	if(num_read == 0 || num_read == EOF) {
		return 0;
	}
	return scanned_time;
}
/** NOTE ABOUT TIMEZONES:
 * C doesn't have great functions for dealing with timezones.
 *
 * So what I'll do is this. I'll read all of the posts in (doing validations).
 * I'll have my list of posts.
 * For every post, I'll write out the written-date, and if present, publish-after
 * values. I'll call `date -f $file +%s`. I'll read in the results, put into
 * a dstring. I'll split the dstring on '\n' into a darray. I should (will
 * need to validate) have a list that's got as many entries as I do dates
 * that were written out. I'll then go through the posts and the darray;
 * one line will be taken as the written-date, and if the publish-after
 * value is set, then another line will be taken as the publish-after value.
 * If a bad date is encountered, I'll know which post, and which field,
 * is invalid. Error messages will be printed out for all posts that have invalid
 * dates, because why not.
 * Ah, also, I can write out a line with the text "now", this will save me
 * some unpleasant work with gmtime and such. Huzzah!
 */

int load_post_dates(configuration_struct* configuration, site_content_struct* site_content) {
	dstring_struct base_dir;
	dstring_struct out_dates;
	dstring_struct date_command;
	dstring_struct date_output;
	darray_struct read_dates;

	dstring_lazy_init(&base_dir);
	dstring_lazy_init(&out_dates);
	dstring_lazy_init(&date_command);
	dstring_lazy_init(&date_output);
	darray_lazy_init(&read_dates, sizeof(char*));

	if(!dstring_append(&base_dir, configuration->content_base_dir)
		|| !dstring_append(&base_dir, "/generating/post_dates")) {
		fprintf(stderr, "Error getting post dates, base_dir dstring append error\n");
		dstring_free(&base_dir);
		return 0;
	}
	if(!dstring_append(&out_dates, "now\n")) {
		fprintf(stderr, "Error getting post dates, out_dates dstring append error\n");
		dstring_free(&base_dir);
		dstring_free(&out_dates);
		return 0;
	}
	int had_error = 0;
	size_t num_dates_written = 1;
	for(size_t i = 0; i < site_content->posts.length; i++) {
		post_struct* post = post_get_from_darray(&site_content->posts, i);
		if(!dstring_append(&out_dates, post->written_date.str)
			|| !dstring_append(&out_dates, "\n")) {
			fprintf(stderr, "Error appending written_date\n");
			had_error = 1;
			break;
		}
		num_dates_written++;
		if(post->publish_after.length > 0) {
			if(!dstring_append(&out_dates, post->publish_after.str)
				|| !dstring_append(&out_dates, "\n")) {
				fprintf(stderr, "Error appending publish_after\n");
				had_error = 1;
				break;
			}
			num_dates_written++;
		}
		if(post->updated_at.length > 0) {
			if(!dstring_append(&out_dates, post->updated_at.str)
				|| !dstring_append(&out_dates, "\n")) {
				fprintf(stderr, "Error appending updated_at\n");
				had_error = 1;
				break;
			}
			num_dates_written++;
		}
	}
	if(had_error) {
		dstring_free(&base_dir);
		dstring_free(&out_dates);
		return 0;
	}
	// Now to write it out...
	if(!dstring_write_file(&out_dates, base_dir.str)) {
		fprintf(stderr, "Error writing post dates to file\n");
		dstring_free(&base_dir);
		dstring_free(&out_dates);
		return 0;
	}
	if(!dstring_append(&date_command, "date 2>/dev/null -f ")
		|| !dstring_append(&date_command, base_dir.str)
		|| !dstring_append(&date_command, " +%s")) {
		fprintf(stderr, "Error with post dates, date_command dstring append error\n");
		dstring_free(&base_dir);
		dstring_free(&out_dates);
		dstring_free(&date_command);
		return 0;
	}
	int process_exit_code;
	if(!dstring_read_process_output(&date_output, popen(date_command.str, "r"), &process_exit_code)) {
		fprintf(stderr, "Error running date command\n");
		dstring_free(&date_output);
		dstring_free(&base_dir);
		dstring_free(&out_dates);
		dstring_free(&date_command);
		return 0;
	}
	if(!dstring_split_to_darray(&date_output, &read_dates, '\n')) {
		fprintf(stderr, "Error with post dates, dstring split error\n");
		dstring_free(&date_output);
		dstring_free(&base_dir);
		dstring_free(&out_dates);
		dstring_free(&date_command);
		darray_free(&read_dates);
		return 0;
	}
	// Now we have the lines, we need to figure out which, if any, failed...
	if(num_dates_written != read_dates.length) {
		fprintf(stderr, "Error with post dates, didn't read in the same number of dates as written out: Wrote %zu, read %zu\n", num_dates_written, read_dates.length);
		dstring_free(&date_output);
		dstring_free(&base_dir);
		dstring_free(&out_dates);
		dstring_free(&date_command);
		darray_free(&read_dates);
		return 0;
	}
	time_t now_time = read_time_from_str(((const char**)read_dates.array)[0]);
	if(!now_time) {
		fprintf(stderr, "Error with post dates, couldn't read now time\n");
		dstring_free(&date_output);
		dstring_free(&base_dir);
		dstring_free(&out_dates);
		dstring_free(&date_command);
		darray_free(&read_dates);
		return 0;
	}
	site_content->current_time = now_time;
	// Now, finally, we get to read in the post dates...
	size_t date_index = 1;
	const char** read_dates_strs = (const char**) read_dates.array;
	// Note: We know that we won't go out of bounds of the array,
	// because we already checked that we read in as many dates as we wrote out,
	// and this is the exact same order/logic used for writing, and nothing
	// has changed the data used for these conditions in the meantime.
	// Also, we aren't bailing on errors here, because we already know
	// if there are errors from the exit code.
	for(size_t i = 0; i < site_content->posts.length; i++) {
		time_t written_date_time = read_time_from_str(read_dates_strs[date_index++]);
		post_struct* post = post_get_from_darray(&site_content->posts, i);
		if(!written_date_time) {
			fprintf(stderr, "Error, written-date %s for post %s invalid.\n", post->written_date.str, post->folder_name.str);
		} else {
			post->written_date_time = written_date_time;
		}
		if(post->publish_after.length > 0) {
			time_t publish_after_time = read_time_from_str(read_dates_strs[date_index++]);
			if(!publish_after_time) {
				fprintf(stderr, "Error, publish-after time %s for post %s invalid.\n", post->publish_after.str, post->folder_name.str);
			} else {
				post->publish_after_time = publish_after_time;
				if(post->publish_when_ready) {
					if(publish_after_time <= now_time) {
						post->can_publish = 1;
					}
				}
			}
		} else {
			if(post->publish_when_ready) {
				post->can_publish = 1;
			}
		}
		if(post->updated_at.length > 0) {
			time_t updated_at_time = read_time_from_str(read_dates_strs[date_index++]);
			if(!updated_at_time) {
				fprintf(stderr, "Error, updated-at time %s for post %s invalid.\n", post->updated_at.str, post->folder_name.str);
			} else {
				post->updated_at_time = updated_at_time;
			}
		}
	}
	dstring_free(&date_output);
	dstring_free(&base_dir);
	dstring_free(&out_dates);
	dstring_free(&date_command);
	darray_free(&read_dates);
	if(process_exit_code) {
		fprintf(stderr, "Error, not all post dates were valid.\n");
		return 0;
	}
	return 1;
}

int load_single_post(dstring_struct* base_dir, struct dirent* dir_ent, void* site_content_void_ptr) {
	site_content_struct* site_content = site_content_void_ptr;
	if(!dstring_append(base_dir, dir_ent->d_name)) {
		fprintf(stderr, "Error loading single post, dstring append error\n");
		return 0;
	}

	post_struct tmp_post_entry;
	post_init(&tmp_post_entry);

	int generate_flag_missing = 0;
	if(!post_load(&tmp_post_entry, base_dir, dir_ent->d_name, &generate_flag_missing) && generate_flag_missing) {
		fprintf(stderr, "Skipping post %s because generate flag is missing\n", dir_ent->d_name);
		dstring_remove_num_chars_in_text(base_dir, dir_ent->d_name);
		post_free(&tmp_post_entry);
		return 1;
	}

	dstring_remove_num_chars_in_text(base_dir, dir_ent->d_name);

	if(!darray_append(&site_content->posts, &tmp_post_entry)) {
		fprintf(stderr, "Error reading post data, darray append error\n");
		post_free(&tmp_post_entry);
		return 0;
	}
	return 1;
}

// TODO: Consider putting posts into series even if they won't be published; then, check
// whether or not a post can be published when printing it out
int load_posts(configuration_struct* configuration, site_content_struct* site_content) {
	// Posts will have their dates validated after all are loaded in.
	// This is due to the timezone conversion stuff mentioned at the top
	// of the program.
	dstring_struct base_dir;

	dstring_lazy_init(&base_dir);

	if(!dstring_append(&base_dir, configuration->content_base_dir)
		|| !dstring_append(&base_dir, "/posts/")) {
		fprintf(stderr, "Error loading all posts, dstring append error\n");
		dstring_free(&base_dir);
		return 0;
	}
	int res = apply_function_to_directory_entries(&base_dir, 0, DT_DIR, load_single_post, site_content);
	dstring_free(&base_dir);
	if(!res) {
		return 0;
	}
	// OK... So now that they are all loaded, we need to validate dates.
	// We do that by calling `date`.
	if(!load_post_dates(configuration, site_content)) {
		fprintf(stderr, "Error loading post dates\n");
		return 0;
	}
	qsort(site_content->posts.array, site_content->posts.length, site_content->posts.elem_size, &post_sort_compare);
	if(!validate_posts(site_content)) {
		fprintf(stderr, "Error validating posts\n");
		return 0;
	}
	return 1;
	
}
int load_site_content(configuration_struct* configuration, site_content_struct* site_content) {
	if(!do_pre_validations(configuration)) {
		return 0;
	}
	if(!load_themes(configuration, site_content)) {
		fprintf(stderr, "Error loading themes\n");
		return 0;
	}
	if(!load_html_components(configuration, site_content)) {
		fprintf(stderr, "Error loading HTML components\n");
		return 0;
	}
	if(!load_misc_pages(configuration, site_content)) {
		fprintf(stderr, "Error loading misc_pages\n");
		return 0;
	}
	// We DO require a misc_page for index.html
	if(!find_misc_page_by_filename(site_content, "index.html")) {
		fprintf(stderr, "Error, missing index.html misc_page\n");
		return 0;
	}
	if(!load_series(configuration, site_content)) {
		fprintf(stderr, "Error loading series data\n");
		return 0;
	}
	if(!load_posts(configuration, site_content)) {
		fprintf(stderr, "Error loading posts\n");
		return 0;
	}
	if(!site_content_setup_tags(site_content)) {
		fprintf(stderr, "Error setting up tags\n");
		return 0;
	}
	return 1;
}
