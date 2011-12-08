#ifndef _TSC2007_H
#define _TSC2007_H

int tsc2007_init(void);
int read_adc(int adcnum);
void print_adc(void);
int pendown(int *x, int *y);

#endif
