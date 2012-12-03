// all pins on BB expansion connector

// GPIO -> BB-Pin -> Expander function

#define MUX_BEAGLE_EXPANDER() \
MUX_VAL(CP(MMC2_CLK),		(IEN  | PTU | EN  | M4)) /*GPIO_130 / MCSPI3-CLK*/\
MUX_VAL(CP(MMC2_CMD),		(IEN  | PTU | EN  | M4)) /*GPIO_131 / MCSPI3-SIMO -> */\
MUX_VAL(CP(MMC2_DAT0),		(IEN  | PTU | EN  | M4)) /*GPIO_132 / MCSPI3-SOMI -> */\
MUX_VAL(CP(MMC2_DAT1),		(IEN  | PTU | EN  | M4)) /*GPIO_133 / UART3-RX (software)*/\
MUX_VAL(CP(MMC2_DAT2),		(IEN  | PTU | EN  | M4)) /*GPIO_134 / UART3-TX (software)*/\
MUX_VAL(CP(MMC2_DAT3),		(IEN  | PTU | EN  | M4)) /*GPIO_135 / MCSPI3-CS0*/\
MUX_VAL(CP(MMC2_DAT4),		(IEN  | PTU | EN  | M4)) /*GPIO_136 / AUX */\
MUX_VAL(CP(MMC2_DAT5),		(IEN  | PTU | EN  | M4)) /*GPIO_137 / POWER */\
MUX_VAL(CP(MMC2_DAT6),		(IEN  | PTU | DIS | M4)) /*GPIO_138 / UART3-RTS (software) -> EXT-ANT */\
MUX_VAL(CP(MMC2_DAT7),		(IEN  | PTU | EN  | M4)) /*GPIO_139 / UART3-CTS (software) */\
MUX_VAL(CP(UART2_RX),		(IEN  | PTU | EN  | M4)) /*GPIO_143 / UART2-RX */\
MUX_VAL(CP(UART2_CTS),		(IEN  | PTU | EN  | M4)) /*GPIO_144 / UART2-CTS -> KEYIRQ*/\
MUX_VAL(CP(UART2_RTS),		(IEN  | PTU | EN  | M4)) /*GPIO_145 / UART2-RTS */\
MUX_VAL(CP(UART2_TX),		(IEN  | PTU | EN  | M4)) /*GPIO_146 / UART2-TX */\
MUX_VAL(CP(MCBSP1_CLKR),	(IEN  | PTD | DIS | M4)) /*GPIO_156 / ... - KEYIRQ -> TCA8418 */\
MUX_VAL(CP(MCBSP1_FSR),		(IEN  | PTU | EN  | M4)) /*GPIO_157 / ... - PENIRQ */\
MUX_VAL(CP(MCBSP1_DX),		(IDIS | PTU | EN  | M4)) /*GPIO_158 / ... - Display DOUT */\
MUX_VAL(CP(MCBSP1_DR),		(IEN  | PTU | DIS | M4)) /*GPIO_159 / McBSP1-DR -> Display DIN - pulled up */\
MUX_VAL(CP(MCBSP_CLKS),		(IEN  | PTU | DIS | M0)) /*GPIO_??? / McBSP_CLKS */\
MUX_VAL(CP(MCBSP1_FSX),		(IDIS | PTU | EN  | M4)) /*GPIO_161 / McBSP1-FSX -> Display CS */\
MUX_VAL(CP(MCBSP1_CLKX),	(IDIS | PTU | EN  | M4)) /*GPIO_162 / McBSP1-CLKX -> Display SCL */