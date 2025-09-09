#if !defined _xselect_h_
# define _xselect_h_
# include <stdint.h>
# include <stdbool.h>
# include <X11/X.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/Xatom.h>

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

#endif /* _xselect_h_ */
