#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

struct s_select {
	Display	*dsp;
	Visual	*vi;
	Window	root_id;
	Window	win_id;
	Atom	wm_delete_window;
	Atom	_net_wm_state;
	Atom	_net_wm_state_above;

	size_t	x0;
	size_t	y0;
	size_t	x1;
	size_t	y1;
};

bool	x_select(size_t *, size_t *, size_t *, size_t *, unsigned char *);
bool	x_selectCreate(struct s_select *);
bool	x_selectPollEvents(struct s_select *);
bool	x_selectTerminate(struct s_select *);

int	main(void) {
	size_t	x0, x1,
			y0, y1;

	x0 = x1 = y0 = y1 = 0;
	if (!x_select(&x0, &y0, &x1, &y1, 0)) {
		return (1);
	}

	printf("SELECTION: START:  %4zu %4zu\n", x0, y0);
	printf("SELECTION: FINISH: %4zu %4zu\n", x1, y1);
	return (0);
}

bool	x_selectPollEvents(struct s_select *cli) {
	XEvent	event;

	while (XPending(cli->dsp)) {
		XNextEvent(cli->dsp, &event);
		switch (event.type) {
			case (ClientMessage): {
				if (event.xclient.data.l[0] == (long) cli->wm_delete_window) {
					return (false);
				}
			} break;

			case (ButtonPress): {
				cli->x0 = event.xbutton.x;
				cli->y0 = event.xbutton.y;
			} break;
			case (ButtonRelease): {
				cli->x1 = event.xbutton.x;
				cli->y1 = event.xbutton.y;
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

/* x_select - selects a pixel data from a specific mouse-selected region.
 * - x0: returns the X coordinate of where the selection begun;
 * - y0: returns the Y coordinate of where the selection begun;
 * - x1: returns the X coordinate of where the selection finished;
 * - y1: returns the Y coordinate of where the selection finished;
 * - d:  returns an RGBA pixel data of the selected region;
 * */
bool	x_select(size_t *x0, size_t *y0, size_t *x1, size_t *y1, unsigned char *d) {
	struct s_select	cli = { 0 };

	if (!x_selectCreate(&cli)) { return (false); }
	while (x_selectPollEvents(&cli)) { }
	if (!x_selectTerminate(&cli)) { return (false); }

	if (x0) { *x0 = cli.x0; }
	if (y0) { *y0 = cli.y0; }
	if (x1) { *x1 = cli.x1; }
	if (y1) { *y1 = cli.y1; }
	return (true);
}

bool	x_selectCreate(struct s_select *cli) {
	XSetWindowAttributes	attr;

	cli->dsp = XOpenDisplay(0);
	if (!cli->dsp) {
		return (false);
	}

	cli->root_id = DefaultRootWindow(cli->dsp);
	if (!cli->root_id) {
		XCloseDisplay(cli->dsp);
		return (false);
	}

	cli->vi = DefaultVisual(cli->dsp, DefaultScreen(cli->dsp));
	if (!cli->vi) {
		XCloseDisplay(cli->dsp);
		return (false);
	}

	attr = (XSetWindowAttributes) { 0 };
	attr.colormap = XCreateColormap(cli->dsp, cli->root_id, cli->vi, false);
	attr.background_pixmap = 0;
	attr.border_pixel = 0;
	attr.override_redirect = true;
	attr.event_mask = StructureNotifyMask;

	cli->win_id = XCreateWindow(
		cli->dsp, cli->root_id,
		0, 0,
		DisplayWidth(cli->dsp, DefaultScreen(cli->dsp)),
		DisplayHeight(cli->dsp, DefaultScreen(cli->dsp)),
		0, CopyFromParent, InputOutput, cli->vi,
		CWOverrideRedirect | CWColormap | CWBackPixmap | CWBorderPixel | CWEventMask,
		&attr
	);

	if (!cli->win_id) {
		XCloseDisplay(cli->dsp);
		return (false);
	}

	cli->wm_delete_window = XInternAtom(cli->dsp, "WM_DELETE_WINDOW", false);
	cli->_net_wm_state = XInternAtom(cli->dsp, "_NET_WM_STATE", false);
	cli->_net_wm_state_above = XInternAtom(cli->dsp, "_NET_WM_STATE_ABOVE", false);

	XChangeProperty(cli->dsp, cli->win_id, cli->_net_wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *) &cli->_net_wm_state_above, 1);
	XSetWMProtocols(cli->dsp, cli->win_id, &cli->wm_delete_window, true);
	XSelectInput(cli->dsp, cli->win_id, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);
	
	XMapWindow(cli->dsp, cli->win_id);

	return (true);
}

bool	x_selectTerminate(struct s_select *cli) {
	XDestroyWindow(cli->dsp, cli->win_id);
	XCloseDisplay(cli->dsp);

	return (true);
}
