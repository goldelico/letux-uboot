#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

ifdef CONFIG_BURNER

LIB	:= $(obj)libcloner_moudle_mg.o

COBJS-$(CONFIG_JZ_VERDOR_BURN_EXTPOL) += extend_policy_manager.o
SUBOBJS-$(CONFIG_JZ_VERDOR_BURN_EXTPOL) += expy/libexpy.o

ifneq ($(findstring y, $(CONFIG_MTD_SFCNOR)$(CONFIG_MTD_SFCNAND)$(CONFIG_MTD_SPINAND)$(CONFIG_JZ_SPI)),)
COBJS-y += cloner_moudle_sfc.o
endif
ifneq ($(findstring y, $(CONFIG_JZ_SPINAND_SN)$(CONFIG_JZ_SPINAND_MAC)),)
COBJS-y += cloner_spinand_firmware.o
endif
ifneq ($(findstring y, $(CONFIG_JZ_NAND_MGR)$(CONFIG_MTD_NAND_JZ)),)
COBJS-y += cloner_moudle_nand.o
endif
ifneq ($(findstring y, $(CONFIG_JZ_MMC)$(CONFIG_JZ_SDHCI)),)
COBJS-y += cloner_moudle_mmc.o
endif
COBJS-$(CONFIG_JZ_EFUSE) += cloner_moudle_efuse.o


COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SUBOBJS := $(addprefix $(obj),$(SUBOBJS-y))
OBJS	+= $(SUBOBJS)

all:	$(LIB)

.PHONY : $(SUBOBJS)
$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(SUBOBJS):
	$(MAKE) -C $(dir $(SUBOBJS))
#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
endif
