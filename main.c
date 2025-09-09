#include "./xselect.h"
#include "./xcolpic.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
		for (int32_t j = i - 1; i > 0 && j >= 0; j--) {
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
					"rgb(%1.3f, %1.3f, %1.3f)\n",
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
					"rgba(%1.3f, %1.3f, %1.3f, %1.3f)\n",
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
	if (opt.coord) {
		printf("%dx%d-%dx%d\n", x0, y0, x1, y1);
	}

	if (d_int) { free(d_int), d_int = 0; }
	return (0);
}
