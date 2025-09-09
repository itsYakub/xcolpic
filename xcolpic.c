#include "./xcolpic.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <string.h>
#include <getopt.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>


static struct option	g_longopt[] = {
	{ "format",	 1, 0, 'f'},
	{ "coords",  0, 0, 'c'},
	{ "help",    0, 0, 'h'},
	{ "version", 0, 0, 'v'},
	{ 0,         0, 0,  0 }
};

bool	x_colpicParseOpt(t_opt *opt, int ac, char **av) {
	int32_t	i;
	int8_t	c;

	while ((c = getopt_long(ac, av, "f:chv", g_longopt, &i)) != -1) {
		switch (c) {
			case ('f'): {
				int32_t	f;

				if (!optarg) {
					exit (1);
				}

				f = 0;
				if (!strcmp(optarg, "rgb")) { f = XCOLPIC_FMT_RGB; }
				else if (!strcmp(optarg, "rgb10")) { f = XCOLPIC_FMT_RGB10; }
				else if (!strcmp(optarg, "rgba")) { f = XCOLPIC_FMT_RGBA; }
				else if (!strcmp(optarg, "rgba10")) { f = XCOLPIC_FMT_RGBA10; }
				else if (!strcmp(optarg, "hex")) { f = XCOLPIC_FMT_HEX; }
				else {
					printf("%s: unregocnized argument '%s'\n", av[0], optarg);
					exit (1);
				}

				opt->format = f;
			} break;
			
			case ('c'): { opt->coord = true; } break;

			case ('h'): {
				printf("Usage: xcolpic [ OPTIONS ] ...\n");
				printf("Access an X session's pixel data from the display.\n\n");
				printf("  -h, --help                                  display this help message;\n");
				printf("  -v, --version                               display the program's current version;\n");
				printf("  -f, --format [ rgb,rgb10,rgba,rgba10,hex ]  sets the output format (default: hex);\n");
				printf("  -c, --coords                                display the selected region;\n");
				exit (0);
			} break;

			case ('v'): { printf("1.0\n"), exit (0); } break;
			default: { exit (1); } break;
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
