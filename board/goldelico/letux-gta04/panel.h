#ifndef _PANEL_H
#define _PANEL_H

#include <video_fb.h>
// #include "TD028TTEC1.h"
// #include "backlight.h"

enum panel_state {
	PANEL_STATE_DEEP_STANDBY,
	PANEL_STATE_SLEEP,
	PANEL_STATE_NORMAL,
};

int panel_display_onoff(int on);
int panel_enter_state(enum panel_state new_state);
const char *panel_state(void);
int board_video_init(GraphicDevice *pGD);

extern int displayColumns;
extern int displayLines;

#endif
