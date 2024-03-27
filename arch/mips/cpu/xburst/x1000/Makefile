#
# Ingenic JZ4775 makefile
#
# Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
# Author: Zoro <ykli@ingenic.cn>
# Based on: arch/mips/cpu/xburst/jz4780/Makefile
#           Written by Paul Burton <paul.burton@imgtec.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

SOBJS-y	=
COBJS-y	= soc.o

COBJS-$(CONFIG_BURNER) += dym_pll.o

ifndef CONFIG_BURNER
COBJS-y += pllsetting.o
endif

COBJS-y += clk.o
COBJS-y += ddr_set_dll.o
COBJS-y += ../ddr_dwc.o
COBJS-$(CONFIG_DDR_TEST) += ../ddr_test.o
COBJS-$(CONFIG_BURNER) += ../param_manage.o
COBJS-$(CONFIG_CHECK_SOCID) += check_socid.o

SRCS	:= $(START:.o=.S) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

HOSTCPPFLAGS =	-include $(SRCTREE)/include/libfdt_env.h \
		-idirafter $(SRCTREE)/include \
		-idirafter $(OBJTREE)/include2 \
		-idirafter $(OBJTREE)/include \
	        -I $(SRCTREE)/lib/libfdt \
		-I $(SRCTREE)/tools \
		-DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE) \
		-DUSE_HOSTCC \
		-D__KERNEL_STRICT_NAMES

$(obj).depend.pllsetting:$(OBJTREE)/include/generated/pll_reg_values.h
ifndef CONFIG_BURNER
$(SPLTREE)/$(dir)/../ddr_dwc.o:$(OBJTREE)/include/generated/ddr_reg_values.h
endif


$(OBJTREE)/include/generated/pll_reg_values.h:$(OBJTREE)/$(dir)/pll_params_creator.exe
	$< > $@

$(OBJTREE)/$(dir)/pll_params_creator.exe:$(SRCTREE)/$(dir)/pll_params_creator.c | $(SRCTREE)/include/configs/$(BOARD).h
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) $(HOSTCPPFLAGS) -o $@ $^

$(obj).depend.clk:$(OBJTREE)/include/generated/clk_reg_values.h


$(OBJTREE)/include/generated/clk_reg_values.h:$(OBJTREE)/$(dir)/clk_params_creator.exe
	$< > $@

$(OBJTREE)/$(dir)/clk_params_creator.exe:$(SRCTREE)/$(dir)/clk_params_creator.c | $(SRCTREE)/include/configs/$(BOARD).h
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) $(HOSTCPPFLAGS) -o $@ $^

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
