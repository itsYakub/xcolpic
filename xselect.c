#include "./xselect.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

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
