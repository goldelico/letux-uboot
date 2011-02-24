// FIXME: Panda has a different initialization scheme!
// we have to extend const struct pad_conf_entry core_padconf_array[]

#include <asm/io.h>
#include <asm/arch/mux_omap4.h>

const struct pad_conf_entry hybrid_padconf_array[] = {
	// add entries
};
	
#if 0

#define MUX_BEAGLE_HYBRID() \
MUX_VAL(CP(MMC2_CLK),		(IEN  | PTU | EN  | M0)) /*GPIO_130 -> MMC2_CLK*/\
MUX_VAL(CP(MMC2_CMD),		(IEN  | PTU | EN  | M0)) /*GPIO_131 -> MMC2_CMD*/\
MUX_VAL(CP(MMC2_DAT0),		(IEN  | PTU | EN  | M0)) /*GPIO_132 -> MMC2_DAT0*/\
MUX_VAL(CP(MMC2_DAT1),		(IEN  | PTU | EN  | M0)) /*GPIO_133 -> MMC2_DAT1*/\
MUX_VAL(CP(MMC2_DAT2),		(IEN  | PTU | EN  | M0)) /*GPIO_134 -> MMC2_DAT2*/\
MUX_VAL(CP(MMC2_DAT3),		(IEN  | PTU | EN  | M0)) /*GPIO_135 -> MMC2_DAT3*/\
MUX_VAL(CP(MMC2_DAT4),		(IEN  | PTU | EN  | M4)) /*GPIO_136 - AUX */\
MUX_VAL(CP(MMC2_DAT5),		(IEN  | PTU | EN  | M4)) /*GPIO_137 - POWER */\
MUX_VAL(CP(MMC2_DAT6),		(IEN  | PTU | DIS  | M4)) /*GPIO_138 - EXT-ANT */\
MUX_VAL(CP(MMC2_DAT7),		(IEN  | PTU | EN  | M4)) /*GPIO_139 - RS232 EXT */\
MUX_VAL(CP(MCBSP1_CLKR),	(IDIS | PTD | DIS | M4)) /*GPIO_156 - GPS ON(0)/OFF(1)*/\
MUX_VAL(CP(MCBSP1_FSR),		(IEN  | PTU | EN  | M4)) /*GPIO_157 - PENIRQ */\
MUX_VAL(CP(MCBSP1_DX),		(IDIS | PTD | EN  | M4)) /*GPIO_158 - DOUT */\
MUX_VAL(CP(MCBSP1_DR),		(IEN  | PTU | DIS | M4)) /*GPIO_159 - DIN - pulled up */\
MUX_VAL(CP(MCBSP_CLKS),		(IEN  | PTU | DIS | M0)) /*McBSP_CLKS*/\
MUX_VAL(CP(MCBSP1_FSX),		(IDIS | PTU | EN  | M4)) /*GPIO_161 - CS */\
MUX_VAL(CP(MCBSP1_CLKX),	(IDIS | PTD | EN  | M4)) /*GPIO_162 - SCL */

#endif
