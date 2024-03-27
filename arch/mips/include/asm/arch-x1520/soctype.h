#ifndef __X1520_TYPE_H__
#define __X1520_TYPE_H__

enum X1520_soc_type {
	X1520L,
	X1520N,
	X1520X,
	X1520M,
	X1520Z,
	X1520_TYPE_END,
};

char *X1520_soc_type_str[X1520_TYPE_END] = {
	"X1520L",
	"X1520N",
	"X1520X",
	"X1520M",
	"X1520Z",
};

int get_X1520_type(void);

#endif
