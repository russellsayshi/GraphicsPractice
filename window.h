#include <stdlib.h>

#define WINDOW_DECORATION_HEIGHT 10

typedef enum {
	false,
	true
} bool;

typedef struct {
	char (*buffer)[];
	unsigned int width;
	unsigned int height;
	unsigned int z_index;
	int x;
	int y;
	bool decoration;
	char exists;
	int id;
	void (*close_handler)();
} RWindow;

typedef char RColor;

typedef struct {
	RColor color;
} RGraphicsContext;

RWindow windows[10];

RGraphicsContext get_graphics_context(RWindow* window) {
	RGraphicsContext context;
	context.color = 0;
	return context;
}

void print_windows() {
	for(int i = 0; i < 10; i++) {
		if(windows[i].exists == 'Y') {
			printf("Window %d: number %d: z_index: %d\n", windows[i].id, i, windows[i].z_index);
		}
	}
}

#define WINDOW_MAX 10
void swap_windows(int index1, int index2) {
	if(index1 >= WINDOW_MAX || index2 >= WINDOW_MAX) {
		return;
	}
	printf("Swapping %d with %d\n", index1, index2);
	RWindow temp;
	temp = windows[index2];
	windows[index2] = windows[index1];
	windows[index1] = temp;
	windows[index1].z_index = index1;
	windows[index2].z_index = index2;
}

RWindow* set_window_z_index(RWindow* window, unsigned int z_index) {
	RWindow original = *window;

	printf("Setting z index of id %d\n", window->id);
	if(window->z_index == z_index) return NULL;
	printf("----- current situation:\n");
	print_windows();
	if(window->z_index != -1) {
		windows[window->z_index].exists = 'N'; //mark dead
		printf("----- marked_dead %d\n", window->z_index);
		print_windows();
		unsigned int z_index_track = z_index;
		while(windows[++z_index_track].exists == 'Y') {
			swap_windows(z_index_track-1, z_index_track);
		}
		printf("----- shifted windows to left to fill gap\n");
		print_windows();
	}
	if(windows[z_index].exists == 'Y') {
		//window already exists there
		unsigned int z_index_track = z_index;
		while(windows[++z_index_track].exists == 'Y') {}
		printf("z_index_track: %d\n", z_index_track);
		//z_index_track is now the index of the last window
		while(z_index_track-- != z_index) {
			swap_windows(z_index_track, z_index_track+1);
		}
	}
	printf("----- shifted windows to right to create gap\n");
	print_windows();
	original.z_index = z_index;
	original.exists = 'Y';
	windows[z_index] = original;
	printf("----- placed in window and marked existing\n");
	print_windows();
	return &windows[z_index];
}

int next_window_id = 0;
/* TODO limit window size & pos to height and width of screen */
RWindow* request_window(unsigned int width, unsigned int height, int x, int y) {
	RWindow window;
	window.width = width;
	window.height = height;
	window.x = x;
	window.y = y;
	window.id = next_window_id++;
	window.exists = 'Y';
	window.decoration = true;
	window.z_index = -1; //ensure we don't replace another window when doing z_index swap

	window.buffer = malloc(width*height);
	return set_window_z_index(&window, 0);
}