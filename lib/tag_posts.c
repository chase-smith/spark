#include "dobjects.h"
#include "tag_posts.h"
#include "post.h"
void tag_posts_free(tag_posts_struct* tag_posts) {
	dstring_free(&tag_posts->tag);
	darray_free(&tag_posts->posts);
}
tag_posts_struct* tag_posts_init(tag_posts_struct* tag_posts) {
	dstring_lazy_init(&tag_posts->tag);
	darray_lazy_init(&tag_posts->posts, sizeof(post_struct*));
	return tag_posts;
}
void tag_posts_print_debug(tag_posts_struct* tag_posts) {
	printf("Tag: %s\n", tag_posts->tag.str);
	printf("# elems: %zu\n", tag_posts->posts.length);
	printf("elem size: %zu\n", tag_posts->posts.elem_size);
	post_struct** posts = (post_struct**) tag_posts->posts.array;
	for(size_t i = 0; i < tag_posts->posts.length; i++) {
		post_struct* p1 = (post_struct*) darray_get_elem(&tag_posts->posts, i);
		post_struct* p2 = posts[i];
		printf("Equal? %d\n", p1 == p2);
		post_print_debug(p2);
	}
}
