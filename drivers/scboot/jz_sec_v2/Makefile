include $(TOPDIR)/config.mk

LIB	:= $(obj)libscboot.o

COBJS-$(CONFIG_JZ_SCBOOT) += aes.o spi_checksum.o
COBJS-$(CONFIG_JZ_SECURE_SUPPORT) += scboot.o
COBJS-$(CONFIG_JZ_CKEYAES) += ckey_aes.o flash.o
COBJS-$(CONFIG_BURNER) += secure.o otp.o
# COBJS-y += cmd_burn_key.o

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

#########################################################################
