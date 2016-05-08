#ifndef _NOPANEL_H
#define _NOPANEL_H

enum panel_state {
	PANEL_STATE_DEEP_STANDBY,
	PANEL_STATE_SLEEP,
	PANEL_STATE_NORMAL,
};

int panel_check(void);
int panel_reg_init(void);

#endif
