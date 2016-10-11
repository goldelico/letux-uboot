obj-y := beagle.o

ifndef CONFIG_SPL_BUILD
obj-y += nopanel.o ../letux-gta04/status.o ../letux-gta04/shutdown.o ../letux-gta04/commands.o ../letux-gta04/ulpi-phy.o ../letux-gta04/twl4030-additions.o
endif

ifdef MORE
obj-y += ../letux-gta04/backlight.o ../letux-gta04/tsc2007.o ../letux-gta04/dssfb.o ../letux-gta04/gps.o ../letux-gta04/systest.o
endif