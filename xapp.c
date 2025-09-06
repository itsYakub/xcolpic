#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#define min(a, b) ((int32_t) a > (int32_t) b ? b : a)
#define abs(a) ((int32_t) a > 0 ? a : -((int32_t) a))

struct s_select {
	Display	*dsp;
	Visual	*vi;
	Window	root_id;
	Window	win_id;
	Atom	wm_delete_window;
	Atom	_net_wm_state;
	Atom	_net_wm_state_above;

	int32_t	x0;
	int32_t	y0;
	int32_t	x1;
	int32_t	y1;
};

bool	x_select(int32_t *, int32_t *, int32_t *, int32_t *, uint32_t **);
bool	x_selectCreate(struct s_select *);
bool	x_selectPollEvents(struct s_select *);
bool	x_selectGetPixelData(struct s_select *, uint32_t **);
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
int	main(int ac, char **av) {
	uint32_t	*d0, *d1; /* d0 - unfiltered data, d1 - filtered data */
	int32_t		x0, x1,   /* x0 - start x, x1 - end x */
				y0, y1;   /* y0 - start y, y1 - end y */

	x0 = x1 = y0 = y1 = 0;
	d0 = d1 = 0;
	if (!x_select(&x0, &y0, &x1, &y1, &d0)) {
		return (1);
	}

	/* TODO(yakub):
	 *  We should print only the filtered pixels
	 * */
	for (size_t i = 0; d0[i]; i++) {
		printf("%x\n", d0[i]);
	}
	if (d0) { free(d0), d0 = 0; }
	if (d1) { free(d1), d1 = 0; }
	return (0);
}

/* x_select - selects a pixel data from a specific mouse-selected region.
 * - x0: returns the X coordinate of where the selection begun;
 * - y0: returns the Y coordinate of where the selection begun;
 * - x1: returns the X coordinate of where the selection finished;
 * - y1: returns the Y coordinate of where the selection finished;
 * - d:  returns an RGBA pixel data of the selected region;
 * */
bool	x_select(int32_t *x0, int32_t *y0, int32_t *x1, int32_t *y1, uint32_t **d) {
	struct s_select	s = { 0 };

	if (!x_selectCreate(&s)) { return (false); }
	while (x_selectPollEvents(&s)) { }
	if (!x_selectGetPixelData(&s, d)) { return (false); }
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
	if (!s->root_id) {
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

	if (!s->win_id) {
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
			 * */
			case (KeyPress): {
				printf("Keyboard interrupt cancels the program execution...\n");
				return (false);
			}
		}
	}
	return (true);
}


bool	x_selectGetPixelData(struct s_select *s, uint32_t **d) {
	XImage	*image;
	int32_t	x, y,
			w, h;

	x = min(s->x0, s->x1);
	y = min(s->y0, s->y1);
	w = abs(s->x1 - s->x0);
	if (!w) { w++; }
	h = abs(s->y1 - s->y0);
	if (!h) { h++; }

	image = XGetImage(
		s->dsp, s->root_id,
		x, y, w, h,
		AllPlanes,
		ZPixmap
	);
	if (!image) {
		return (false);
	}

	(*d) = (uint32_t *) calloc(w * h + 1, sizeof(uint32_t));
	if (!(*d)) {
		XDestroyImage(image);
		return (false);
	}

	for (size_t i = 0, s = w * h; i < s; i++) {
		char	pixel[4];
        
		pixel[3] = image->data[2]; 
		pixel[2] = image->data[1];
		pixel[1] = image->data[0];
		pixel[0] = 0xff; /* let's ensure that every pixel caught is opaque */
		(*d)[i] = *((uint32_t *) pixel);
	}

	XDestroyImage(image);
	return (true);
}

bool	x_selectTerminate(struct s_select *s) {
	XDestroyWindow(s->dsp, s->win_id);
	XCloseDisplay(s->dsp);

	return (true);
}
