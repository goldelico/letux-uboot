#
# Ingenic tools makefile
#
# Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
# Author: Zoro <ykli@ingenic.cn>
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

ifndef CONFIG_SPL_BUILD
HOSTCPPFLAGS =	-include $(SRCTREE)/include/libfdt_env.h \
		-idirafter $(SRCTREE)/include \
		-idirafter $(OBJTREE)/include2 \
		-idirafter $(OBJTREE)/include \
	        -I $(SRCTREE)/lib/libfdt \
		-I $(SRCTREE)/tools \
		-DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE) \
		-DUSE_HOSTCC \
		-D__KERNEL_STRICT_NAMES
#HOSTCPPFLAGS = -I$(OBJTREE)include2 -I$(OBJTREE)/include -I$(SRCTREE)/include -DUSE_HOSTCC

DDR-$(CONFIG_DDR_PARAMS_CREATOR)	+= ddr_params_creator ddr_reg_values.h

ifdef CONFIG_MULT_DDR_PARAMS_CREATOR
DDR-$(CONFIG_MULT_DDR_PARAMS_CREATOR)	+= ddr_reg_values.h mult_ddr_params_header
DDR-$(CONFIG_DDR_TYPE_DDR3)		+= ddr_params_creator_ddr3 ddr_reg_values_ddr3.h
DDR-$(CONFIG_DDR_TYPE_DDR2)		+= ddr_params_creator_ddr2 ddr_reg_values_ddr2.h
DDR-$(CONFIG_DDR_TYPE_LPDDR)		+= ddr_params_creator_lpddr ddr_reg_values_lpddr.h
DDR-$(CONFIG_DDR_TYPE_LPDDR2)		+= ddr_params_creator_lpddr2 ddr_reg_values_lpddr2.h
DDR-$(CONFIG_DDR_TYPE_LPDDR3)		+= ddr_params_creator_lpddr3 ddr_reg_values_lpddr3.h

endif

MBR-$(CONFIG_MBR_CREATOR)		+= mbr_creator mbr.bin
GPT-$(CONFIG_GPT_CREATOR)		+= mbr-gpt.bin
SPI-$(CONFIG_SPI_SPL_CHECK)		+= spi_checksum
MMC-$(CONFIG_MMC_SPL_PARAMS)		+= mmc_params
SPI-$(CONFIG_SPL_SFC_SUPPORT)		+= sfc_timing_val.h
SPI-$(CONFIG_SPL_SFC_NAND)		+= sfc_nand_params.h
SPI-$(CONFIG_NOR_BUILTIN_PARAMS)	+= sfc_nor_builtin_params
SPI-$(CONFIG_NAND_BUILTIN_PARAMS)	+= sfc_nand_builtin_params
SPL-$(CONFIG_SPL_PARAMS_FIXER)		+= spl_params_fixer
SPL-$(CONFIG_SPL_PAD_TO_BLOCK)		+= spl_pad_to_block
AUDIO-$(CONFIG_AUDIO_CAL_DIV) += audio_div_values.h
#IMG2RLE-$(CONFIG_RLE_LCD_LOGO)          += img2rle
#BIN2ARRAY-$(CONFIG_RLE_LCD_LOGO)        += bin2array
LOGO-$(CONFIG_RLE_LCD_LOGO) = $(RLE_BOOT_LOGO_H)
ifndef CONFIG_BATTERYDET_LED
CHARGE_LOGO-$(CONFIG_CMD_BATTERYDET) = $(RLE_CHARGE_LOGO_H)
endif
RLE_BOOT_LOGO_H = $(OBJTREE)/include/rle_boot_logo.h
ifndef CONFIG_BATTERYDET_LED
RLE_CHARGE_LOGO_H = $(OBJTREE)/include/rle_charge_logo.h
endif

TOOLS	= $(DDR-y) $(MBR-y) $(GPT-y) $(SPI-y) $(MMC-y) $(SPL-y) $(LOGO-y) $(CHARGE_LOGO-y) $(AUDIO-y) #$(IMG2RLE-y) $(BIN2ARRAY-y)
TOOLS	:= $(addprefix $(obj),$(TOOLS))

ifeq ($(CONFIG_GPT_TAB_BUILT_IN),y)
TOOLS := $(TOOLS) gpt_partitions_to_c
endif

DDR_CREATOR_DIR := ddr_creator
ifeq ($(if $(CONFIG_X2000)$(CONFIG_X2000_V12)$(CONFIG_M300)$(CONFIG_X2100)$(CONFIG_X2500)$(CONFIG_X2600)$(CONFIG_AD100),y,n),y)
DDR_CREATOR_DIR := ddr_creator_x2000
else ifeq ($(if $(CONFIG_X1600),y,n),y)
DDR_CREATOR_DIR := ddr_creator_x1600
else ifeq ($(if $(CONFIG_A1),y,n),y)
DDR_CREATOR_DIR := ddr_creator_a1
else ifeq ($(if $(CONFIG_X2580),y,n),y)
DDR_CREATOR_DIR := ddr_creator_x2580
endif

ifdef CONFIG_MULT_DDR_PARAMS_CREATOR
DDR-TYPE-y = $(DDR_CREATOR_DIR)/mult_ddr_params_creator.c
DDR-TYPE-LPDDR3-$(CONFIG_DDR_TYPE_LPDDR3) = $(DDR_CREATOR_DIR)/lpddr3_params.c
DDR-TYPE-LPDDR2-$(CONFIG_DDR_TYPE_LPDDR2) = $(DDR_CREATOR_DIR)/lpddr2_params.c
DDR-TYPE-LPDDR-$(CONFIG_DDR_TYPE_LPDDR) = $(DDR_CREATOR_DIR)/lpddr_params.c
DDR-TYPE-DDR2-$(CONFIG_DDR_TYPE_DDR2) = $(DDR_CREATOR_DIR)/ddr2_params.c
DDR-TYPE-DDR3-$(CONFIG_DDR_TYPE_DDR3) = $(DDR_CREATOR_DIR)/ddr3_params.c

ALL_INCLUDE_PRIVATE := $(SRCTREE)/include/configs/$(BOARD).h $(DDR_CREATOR_DIR)/ddr_params_creator.h
all:	$(obj).depend $(TOOLS)

$(obj)ddr_params_creator_ddr3: $(DDR-TYPE-y) $(DDR-TYPE-DDR3-y)
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) $(CONFIG_DDR3_DFLAGS) -o  $@ $^
	$(HOSTSTRIP) $@

$(obj)ddr_params_creator_ddr2: $(DDR-TYPE-y) $(DDR-TYPE-DDR2-y)
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) $(CONFIG_DDR2_DFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)ddr_params_creator_lpddr: $(DDR-TYPE-y) $(DDR-TYPE-LPDDR-y)
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) $(CONFIG_LPDDR2_DFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)ddr_params_creator_lpddr2: $(DDR-TYPE-y) $(DDR-TYPE-LPDDR2-y)
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) $(CONFIG_LPDDR2_DFLAGS) -o $@ $^
	$(HOSTSTRIP) $@
$(obj)ddr_params_creator_lpddr3: $(DDR-TYPE-y) $(DDR-TYPE-LPDDR3-y)
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) $(CONFIG_LPDDR3_DFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)ddr_reg_values_lpddr3.h:	$(obj)ddr_params_creator_lpddr3
	$(OBJTREE)/tools/ingenic-tools/ddr_params_creator_lpddr3  $(OBJTREE)/include/generated/$(notdir $@)
$(obj)ddr_reg_values_lpddr2.h:	$(obj)ddr_params_creator_lpddr2
	$(OBJTREE)/tools/ingenic-tools/ddr_params_creator_lpddr2  $(OBJTREE)/include/generated/$(notdir $@)
$(obj)ddr_reg_values_lpddr.h:	$(obj)ddr_params_creator_lpddr
	$(OBJTREE)/tools/ingenic-tools/ddr_params_creator_lpddr  $(OBJTREE)/include/generated/$(notdir $@)
$(obj)ddr_reg_values_ddr2.h:	$(obj)ddr_params_creator_ddr2
	$(OBJTREE)/tools/ingenic-tools/ddr_params_creator_ddr2  $(OBJTREE)/include/generated/$(notdir $@)
$(obj)ddr_reg_values_ddr3.h:	$(obj)ddr_params_creator_ddr3
	$(OBJTREE)/tools/ingenic-tools/ddr_params_creator_ddr3  $(OBJTREE)/include/generated/$(notdir $@)

DDR-H-$(CONFIG_DDR_TYPE_DDR3)		= generated/ddr_reg_values_ddr3.h
DDR-H-$(CONFIG_DDR_TYPE_DDR2)		+= generated/ddr_reg_values_ddr2.h
DDR-H-$(CONFIG_DDR_TYPE_LPDDR)		+= generated/ddr_reg_values_lpddr.h
DDR-H-$(CONFIG_DDR_TYPE_LPDDR2)		+= generated/ddr_reg_values_lpddr2.h
DDR-H-$(CONFIG_DDR_TYPE_LPDDR3)		+= generated/ddr_reg_values_lpddr3.h

$(obj)mult_ddr_params_header:$(DDR_CREATOR_DIR)/mult_ddr_params_header.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^

$(obj)ddr_reg_values.h: $(obj)mult_ddr_params_header
	$(OBJTREE)/tools/ingenic-tools/mult_ddr_params_header  $(OBJTREE)/include/generated/$(notdir $@) $(DDR-H-y)
else

DDR-TYPE-y = $(DDR_CREATOR_DIR)/ddr_params_creator.o
DDR-TYPE-$(CONFIG_DDR_TYPE_LPDDR3) += $(DDR_CREATOR_DIR)/lpddr3_params.o
DDR-TYPE-$(CONFIG_DDR_TYPE_LPDDR2) += $(DDR_CREATOR_DIR)/lpddr2_params.o
DDR-TYPE-$(CONFIG_DDR_TYPE_LPDDR) += $(DDR_CREATOR_DIR)/lpddr_params.o
DDR-TYPE-$(CONFIG_DDR_TYPE_DDR2) += $(DDR_CREATOR_DIR)/ddr2_params.o
DDR-TYPE-$(CONFIG_DDR_TYPE_DDR3) += $(DDR_CREATOR_DIR)/ddr3_params.o

ifeq ($(DDR_CREATOR_DIR), ddr_creator_x2000)
DDR-TYPE-y	+= $(DDR_CREATOR_DIR)/supported_ddr_chips.o
else ifeq ($(DDR_CREATOR_DIR), ddr_creator_x1600)
DDR-TYPE-y	+= $(DDR_CREATOR_DIR)/supported_ddr_chips.o
else ifeq ($(DDR_CREATOR_DIR), ddr_creator_x2580)
DDR-TYPE-y  += $(DDR_CREATOR_DIR)/supported_ddr_chips.o
endif

define PRIVATE_BUILD
$(1):$(2) $(3)
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -c -o $(1) $(2)
endef

ALL_INCLUDE_PRIVATE := $(SRCTREE)/include/configs/$(BOARD).h $(DDR_CREATOR_DIR)/ddr_params_creator.h
all:	$(obj).depend $(TOOLS)

ddr-type-objs-y := $(addprefix $(obj), $(DDR-TYPE-y))
$(obj)ddr_params_creator: $(ddr-type-objs-y)
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@


$(shell [ -d $(obj)$(DDR_CREATOR_DIR) ] || mkdir -p $(obj)$(DDR_CREATOR_DIR))

$(foreach f,$(DDR-TYPE-y), \
	$(eval $(call PRIVATE_BUILD,$(obj)$(f),$(f:.o=.c),$(ALL_INCLUDE_PRIVATE))))

$(obj)ddr_reg_values.h:   $(obj)ddr_params_creator
	$(OBJTREE)/tools/ingenic-tools/ddr_params_creator > $(OBJTREE)/include/generated/$(notdir $@)

endif

$(obj)audio_div_values.h: $(obj)audio_params_creator.py
	$(OBJTREE)/tools/ingenic-tools/audio_params_creator.py $(CONFIG_AUDIO_APLL) $(CONFIG_AUDIO_MPLL) > $(OBJTREE)/include/generated/$(notdir $@)

$(obj)sfc_timing_val.h: $(obj)sfc_timing_params
	        $(OBJTREE)/tools/ingenic-tools/sfc_timing_params > $(OBJTREE)/include/generated/$(notdir $@)

$(obj)sfc_nand_params.h: $(obj)sfc_nand_params
	$(OBJTREE)/tools/ingenic-tools/sfc_nand_params > $(OBJTREE)/include/generated/$(notdir $@)

$(obj)mbr_creator:	mbr_creator.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)mbr.bin:	$(obj)mbr_creator
	$(OBJTREE)/tools/ingenic-tools/mbr_creator	\
		p0off=$(CONFIG_MBR_P0_OFF),p0end=$(CONFIG_MBR_P0_END),p0type=$(CONFIG_MBR_P0_TYPE) \
		p1off=$(CONFIG_MBR_P1_OFF),p1end=$(CONFIG_MBR_P1_END),p1type=$(CONFIG_MBR_P1_TYPE) \
		p2off=$(CONFIG_MBR_P2_OFF),p2end=$(CONFIG_MBR_P2_END),p2type=$(CONFIG_MBR_P2_TYPE) \
		p3off=$(CONFIG_MBR_P3_OFF),p3end=$(CONFIG_MBR_P3_END),p3type=$(CONFIG_MBR_P3_TYPE) \
		-o $@ > /dev/zero

$(obj)gpt_creator:	$(SRCTREE)/lib/crc32.c gpt_creator.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

ifdef CONFIG_GPT_TABLE_PATH
$(obj)mbr-gpt.bin:	$(obj)gpt_creator
ifdef CONFIG_JZSD_OTA_VERSION20
	$(OBJTREE)/tools/ingenic-tools/gpt_creator		\
	$(CONFIG_GPT_TABLE_PATH)/partitions_mmc_ota.tab mbr-of-gpt.bin gpt.bin
else
	$(OBJTREE)/tools/ingenic-tools/gpt_creator		\
	$(CONFIG_GPT_TABLE_PATH)/partitions.tab mbr-of-gpt.bin gpt.bin
endif
	cat mbr-of-gpt.bin gpt.bin > $@
	dd if=/dev/zero of=file.bin bs=512 count=33
	cat $@ file.bin > file2.bin
	dd if=file2.bin of=$@ bs=512 count=34
	rm file* -rf
else
$(obj)mbr-gpt.bin:	$(obj)mbr.bin
	dd if=/dev/zero of=gpt.bin bs=512 count=33
	cat mbr.bin gpt.bin > $@
endif

$(obj)spi_checksum:	spi_checksum.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)mmc_params:	mmc_params.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

fixer_var:=
ifeq ($(if $(CONFIG_X2000_V12)$(CONFIG_M300)$(CONFIG_X2100),y,n),y)
	fixer_var:=spl_params_fixer_x2000_v12.c
else ifeq ($(if $(CONFIG_X2500),y,n),y)
	fixer_var:=spl_params_fixer_x2500.c
else ifeq ($(if $(CONFIG_X1600),y,n),y)
	fixer_var:=spl_params_fixer_x1600.c
else ifeq ($(if $(CONFIG_X2580),y,n),y)
	fixer_var:=spl_params_fixer_x2580.c
else ifeq ($(if $(CONFIG_X2600),y,n),y)
	fixer_var:=spl_params_fixer_x2600.c
else ifeq ($(if $(CONFIG_AD100),y,n),y)
	fixer_var:=spl_params_fixer_ad100.c
else
	fixer_var:=spl_params_fixer.c
endif

$(obj)spl_params_fixer:	$(fixer_var)
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@


spl_source_list:=
ifeq ($(if $(CONFIG_SPL_PARAMS_FIXER),y,n),y)
	spl_source_list += $(fixer_var)
endif

ifeq ($(if $(CONFIG_SPI_SPL_CHECK),y,n),y)
	spl_source_list += spi_checksum.c
endif

ifeq ($(if $(CONFIG_MMC_SPL_PARAMS),y,n),y)
	spl_source_list += mmc_params.c
endif

ifeq ($(if $(CONFIG_SPL_PAD_TO_BLOCK),y,n),y)
	spl_source_list +=  spl_pad_to_block.c
endif

ifeq ($(if $(CONFIG_NOR_BUILTIN_PARAMS),y,n),y)
	spl_source_list += sfc_nor_builtin_params.c sfc_builtin_params/nor_device.c
endif

ifeq ($(if $(CONFIG_NAND_BUILTIN_PARAMS),y,n),y)
	spl_source_list += sfc_nand_builtin_params.c sfc_builtin_params/nand_device.c
endif

spl_tools_source:
	echo "$(spl_source_list)" > .spl_source_tools_depend



$(obj)sfc_timing_params: sfc_timing_params.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)sfc_nor_builtin_params: sfc_nor_builtin_params.c sfc_builtin_params/nor_device.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)sfc_nand_builtin_params: sfc_nand_builtin_params.c sfc_builtin_params/nand_device.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)spl_pad_to_block:	spl_pad_to_block.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(obj)sfc_nand_params: nand_device/
	make -C $<
	cp -f nand_device/sfc_nand_params $@

gpt_partitions_to_c:
ifdef CONFIG_JZSD_OTA_VERSION20
	$(SRCTREE)/tools/ingenic-tools/gpt_tab_to_c.sh \
	 $(CONFIG_GPT_TABLE_PATH)/partitions_mmc_ota.tab > $(SRCTREE)/common/spl/spl_gpt_tab_data.h
else
	$(SRCTREE)/tools/ingenic-tools/gpt_tab_to_c.sh \
	 $(CONFIG_GPT_TABLE_PATH)/partitions.tab > $(SRCTREE)/common/spl/spl_gpt_tab_data.h
endif

.PHONY:gpt_partitions_to_c

clean:
	rm -f $(TOOLS) $(obj).depend  \
		$(TOPDIR)/tools/logos/*.rle \
		$(TOPDIR)/tools/charge_logo/*.rle \
		$(TOPDIR)/board/$(BOARDDIR)/logo/*.rle \
		$(TOPDIR)/board/$(BOARDDIR)/charge_logo/*.rle \
		$(obj)*.o \
		$(obj)ddr_params_creator \
		$(obj)sfc_nand_params \
		$(obj)sfc_timing_params \
		$(obj)sfc_nor_builtin_params \
		$(obj)sfc_nand_builtin_params \
		$(TOPDIR)/tools/ingenic-tools/.spl_source_tools_depend

distclean:	clean

else
LIB	= $(obj)lib_ddr_params_creator.o

COBJS	:= ddr_params_creator.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS))
endif

#########################################################################

ifndef CONFIG_SPL_BUILD
include $(SRCTREE)/tools/ingenic-tools/logo.mk
endif
# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend


#########################################################################
