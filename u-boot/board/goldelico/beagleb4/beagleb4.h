// all pins on BB expansion connector

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */
// pin GPIO_### / other function - used by expander

#define MUX_BEAGLE_EXPANDER() \
\
MUX_VAL(CP(MMC2_CLK),		(IDIS | PTD | EN  | M4)) /* 21 GPIO_130 / MCSPI3-CLK	- unused*/\
MUX_VAL(CP(MMC2_CMD),		(IDIS | PTD | EN  | M4)) /* 19 GPIO_131 / MCSPI3-SIMO	- unused*/\
MUX_VAL(CP(MMC2_DAT0),		(IDIS | PTU | EN  | M4)) /* 17 GPIO_132 / MCSPI3-SOMI	- unused*/\
MUX_VAL(CP(MMC2_DAT1),		(IDIS | PTU | EN  | M4)) /* 15 GPIO_133					- unused*/\
MUX_VAL(CP(MMC2_DAT2),		(IDIS | PTU | EN  | M4)) /* 13 GPIO_134 / MCSPI3-CS1	- unused*/\
MUX_VAL(CP(MMC2_DAT3),		(IDIS | PTU | EN  | M4)) /* 11 GPIO_135 / MCSPI3-CS0	- unused*/\
MUX_VAL(CP(MMC2_DAT4),		(IEN  | PTD | EN  | M4)) /*  9 GPIO_136					- AUX button */\
MUX_VAL(CP(MMC2_DAT5),		(IEN  | PTU | EN  | M4)) /*  7 GPIO_137					- POWER button */\
MUX_VAL(CP(MMC2_DAT6),		(IEN  | PTD | EN  | M4)) /*  5 GPIO_138             	- KEYIRQ (TRF IRQ/GPS Time Mark)*/\
MUX_VAL(CP(MMC2_DAT7),		(IDIS | PTU | EN  | M4)) /*  3 GPIO_139					- UART3-EXT*/\
\
MUX_VAL(CP(UART2_CTS),		(IEN  | PTD | DIS | M4)) /*  4 GPIO_144 / UART2-CTS		- ext. Antenna detect; don't use pullup/down!*/\
MUX_VAL(CP(UART2_TX),		(IDIS | PTU | EN  | M0)) /*  6 GPIO_146 / UART2-TX		- GPS RX*/\
MUX_VAL(CP(MCBSP3_FSX),		(IEN  | PTU | EN  | M1)) /*  8 GPIO_143 / UART2-RX		- GPS TX*/\
MUX_VAL(CP(UART2_RTS),		(IDIS | PTU | EN  | M4)) /* 10 GPIO_145 / UART2-RTS / GPT10 - backlight */\
MUX_VAL(CP(UART2_RX),		(IDIS | PTD | DIS | M4)) /* NA GPIO_147                 - don't switch to UART mode!*/\
MUX_VAL(CP(MCBSP1_DX),		(IDIS | PTD | EN  | M4)) /* 12 GPIO_158 / McBSP1-DX		- LVDS Display shutdown */\
MUX_VAL(CP(MCBSP1_CLKX),	(IEN  | PTD | EN  | M4)) /* 14 GPIO_162 / McBSP1-CLKX	- unused */\
MUX_VAL(CP(MCBSP1_FSX),		(IDIS | PTD | EN  | M4)) /* 16 GPIO_161 / McBSP1-FSX	- backlight shutdown */\
MUX_VAL(CP(MCBSP1_DR),		(IEN  | PTD | EN  | M4)) /* 18 GPIO_159 / McBSP1-DR		- controls EXT line for RS232 */\
MUX_VAL(CP(MCBSP1_CLKR),	(IDIS | PTD | EN  | M4)) /* 20 GPIO_156 / McBSP1-CLR	- GPS ON/OFF */\
MUX_VAL(CP(MCBSP1_FSR),		(IEN  | PTU | EN  | M4)) /* 22 GPIO_157 / McBSP1-FSR	- PENIRQ */\


