
include $(TOPDIR)/config.mk

LIB	:= $(obj)libprintertest.o

COBJS-y += cmd_printer_sy.o
COBJS-y += adc.o
COBJS-y += ssi.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))


#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

########################################################################
