#
# Copyright (C) 2011 Xiangfu Liu <xiangfu@openmobilefree.net>
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).o

START	= start.o
SOBJS-y	=
has_jz_serial = y
ifdef CONFIG_GPIO_SPI_TO_UART
has_jz_serial = n
endif
ifdef CONFIG_GPIO_SPI_TO_UART2
has_jz_serial = n
endif

COBJS-$(CONFIG_GPIO_SPI_TO_UART) = gpio_spi_to_uart.o
COBJS-$(CONFIG_GPIO_SPI_TO_UART2) = gpio_spi_to_uart2.o
COBJS-$(has_jz_serial)	= jz_serial.o

ifdef CONFIG_SPL_BUILD
COBJS-y	+= spl_cpu.o
ifdef CONFIG_X1600
COBJS-y += ./x1600/timer.o
else
COBJS-y += spl_timer.o
endif
else
COBJS-y	+= cpu.o
ifdef CONFIG_X1600
COBJS-y	+= ./x1600/timer.o
else
COBJS-y	+= timer.o
endif
endif
COBJS-$(CONFIG_XBURST_TRAPS) += traps.o
COBJS-$(CONFIG_SPL_OS_BOOT) += os_boot.o

ifdef	CONFIG_SPL_BUILD
ifdef	CONFIG_SPL_NO_CPU_SUPPORT_CODE
START	:=
endif
endif

SRCS	:= $(START:.o=.S) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
