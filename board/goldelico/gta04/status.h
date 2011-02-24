#ifndef _STATUS_H
#define _STATUS_H

int status_init (void);

void status_set_status (int value);
int status_get_buttons (void);

int status_set_flash (int mode);	// 0: off, 1: torch, 2: flash
int status_set_vibra (int mode);	// 0: off, 1: left, 2: right

#endif
