#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <getopt.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

struct s_select {
	Display		*dsp;
	Visual		*vi;
	Window		root_id;
	Window		win_id;
	Atom		wm_delete_window;
	Atom		_net_wm_state;
	Atom		_net_wm_state_above;

	uint32_t	x0;
	uint32_t	y0;
	uint32_t	x1;
	uint32_t	y1;
};

bool	x_select(uint32_t *, uint32_t *, uint32_t *, uint32_t *);
bool	x_selectCreate(struct s_select *);
bool	x_selectPollEvents(struct s_select *);
bool	x_selectTerminate(struct s_select *);

/* TODO(yakub):
 *  Add program options:
 *  - enumerated:
 *   - to print RGB values;
 *   - to print HSV values;
 *   - to print HEX values (default);
 *   - to print both RGB and HEX valeus;
 *   - to print both HSV and HEX valeus;
 *  - to print coordinates;
 *  - version;
 *  - help;
 * */



#define _min(a, b) (a < b ? a : b)
#define _abs(a) ((int32_t) (a) > 0 ? (uint32_t) (a) : (uint32_t) ((int32_t) (a) * -1))

#define XCOLPIC_FMT_RGB		1
#define XCOLPIC_FMT_RGB10	2
#define XCOLPIC_FMT_RGBA	3
#define XCOLPIC_FMT_RGBA10	4
#define XCOLPIC_FMT_HEX		5

static struct option	g_longopt[] = {
	{ "version", 0, 0, 'v'},
	{ "rgb",     0, 0,  0},
	{ "rgb10",     0, 0,  0},
	{ "rgba",     0, 0,  0},
	{ "rgba10",     0, 0,  0},
	{ "hex",     0, 0,  0},
	{ "help",    0, 0, 'h'},
	{ 0,         0, 0,  0 }
};

struct s_opt {
	uint8_t	format;
	bool	coord;
};

typedef struct s_opt	t_opt;

bool	x_colpicParseOpt(t_opt *, int, char **);
bool	x_colpicSelectData(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t **, uint32_t *);

int	main(int ac, char **av) {
	uint32_t	*d_int; /* d_int - data (int) */
	uint32_t	d_siz;  /* d_siz - size of d_int array */  
	uint32_t	x0, x1, /* x0 - start x, x1 - end x */
				y0, y1; /* y0 - start y, y1 - end y */
	t_opt		opt;
	
	opt = (t_opt) { .format = XCOLPIC_FMT_HEX, .coord = false };
	if (!x_colpicParseOpt(&opt, ac, av)) { return (1); }

	x0 = x1 = y0 = y1 = 0;
	if (!x_select(&x0, &y0, &x1, &y1)) {
		return (1);
	}

	d_int = (uint32_t *) 0;
	d_siz = 0;
	if (!x_colpicSelectData(_min(x0, x1), _min(y0, y1), _abs(x1 - x0), _abs(y1 - y0), &d_int, &d_siz)) { return (1); }

	for (size_t i = 0; i < d_siz; i++) {
		bool	skip;

		skip = false;
		for (int32_t j = i - 1; i > 0 && j > 0; j--) {
			if (d_int[i] == d_int[j]) {
				skip = true;
				break;
			}
		}
		if (skip) { continue; }

		switch (opt.format) {
			case (XCOLPIC_FMT_RGB): {
				printf(
					"rgb(%d, %d, %d)\n",
					(d_int[i] >> 8 * 3) & 0xff,
					(d_int[i] >> 8 * 2) & 0xff,
					(d_int[i] >> 8 * 1) & 0xff
				);
			} break;
			case (XCOLPIC_FMT_RGB10): {
				printf(
					"rgb(%1.1f, %1.1f, %1.1f)\n",
					((d_int[i] >> 8 * 3) & 0xff) / 255.0,
					((d_int[i] >> 8 * 2) & 0xff) / 255.0,
					((d_int[i] >> 8 * 1) & 0xff) / 255.0
				);
			} break;
			case (XCOLPIC_FMT_RGBA): {
				printf(
					"rgba(%d, %d, %d, %d)\n",
					(d_int[i] >> 8 * 3) & 0xff,
					(d_int[i] >> 8 * 2) & 0xff,
					(d_int[i] >> 8 * 1) & 0xff,
					(d_int[i] >> 8 * 0) & 0xff
				);
			} break;
			case (XCOLPIC_FMT_RGBA10): {
				printf(
					"rgba(%1.1f, %1.1f, %1.1f, %1.1f)\n",
					((d_int[i] >> 8 * 3) & 0xff) / 255.0,
					((d_int[i] >> 8 * 2) & 0xff) / 255.0,
					((d_int[i] >> 8 * 1) & 0xff) / 255.0,
					((d_int[i] >> 8 * 0) & 0xff) / 255.0
				);
			} break;
			case (XCOLPIC_FMT_HEX): {
				printf("#%x\n", d_int[i]);
			} break;
		}
	}

	if (d_int) { free(d_int), d_int = 0; }
	return (0);
}

/* x_select - selects a pixel data from a specific mouse-selected region.
 * - x0: returns the X coordinate of where the selection begun;
 * - y0: returns the Y coordinate of where the selection begun;
 * - x1: returns the X coordinate of where the selection finished;
 * - y1: returns the Y coordinate of where the selection finished;
 * */
bool	x_select(uint32_t *x0, uint32_t *y0, uint32_t *x1, uint32_t *y1) {
	struct s_select	s = { 0 };

	if (!x_selectCreate(&s)) { return (false); }
	while (x_selectPollEvents(&s)) { }
	if (!x_selectTerminate(&s)) { return (false); }

	if (x0) { *x0 = s.x0; }
	if (y0) { *y0 = s.y0; }
	if (x1) { *x1 = s.x1; }
	if (y1) { *y1 = s.y1; }
	return (true);
}

bool	x_selectCreate(struct s_select *s) {
	XSetWindowAttributes	attr;

	s->dsp = XOpenDisplay(0);
	if (!s->dsp) {
		return (false);
	}

	s->root_id = DefaultRootWindow(s->dsp);
	if (s->root_id == None) {
		XCloseDisplay(s->dsp);
		return (false);
	}

	s->vi = DefaultVisual(s->dsp, DefaultScreen(s->dsp));
	if (!s->vi) {
		XCloseDisplay(s->dsp);
		return (false);
	}

	attr = (XSetWindowAttributes) { 0 };
	attr.colormap = XCreateColormap(s->dsp, s->root_id, s->vi, false);
	attr.background_pixmap = 0;
	attr.border_pixel = 0;
	attr.override_redirect = true;
	attr.event_mask = StructureNotifyMask;

	s->win_id = XCreateWindow(
		s->dsp, s->root_id,
		0, 0,
		DisplayWidth(s->dsp, DefaultScreen(s->dsp)),
		DisplayHeight(s->dsp, DefaultScreen(s->dsp)),
		0, CopyFromParent, InputOutput, s->vi,
		CWOverrideRedirect | CWColormap | CWBackPixmap | CWBorderPixel | CWEventMask,
		&attr
	);

	if (s->win_id == None) {
		XCloseDisplay(s->dsp);
		return (false);
	}

	s->wm_delete_window = XInternAtom(s->dsp, "WM_DELETE_WINDOW", false);
	s->_net_wm_state = XInternAtom(s->dsp, "_NET_WM_STATE", false);
	s->_net_wm_state_above = XInternAtom(s->dsp, "_NET_WM_STATE_ABOVE", false);

	XChangeProperty(s->dsp, s->win_id, s->_net_wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *) &s->_net_wm_state_above, 1);
	XSetWMProtocols(s->dsp, s->win_id, &s->wm_delete_window, true);
	XSelectInput(s->dsp, s->win_id, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);
	
	XMapWindow(s->dsp, s->win_id);

	return (true);
}

bool	x_selectPollEvents(struct s_select *s) {
	XEvent	event;

	while (XPending(s->dsp)) {
		XNextEvent(s->dsp, &event);
		switch (event.type) {
			case (ClientMessage): {
				if (event.xclient.data.l[0] == (long) s->wm_delete_window) {
					return (false);
				}
			} break;

			case (ButtonPress): {
				s->x0 = event.xbutton.x;
				s->y0 = event.xbutton.y;
			} break;
			case (ButtonRelease): {
				s->x1 = event.xbutton.x;
				s->y1 = event.xbutton.y;
				return (false);
			} 

			/* NOTE(yakub):
			 *  We don't want any keyboard inputs during the program execution
			 *
			 * TODO(yakub):
			 *  This peace of code doesn't work; key inputs doesn't quit the program
			 * */
			case (KeyPress): {
				printf("Keyboard interrupt cancels the program execution...\n");
				XDestroyWindow(s->dsp, s->win_id);
				XCloseDisplay(s->dsp);
				exit (1);
			}
		}
	}
	return (true);
}

bool	x_selectTerminate(struct s_select *s) {
	XDestroyWindow(s->dsp, s->win_id);
	XCloseDisplay(s->dsp);

	return (true);
}



bool	x_colpicParseOpt(t_opt *opt, int ac, char **av) {
	int32_t	i;
	int8_t	c;

	/* TODO(yakub):
	 *  Parse options
	 * */
	(void) opt;
	while ((c = getopt_long(ac, av, "hv", g_longopt, &i)) != -1) {
		switch (c) {
			case ('h'): {
				printf("help\n");
				exit (0);
			} break;

			case ('v'): {
				printf("1.0\n");
				exit (0);
			} break;

			default: {
				if (i >= XCOLPIC_FMT_RGB && i <= XCOLPIC_FMT_HEX) {
					opt->format = i;
				}
				else {
					exit (1);
				}
			} break;
		}
	}
	return (true);
}

bool	x_colpicSelectData(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t **d, uint32_t *s) {
	uint32_t	*res;
	Display		*dsp;
	XImage		*img;
	Window		root;

	dsp = XOpenDisplay(0);
	if (!dsp) {
		return (false);
	}
	root = DefaultRootWindow(dsp);
	if (root == None) {
		XCloseDisplay(dsp);
		return (false);
	}

	if (!w) { w++; }
	if (!h) { h++; }
	img = XGetImage(dsp, root, x, y, w, h, AllPlanes, ZPixmap);
	if (!img) {
		XCloseDisplay(dsp);
		return (false);
	}

	res = (uint32_t *) calloc(w * h, sizeof(uint32_t));
	if (!res) {
		XDestroyImage(img);
		XCloseDisplay(dsp);
		return (false);
	}

	for (size_t i = 0; i < w * h; i++, (*s)++) {
		uint8_t	data[4];
		
		data[0] = 0xff;
		data[1] = img->data[i * 4 + 0];
		data[2] = img->data[i * 4 + 1];
		data[3] = img->data[i * 4 + 2];
		res[i] = *((uint32_t *) data);
	}
	
	*d = res;
	XDestroyImage(img);
	XCloseDisplay(dsp);
	return (true);
}
