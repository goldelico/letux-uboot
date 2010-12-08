// in the long run we should have our own board file

/*
 * IEN  - Input Enable
 * IDIS - Input Disable (i.e. output enable)
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * M4	- Mode 4 (GPIO)
 */

#define MUX_BEAGLE_GTA04() \
MUX_VAL(CP(ETK_CLK),		(IDIS | PTU | EN  | M4)) /*GPIO_12 - Display serial clock*/\
MUX_VAL(CP(ETK_CTL),		(IDIS | PTU | EN  | M4)) /*GPIO_13 - IrDA FIR-SEL*/\
MUX_VAL(CP(ETK_D4),			(IDIS | PTU | DIS | M4)) /*GPIO_18 - Display DIN*/\
MUX_VAL(CP(ETK_D5),			(IDIS | PTU | DIS | M4)) /*GPIO_19 - Display select*/\
MUX_VAL(CP(ETK_D6),			(IDIS | PTU | DIS | M4)) /*GPIO_20 - Display DOUT*/\
MUX_VAL(CP(ETK_D7),			(IEN  | PTU | EN  | M4)) /*GPIO_21 - RS232 enable*/\
MUX_VAL(CP(GPMC_NCS6),		(IDIS | PTU | DIS | M4)) /*GPIO_57/GPT_11 - Backlight enable*/\
MUX_VAL(CP(GPMC_WAIT3),		(IDIS | PTU | DIS | M4)) /*GPIO_65 - AUX IN/OUT*/\
MUX_VAL(CP(CSI2_DX0),		(IEN  | PTU | EN  | M4)) /*GPIO_112 - VIDEOON FIXME: this is Input Only!*/\
MUX_VAL(CP(CSI2_DY0),		(IEN  | PTU | DIS | M4)) /*GPIO_113 - Barometer INT*/\
MUX_VAL(CP(CSI2_DX1),		(IEN  | PTU | DIS | M4)) /*GPIO_114 - Accel1 INT*/\
MUX_VAL(CP(CSI2_DY1),		(IEN  | PTU | DIS | M4)) /*GPIO_115 - Accel2 INT*/\
MUX_VAL(CP(MMC2_CLK),		(IEN  | PTU | EN  | M0)) /*GPIO_130 -> MMC2_CLK*/\
MUX_VAL(CP(MMC2_CMD),		(IEN  | PTU | EN  | M0)) /*GPIO_131 -> MMC2_CMD*/\
MUX_VAL(CP(MMC2_DAT0),		(IEN  | PTU | EN  | M0)) /*GPIO_132 -> MMC2_DAT0*/\
MUX_VAL(CP(MMC2_DAT1),		(IEN  | PTU | EN  | M0)) /*GPIO_133 -> MMC2_DAT1*/\
MUX_VAL(CP(MMC2_DAT2),		(IEN  | PTU | EN  | M0)) /*GPIO_134 -> MMC2_DAT2*/\
MUX_VAL(CP(MMC2_DAT3),		(IEN  | PTU | EN  | M0)) /*GPIO_135 -> MMC2_DAT3*/\
MUX_VAL(CP(MMC2_DAT4),		(IDIS | PTU | DIS | M1)) /*GPIO_136 - MMC2_DIR_DAT0 */\
MUX_VAL(CP(MMC2_DAT5),		(IDIS | PTU | DIS | M1)) /*GPIO_137 - MMC2_DIR_DAT1 */\
MUX_VAL(CP(MMC2_DAT6),		(IDIS | PTU | DIS | M1)) /*GPIO_138 - MMC2_DIR_CMD */\
MUX_VAL(CP(MMC2_DAT7),		(IEN  | PTU | DIS | M1)) /*GPIO_139 - MMC2_DIR_CLKIN */\
MUX_VAL(CP(UART2_CTS),		(IEN  | PTU | DIS | M4)) /*GPIO_144 - ext Ant */\
MUX_VAL(CP(UART2_RTS),		(IDIS | PTD | DIS | M4)) /*GPIO_145 - GPS ON(0)/OFF(1)*/\
MUX_VAL(CP(UART2_TX),		(IDIS | PTU | DIS | M0)) /*GPIO_146 - GPS_TX */\
MUX_VAL(CP(UART2_RX),		(IEN  | PTU | DIS | M0)) /*GPIO_147 - GPS_RX */\
MUX_VAL(CP(MCBSP1_CLKR),	(IDIS | PTD | DIS | M0)) /*GPIO_156 - FM TRX*/\
MUX_VAL(CP(MCBSP1_FSR),		(IEN  | PTU | EN  | M0)) /*GPIO_157 -  */\
MUX_VAL(CP(MCBSP1_DX),		(IDIS | PTD | EN  | M0)) /*GPIO_158 -  */\
MUX_VAL(CP(MCBSP1_DR),		(IEN  | PTU | DIS | M0)) /*GPIO_159 -  */\
MUX_VAL(CP(MCBSP1_FSX),		(IDIS | PTU | EN  | M0)) /*GPIO_161 -  */\
MUX_VAL(CP(MCBSP1_CLKX),	(IDIS | PTD | EN  | M0)) /*GPIO_162 -  */\
MUX_VAL(CP(MCBSP_CLKS),		(IEN  | PTU | DIS | M4)) /*GPIO_160 - PENIRQ*/\
MUX_VAL(CP(MCSPI1_CLK),		(IEN  | PTU | DIS | M4)) /*GPIO_171 - Version sense*/\
MUX_VAL(CP(MCSPI1_SIMO),	(IEN  | PTU | DIS | M4)) /*GPIO_172 - Version sense*/\
MUX_VAL(CP(MCSPI1_SOMI),	(IEN  | PTU | DIS | M4)) /*GPIO_173 - Version sense*/\
MUX_VAL(CP(MCSPI1_CS0),		(IEN  | PTD | EN  | M4)) /*GPIO_174 - USB-PHY-RESET*/\
MUX_VAL(CP(MCSPI1_CS1),		(IEN  | PTD | EN  | M4)) /*GPIO_175/MMC3CMD - unused*/\
MUX_VAL(CP(MCSPI1_CS2),		(IEN  | PTD | EN  | M4)) /*GPIO_176/MMC3CLK - unused*/\


