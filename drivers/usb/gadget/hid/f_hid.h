#ifndef __F_HID_H__
#define __F_HID_H__

#ifdef DEBUG
#define PRINT_DEBUG(frame, argv...) fprintf(stderr, "%s %s %d " frame, __FILE__, __func__, __LINE__, ##argv)
#else
#define PRINT_DEBUG(frame, argv...)
#endif

enum hid_type {
	KEYBOARD,
	MOUSE,
	HID_TYPE_MAX,
};

#endif
