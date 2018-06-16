#include "dobjects.h"
#include "tag_posts.h"
#include "post.h"
void tag_posts_free(tag_posts_struct* tag_posts) {
	dstring_free(&tag_posts->tag);
	darray_free(&tag_posts->posts);
}
void tag_posts_init(tag_posts_struct* tag_posts) {
	dstring_lazy_init(&tag_posts->tag);
	darray_lazy_init(&tag_posts->posts, sizeof(post_struct*));
}
void tag_posts_print_debug(tag_posts_struct* tag_posts) {
	printf("Tag: %s\n", tag_posts->tag.str);
	printf("Number of posts: %zu\n", tag_posts->posts.length);
	for(size_t i = 0; i < tag_posts->posts.length; i++) {
		post_struct* post = post_get_from_darray_of_post_pointers(&tag_posts->posts, i);
		printf("Post %zu title:\t%s\n", i + 1, post->title.str);
	}
}
