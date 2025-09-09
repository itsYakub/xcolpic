#if !defined _xcolpic_h_
# define _xcolpic_h_
# include <stdint.h>
# include <stdbool.h>
# include <X11/X.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/Xatom.h>

# define _min(a, b) (a < b ? a : b)
# define _abs(a) ((int32_t) (a) > 0 ? (uint32_t) (a) : (uint32_t) ((int32_t) (a) * -1))

# define XCOLPIC_FMT_RGB	1
# define XCOLPIC_FMT_RGB10	2
# define XCOLPIC_FMT_RGBA	3
# define XCOLPIC_FMT_RGBA10	4
# define XCOLPIC_FMT_HEX	5

struct s_opt {
	uint8_t	format;
	bool	coord;
};

typedef struct s_opt	t_opt;

bool	x_colpicParseOpt(t_opt *, int, char **);
bool	x_colpicSelectData(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t **, uint32_t *);

#endif /* _xcolpic_h_ */
