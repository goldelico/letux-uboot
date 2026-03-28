include $(TOPDIR)/config.mk

LIB	:= $(obj)libscboot.o

COBJS-$(CONFIG_JZ_SCBOOT) += scboot.o md5.o flash.o aes.o spi_checksum.o test_common.o
COBJS-$(CONFIG_JZ_SBOOT_TEST) += cmd_sctest.o test_scboot.o test_nku.o test_otp.o
COBJS-$(CONFIG_JZ_CKEYAES) += ckey_aes.o
COBJS-$(CONFIG_BURNER) += secure.o rsa.o otp.o get_rn.o

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
