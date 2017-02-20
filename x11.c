#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "text.h"
#include "window.h"

#define SET_COLOR(color) (XSetForeground(dis, gc, (color)))
#define DRAW_DOT(x, y) (XDrawPoint(dis, win, gc, (x), (y)))
#define FLUSH() (XFlush((dis)))

Display* dis;
int screen;
Window win;
GC gc;

unsigned int width, height, border;
int win_x, win_y;

unsigned long black, white;

void init_x();
void close_x();
void handle_events();
void redraw();

int main(int argc, char** argv) {
	printf("Press r to redraw.\n");

	init_x();
	redraw();

	handle_events();

	close_x();
	return 0;
}

void update_width_height() {
	Window root;
	unsigned int depth;
	XGetGeometry(dis, win, &root, &win_x, &win_y, &width, &height, &border, &depth);
}

void draw_rect(int x, int y, int wid, int hei) {
	int x1 = x;
	int y1 = y;
	int x2 = x + wid;
	int y2 = y + hei;
	for(x = x1; x <= x2; x++) {
		for(y = y1; y <= y2; y++) {
			DRAW_DOT(x, y);
		}
	}
}

void draw_rect_window(int x, int y, int wid, int hei, RGraphicsContext gc, RWindow* window) {
	int x1 = x;
	int y1 = y;
	int x2 = x + wid;
	int y2 = y + hei;
	char* buffer = *(window->buffer);
	RColor color = gc.color;
	int bufferindex = y * window->width + x;
	int buffermax = window->width * window->height;
	for(y = y1; y <= y2; y++) {
		for(x = x1; x <= x2; x++) {
			//printf("Drawing %d out of %d\n", bufferindex, window->width * window->height);
			if(bufferindex >= buffermax) return;
			buffer[bufferindex] = color;
			bufferindex++;
		}
		bufferindex += window->width - (x2-x1) - 1;
	}
}

void draw_horizontal_line(int x, int wid, int y) {
	int x2 = wid + x;
	for(; x <= x2; x++) {
		DRAW_DOT(x, y);
	}
}

void draw_vertical_line(int y, int hei, int x) {
	int y2 = hei + y;
	for(; y <= y2; y++) {
		DRAW_DOT(x, y);
	}
}

void draw_box(int x, int y, int wid, int hei) {
	draw_horizontal_line(x, wid, y);
	draw_horizontal_line(x, wid, y+hei);
	draw_vertical_line(y, hei, x);
	draw_vertical_line(y, hei, x+wid);
}

void draw_letter(int letterIndex, int x, int y) {
	//printf("Drawing letter %c at %d, %d\n", letterIndex + 'A', x, y);
	for(int xp = 0; xp < LETTER_WIDTH; xp++) {
		for(int yp = 0; yp < LETTER_HEIGHT; yp++) {
			if(LETTERS[letterIndex][yp][xp] == '#') {
				DRAW_DOT(x + xp, y + yp);
			}
		}
	}
}

void draw_letter_window(int letterIndex, int x, int y, RGraphicsContext gc, RWindow* window) {
	//printf("Drawing letter %c at %d, %d\n", letterIndex + 'A', x, y);
	int bufferindex = y * window->width + x;
	char* buffer = *(window->buffer);
	RColor color = gc.color;
	for(int yp = 0; yp < LETTER_WIDTH; yp++) {
		for(int xp = 0; xp < LETTER_HEIGHT; xp++) {
			if(LETTERS[letterIndex][yp][xp] == '#') {
				buffer[bufferindex] = color;
			}
			bufferindex++;
		}
		bufferindex += window->width - LETTER_WIDTH;
	}
}

void draw_big_letter(int letterIndex, int x, int y) {
	//printf("Drawing letter %c at %d, %d\n", letterIndex + 'A', x, y);
	for(int xp = 0; xp < LETTER_WIDTH; xp++) {
		for(int yp = 0; yp < LETTER_HEIGHT; yp++) {
			if(LETTERS[letterIndex][yp][xp] == '#') {
				DRAW_DOT(x + (xp*2)  , y + (yp*2)  );
				DRAW_DOT(x + (xp*2)+1, y + (yp*2)  );
				DRAW_DOT(x + (xp*2)  , y + (yp*2)+1);
				DRAW_DOT(x + (xp*2)+1, y + (yp*2)+1);
			}
		}
	}
}

void draw_string_window(char* str, int x, int y, int maxx, int maxy, RGraphicsContext gc, RWindow* window) {
	int len = strlen(str);
	int xoriginal = x;
	for(int i = 0; i < len; i++) {
		if(str[i] >= 'A' && str[i] <= 'Z') {
			draw_letter_window(str[i] - 'A', x, y, gc, window);
		} else if(str[i] >= 'a' && str[i] <= 'z') {
			draw_letter_window(str[i] - 'a', x, y, gc, window);
		}
		x += LETTER_WIDTH + 1;
		if(x >= maxx) {
			y += LETTER_HEIGHT + 1;
			if(y + LETTER_HEIGHT >= maxy) {
				return;
			}
			x = xoriginal;
		}
	}
}

void draw_string(char* str, int x, int y, int maxx, int maxy) {
	int len = strlen(str);
	int xoriginal = x;
	for(int i = 0; i < len; i++) {
		if(str[i] >= 'A' && str[i] <= 'Z') {
			draw_letter(str[i] - 'A', x, y);
		} else if(str[i] >= 'a' && str[i] <= 'z') {
			draw_letter(str[i] - 'a', x, y);
		}
		x += LETTER_WIDTH + 1;
		if(x >= maxx) {
			y += LETTER_HEIGHT + 1;
			if(y + LETTER_HEIGHT >= maxy) {
				return;
			}
			x = xoriginal;
		}
	}
}

void draw_big_string(char* str, int x, int y, int maxx, int maxy) {
	int len = strlen(str);
	int xoriginal = x;
	for(int i = 0; i < len; i++) {
		if(str[i] >= 'A' && str[i] <= 'Z') {
			draw_big_letter(str[i] - 'A', x, y);
		} else if(str[i] >= 'a' && str[i] <= 'z') {
			draw_big_letter(str[i] - 'a', x, y);
		}
		x += LETTER_WIDTH*2 + 2;
		if(x >= maxx) {
			y += LETTER_HEIGHT*2 + 2;
			if(y + LETTER_HEIGHT*2 >= maxy) {
				return;
			}
			x = xoriginal;
		}
	}
}

void draw_window(RWindow* window) {
	SET_COLOR(white);
	int color = 0;
	char* buffer = *(window->buffer);
	//printf("buffer: %p\n", buffer);
	
	int starty = window->y;
	if(window->decoration) {
		starty += WINDOW_DECORATION_HEIGHT;
		draw_rect(window->x, window->y, window->width, WINDOW_DECORATION_HEIGHT);
		SET_COLOR(black);
		draw_box(window->x, window->y, window->width, WINDOW_DECORATION_HEIGHT);
		SET_COLOR(white);
	}
	for(int y = starty; y < starty + window->height; y++) {
		for(int x = window->x; x < window->x + window->width; x++) {
			//if(buffer[y*window.width + x] != 0) {
			//	printf("whoop %d %d, aka %d out of %d\n", x, y, y * window.width + x, window.width * window.height);
			//}
			if(buffer[(y - starty) * window->width + (x - window->x)] != color) {
				color = color == 0 ? 1 : 0;
				SET_COLOR(color == 0 ? white : black);
			}
			DRAW_DOT(x, y);
		}
	}
	SET_COLOR(black);
	draw_box(window->x, starty, window->width, window->height);
}

void redraw() {
	printf("Redrawing...\n");
	update_width_height();
	//printf("Width: %d, height: %d\n", width, height);

	SET_COLOR(white);
	draw_rect(0, 0, width-1, height-1);

	SET_COLOR(black);
	draw_big_string("abcdef ghijklmnopqrstuvwxyz hello world", 30, 30, width-30, height-30);
	
	for(int i = 10-1; i >= 0; i--) {
		//draw window
		if(windows[i].exists == 'Y') {
			draw_window(&windows[i]);
			//printf("Drawing window %d\n", i);
		}
	}
}

void right_click_handler(int x, int y) {
	/*SET_COLOR(black);
	draw_rect(x, y, x+30, y+50);
	
	SET_COLOR(white);
	draw_box(x, y, 30, 50);
	draw_string("File", x+5, y+5, x+25, y+45);
	draw_string("Edit", x+5, y+LETTER_HEIGHT+5, x+25, y+45);*/
	
	RWindow* window = request_window(20, 20, 20, 20);
	RGraphicsContext context = get_graphics_context(window);
	context.color = 1;
	draw_rect_window(5, 5, 15, 15, context, window);
	context.color = 0;
	draw_string_window("abc", 10, 10, window->width, window->height, context, window);
}

bool rect_contains(int x1, int y1, int x2, int y2, int x, int y) {
	return x >= x1 && x <= x2 && y >= y1 && y <= y2;
}

enum left_click_state_e {
	LCS_NORMAL,
	LCS_MOVEMENT
};

struct window_click_info {
	int index; //index of window clicked
	bool clicked_inside_window; //true if window was clicked. if false, no window found, all other info is invalid
	bool decoration; //true if window decoration was clicked, false if window interior clicked
};

struct window_click_info get_window_clicked(int x, int y) {
	for(int i = 0; i < 10; i++) {
		if(windows[i].exists != 'Y') break;
		int winx = windows[i].x;
		int winy = windows[i].y;
		int winwid = windows[i].width;
		int winhei = windows[i].height;
		if(windows[i].decoration) {
			winhei += WINDOW_DECORATION_HEIGHT;
		}
		if(rect_contains(winx, winy, winx + winwid, winy + winhei, x, y)) {
			//clicked inside window
			struct window_click_info ret;
			ret.index = i;
			ret.clicked_inside_window = true;
			ret.decoration = rect_contains(winx, winy, winx + winwid, winy + WINDOW_DECORATION_HEIGHT, x, y);
			return ret;
		} else {
			printf("%d, %d not contained in %d, %d, %d, %d\n", x, y, winx, winy, winx+winwid, winy+winhei);
		}
	}
	
	struct window_click_info ret;
	ret.clicked_inside_window = false;
	return ret;
}

enum left_click_state_e left_click_state = LCS_NORMAL;
void left_click_handler(int x, int y) {
	struct window_click_info info;
	switch(left_click_state) {
		case LCS_NORMAL:
			info = get_window_clicked(x, y);
			if(info.clicked_inside_window) {
				//we clicked in a window
				printf("Clicked window %d, decoration? %d\n", info.index, info.decoration);
				if(windows[info.index].z_index != 0) {
					printf("Setting z index of %d\n", info.index);
					set_window_z_index(&windows[info.index], 0);
				}
				if(info.decoration) {
					//clicked decoration
					left_click_state = LCS_MOVEMENT;
					//active_window_index = info.index;
				}
				redraw();
			} else {
				printf("Clicked in air.\n");
			}
			break;
		case LCS_MOVEMENT:
			windows[0].x = x;
			windows[0].y = y;
			left_click_state = LCS_NORMAL;
			redraw();
			break;
	}
}

void handle_events() {
	XEvent event;
	KeySym key;
	char text[255];

	while(1) {
		XNextEvent(dis, &event);

		if(event.type == Expose && event.xexpose.count == 0) {
			//redraw();
		}

		if(event.type == KeyPress &&
			XLookupString(&event.xkey, text, 255, &key, 0) == 1) {
			switch(text[0]) {
				case 'q':
					close_x();
					break;
				case 'r':
					redraw();
					break;
			}
			printf("You pressed the %c key!\n", text[0]);
		}

		if(event.type == ButtonPress) {
			//printf("You pressed a button at (%i, %i)\n", event.xbutton.x, event.xbutton.y);
			//XDrawPoint(dis, win, gc, event.xbutton.x, event.xbutton.y);
			//XFlush(dis);
			//DRAW_DOT(event.xbutton.x, event.xbutton.y);
			//FLUSH();
			int button = event.xbutton.button;
			if(button == 3) { //right click
				right_click_handler(event.xbutton.x, event.xbutton.y);
				redraw();
			} else if(button == 2) {
				windows[0].x = event.xbutton.x;
				windows[0].y = event.xbutton.y;
				redraw();
			} else if(button == 1) {
				left_click_handler(event.xbutton.x, event.xbutton.y);
			}
		}
	}
}

//WARNING all of the below is taken directly from
//http://math.msu.su/~vvb/2course/Borisenko/CppProjects/GWindow/xintro.html
//The purpose of this is for me to practice creating a graphics toolkit for an
//operating system I am writing, not to learn X11 (even though I am doing that)
//but to save time this is directly from the linked tutorial so I can focus on just
//drawing.
//X11 is just to practice and in the future this will be connected to no graphics
//toolkit and will run without an OS.

void init_x() {
	/* get the colors black and white (see section for details) */

	/* use the information from the environment variable DISPLAY 
	   to create the X connection:
	*/
	printf("If this segfaults, set your DISPLAY variable.\n");
	dis=XOpenDisplay((char *)0);
	screen=DefaultScreen(dis);
	black=BlackPixel(dis,screen),/* get color black */
	white=WhitePixel(dis, screen);  /* get color white */

	/* once the display is initialized, create the window.
	   This window will be have be 200 pixels across and 300 down.
	   It will have the foreground white and background black
	*/
	//printf("got to here\n");
	win=XCreateSimpleWindow(dis,DefaultRootWindow(dis),0,0,
		200, 300, 5, white, black);

	/* here is where some properties of the window can be set.
	   The third and fourth items indicate the name which appears
	   at the top of the window and the name of the minimized window
	   respectively.
	*/
	XSetStandardProperties(dis,win,"Graphics test","Hello World",None,NULL,0,NULL);

	/* this routine determines which types of input are allowed in
	   the input.  see the appropriate section for details...
	*/
	XSelectInput(dis, win, ExposureMask|ButtonPressMask|KeyPressMask);

	/* create the Graphics Context */
	gc=XCreateGC(dis, win, 0,0);

	/* here is another routine to set the foreground and background
	   colors _currently_ in use in the window.
	*/
	XSetBackground(dis,gc,black);
	XSetForeground(dis,gc,white);

	/* clear the window and bring it on top of the other windows */
	XClearWindow(dis, win);
	XMapRaised(dis, win);
}

void close_x() {
	XFreeGC(dis, gc);
	XDestroyWindow(dis, win);
	XCloseDisplay(dis);
	exit(1);
}
