/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _MAINBOARD_GPIO_H
#define _MAINBOARD_GPIO_H

#include <soc/gpio.h>

#ifndef __ACPI__

#define PAD_NC_PWROK(pad, pull)	PAD_CFG_GPI(pad, pull, PWROK)

const struct pad_config tagada_gpio_config[] = {
	// GBE0_SDP0 (GPIO_14) NC
/*ME	PAD_CFG_NF(NORTH_ALL_GBE0_SDP0, NONE, PWROK, NF1), */
	// GBE1_SDP0 (GPIO_15) NC
	PAD_CFG_NF(NORTH_ALL_GBE1_SDP0, NONE, PWROK, NF1),
	// GBE2_I2C_CLK (GPIO_16) NC
	PAD_CFG_GPO(NORTH_ALL_GBE0_SDP1, 0, PWROK),
	// GBE2_I2C_DATA (GPIO_17) NC
	PAD_NC_PWROK(NORTH_ALL_GBE1_SDP1, NONE),
	// GBE2_SDP0 (GPIO_18) NC
	PAD_CFG_NF(NORTH_ALL_GBE0_SDP2, NONE, PWROK, NF2),
	// GBE3_SDP0 (GPIO_19) NC
	PAD_CFG_NF(NORTH_ALL_GBE1_SDP2, NONE, PWROK, NF2),
	// GBE3_I2C_CLK (GPIO_20) NC
	PAD_CFG_GPO(NORTH_ALL_GBE0_SDP3, 0, PWROK),
	// GBE3_I2C_DATA (GPIO_21) NC
	PAD_NC_PWROK(NORTH_ALL_GBE1_SDP3, NONE),
	// GBE2_LED0 (GPIO_22) Z1:NC / A0:ETH0_LED0
	PAD_CFG_NF(NORTH_ALL_GBE2_LED0, NONE, PWROK, NF1),
	// GBE2_LED1 (GPIO_23) Z1:NC / A0:ETH0_LED1
	PAD_CFG_NF(NORTH_ALL_GBE2_LED1, NONE, PWROK, NF1),
	// GBE0_I2C_CLK (GPIO_24) NC
	PAD_CFG_GPO(NORTH_ALL_GBE0_I2C_CLK, 0, PWROK),
	// GBE0_I2C_DATA (GPIO_25) NC
	PAD_NC_PWROK(NORTH_ALL_GBE0_I2C_DATA, NONE),
	// GBE1_I2C_CLK (GPIO_26) NC
	PAD_CFG_GPO(NORTH_ALL_GBE1_I2C_CLK, 0, PWROK),
	// GBE1_I2C_DATA (GPIO_27) NC
	PAD_NC_PWROK(NORTH_ALL_GBE1_I2C_DATA, NONE),
	// NCSI_RXD0 (GPIO_28) NC
	PAD_CFG_NF(NORTH_ALL_NCSI_RXD0, NONE, PWROK, NF2),
	// NCSI_CLK_IN (GPIO_29) Pull Down
	PAD_CFG_NF(NORTH_ALL_NCSI_CLK_IN, NONE, PWROK, NF2),
	// NCSI_RXD1 (GPIO_30) NC
	PAD_CFG_NF(NORTH_ALL_NCSI_RXD1, NONE, PWROK, NF2),
	// NCSI_CRS_DV (GPIO_31) NC
	PAD_CFG_NF(NORTH_ALL_NCSI_CRS_DV, NONE, PWROK, NF2),
	// NCSI_ARB_IN (GPIO_32) NC
	PAD_CFG_NF(NORTH_ALL_NCSI_ARB_IN, NONE, PWROK, NF2),
	// NCSI_TX_EN (GPIO_33) Pull Down
	PAD_CFG_NF(NORTH_ALL_NCSI_TX_EN, NONE, PWROK, NF2),
	// NCSI_TXD0 (GPIO_34) Pull Down
	PAD_CFG_NF(NORTH_ALL_NCSI_TXD0, NONE, PWROK, NF2),
	// NCSI_TXD1 (GPIO_35) Pull Down
	PAD_CFG_NF(NORTH_ALL_NCSI_TXD1, NONE, PWROK, NF2),
	// NCSI_ARB_OUT (GPIO_36) NC
	PAD_CFG_NF(NORTH_ALL_NCSI_ARB_OUT, NONE, PWROK, NF2),
	// GBE0_LED0 (GPIO_37) Z1:ETH0_LED0 / A1:ETH1_LED0
	PAD_CFG_NF(NORTH_ALL_GBE0_LED0, NONE, PWROK, NF1),
	// GBE0_LED1 (GPIO_38) Z1:ETH0_LED1 / A1:ETH1_LED1
	PAD_CFG_NF(NORTH_ALL_GBE0_LED1, NONE, PWROK, NF1),
	// GBE1_LED0 (GPIO_39) Z1:ETH1_LED0 / A1:NC
	PAD_CFG_NF(NORTH_ALL_GBE1_LED0, NONE, PWROK, NF1),
	// GBE1_LED1 (GPIO_40) Z1:ETH1_LED1 / A1:NC
	PAD_CFG_NF(NORTH_ALL_GBE1_LED1, NONE, PWROK, NF1),
	// ADR-COMPLETE (GPIO_0) LFFF: DVT_GPIO<0> : BOOTED, output
	PAD_CFG_GPO(NORTH_ALL_GPIO_0, 0, PWROK),
	// PCIE_CLKREQ0_N (GPIO_41) Pull Up
	PAD_CFG_NF(NORTH_ALL_PCIE_CLKREQ0_N, NONE, PWROK, NF1),
	// PCIE_CLKREQ1_N (GPIO_42) Pull Up
	PAD_CFG_NF(NORTH_ALL_PCIE_CLKREQ1_N, NONE, PWROK, NF1),
	// PCIE_CLKREQ2_N (GPIO_43) Pull Up
	PAD_CFG_NF(NORTH_ALL_PCIE_CLKREQ2_N, NONE, PWROK, NF1),
	// PCIE_CLKREQ3_N (GPIO_44) Pull Up
	PAD_CFG_NF(NORTH_ALL_PCIE_CLKREQ3_N, NONE, PWROK, NF1),
	// PCIE_CLKREQ4_N (GPIO_45) Pull Up
	PAD_CFG_NF(NORTH_ALL_PCIE_CLKREQ4_N, NONE, PWROK, NF1),
	// GBE_MDC (GPIO_1) NC
	PAD_CFG_NF(NORTH_ALL_GPIO_1, NONE, PWROK, NF1),
	// GBE_MDIO  (GPIO_2) NC
	PAD_CFG_NF(NORTH_ALL_GPIO_2, NONE, PWROK, NF1),
	// SVID_ALERT_N (GPIO_47) SVID_ALERTn
	PAD_CFG_NF(NORTH_ALL_SVID_ALERT_N, NONE, PWROK, NF1),
	// SVID_DATA (GPIO_48) SVID_DATA
	PAD_CFG_NF(NORTH_ALL_SVID_DATA, NONE, PWROK, NF1),
	// SVID_CLK (GPIO_49) SVID_CLK
	PAD_CFG_NF(NORTH_ALL_SVID_CLK, NONE, PWROK, NF1),
	// THERMTRIP_N (GPIO_50) SOC_THERMTRIPn Pull Up
	PAD_CFG_NF(NORTH_ALL_THERMTRIP_N, NONE, PWROK, NF1),
	// PROCHOT_N (GPIO_51) PROCHOTn Pull Up
	PAD_CFG_NF(NORTH_ALL_PROCHOT_N, NONE, PWROK, NF1),
	// MEMHOT_N (GPIO_52) SOC_MEMHOTn
	PAD_CFG_NF(NORTH_ALL_MEMHOT_N, NONE, PWROK, NF1),
	// DFX_PORT_CLK0 (GPIO_53) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT_CLK0, NONE, PWROK, NF1),
	// DFX_PORT_CLK1 (GPIO_54) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT_CLK1, NONE, PWROK, NF1),
	// DFX_PORT0 (GPIO_55) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT0, NONE, PWROK, NF1),
	// DFX_PORT1 (GPIO_56) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT1, NONE, PWROK, NF1),
	// DFX_PORT2 (GPIO_57) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT2, NONE, PWROK, NF1),
	// DFX_PORT3 (GPIO_58) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT3, NONE, PWROK, NF1),
	// DFX_PORT4 (GPIO_59) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT4, NONE, PWROK, NF1),
	// DFX_PORT5 (GPIO_60) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT5, NONE, PWROK, NF1),
	// DFX_PORT6 (GPIO_61) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT6, NONE, PWROK, NF1),
	// DFX_PORT7 (GPIO_62) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT7, NONE, PWROK, NF1),
	// DFX_PORT8 (GPIO_63) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT8, NONE, PWROK, NF1),
	// DFX_PORT9 (GPIO_134) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT9, NONE, PWROK, NF1),
	// DFX_PORT10 (GPIO_135) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT10, NONE, PWROK, NF1),
	// DFX_PORT11 (GPIO_136) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT11, NONE, PWROK, NF1),
	// DFX_PORT12 (GPIO_137) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT12, NONE, PWROK, NF1),
	// DFX_PORT13 (GPIO_138) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT13, NONE, PWROK, NF1),
	// DFX_PORT14 (GPIO_139) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT14, NONE, PWROK, NF1),
	// DFX_PORT15 (GPIO_140) NC
	PAD_CFG_NF(SOUTH_DFX_DFX_PORT15, NONE, PWROK, NF1),
	// SPI_TPM_CS_N (GPIO_12) HS_TCO_WDT NC (Possible Pull Up)
	PAD_CFG_NF(SOUTH_GROUP0_GPIO_12, NONE, PWROK, NF1),
	// SMB5_GBE_ALRT_N (GPIO_13) LAN_ALRTn Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SMB5_GBE_ALRT_N, NONE, PWROK, NF3),
	// PCIE_CLKREQ5_N (GPIO_98) Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_PCIE_CLKREQ5_N, NONE, PWROK, NF1),
	// PCIE_CLKREQ6_N (GPIO_99) Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_PCIE_CLKREQ6_N, NONE, PWROK, NF1),
	// PCIE_CLKREQ7_N (GPIO_100) Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_PCIE_CLKREQ7_N, NONE, PWROK, NF1),
	// UART0_RXD (GPIO_101) CONSOLE_RX
	PAD_CFG_NF(SOUTH_GROUP0_UART0_RXD, NONE, PWROK, NF1),
	// UART0_TXD (GPIO_102) CONSOLE_TX
	PAD_CFG_NF(SOUTH_GROUP0_UART0_TXD, NONE, PWROK, NF1),
	// SMB5_GBE_CLK (GPIO_103) LAN_SLC Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SMB5_GBE_CLK, NONE, PWROK, NF3),
	// SMB_GBE_DATA (GPIO_104) LAN_SDA Pull UP
	PAD_CFG_NF(SOUTH_GROUP0_SMB5_GBE_DATA, NONE, PWROK, NF3),
	// ERROR2_N (GPIO_105) ERRORn2
	PAD_CFG_NF(SOUTH_GROUP0_ERROR2_N, NONE, PWROK, NF1),
	// ERROR1_N (GPIO_106) ERRORn1
	PAD_CFG_NF(SOUTH_GROUP0_ERROR1_N, NONE, PWROK, NF1),
	// ERROR0_N (GPIO_107) ERRORn0 Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_ERROR0_N, NONE, PWROK, NF1),
	// IERR_N (CATERR_N) (GPIO_108) IERRn (HardStrap Pull Up)
	PAD_CFG_NF(SOUTH_GROUP0_IERR_N, NONE, PWROK, NF1),
	// MCERR_N  (GPIO_109) MCERR
	PAD_CFG_NF(SOUTH_GROUP0_MCERR_N, NONE, PWROK, NF1),
	// SMB0_LEG_CLK (GPIO_110) LEG_SCL Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SMB0_LEG_CLK, NONE, PWROK, NF1),
	// SMB0_LEG_DATA (GPIO_111) LEG_SDA Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SMB0_LEG_DATA, NONE, PWROK, NF1),
	// SMB0_LEG_ALRT_N (GPIO_112) Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SMB0_LEG_ALRT_N, NONE, PWROK, NF1),
	// SMB1_HOST_DATA (GPIO_113) HOST_SDA Pull Up
/*ME	PAD_CFG_NF(SOUTH_GROUP0_SMB1_HOST_DATA, NONE, PWROK, NF1), */
	// SMB1_HOST_CLK (GPIO_114) HOST_SCL Pull Up
/*ME	PAD_CFG_NF(SOUTH_GROUP0_SMB1_HOST_CLK, NONE, PWROK, NF1), */
	// SMB2_PECI_DATA (GPIO_115) Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SMB2_PECI_DATA, NONE, PWROK, NF1),
	// SMB2_PECI_CLK (GPIO_116) Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SMB2_PECI_CLK, NONE, PWROK, NF1),
	// SMB4_CSME0_DATA (GPIO_117) ME_SDA Pull Up
/*ME	PAD_CFG_NF(SOUTH_GROUP0_SMB4_CSME0_DATA, NONE, PWROK, NF1), */
	// SMB4_CSME0_CLK (GPIO_118) ME_SCL Pull Up
/*ME	PAD_CFG_NF(SOUTH_GROUP0_SMB4_CSME0_CLK, NONE, PWROK, NF1), */
	// SMB4_CSME0_ALRT_N (GPIO_119) ME_ALRTn Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SMB4_CSME0_ALRT_N, NONE, PWROK, NF1),
	// USB_OC0_N (GPIO_120) Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_USB_OC0_N, NONE, PWROK, NF1),
	// FLEX_CLK_SE0 (GPIO_121) NC
	PAD_CFG_NF(SOUTH_GROUP0_FLEX_CLK_SE0, NONE, PWROK, NF1),
	// FLEX_CLK_SE1 (GPIO_122) NC
	PAD_CFG_NF(SOUTH_GROUP0_FLEX_CLK_SE1, NONE, PWROK, NF1),
	// GBE3_LED1 (GPIO_4) LFFF: M2A_CFGn : M2A_SATAn, input
	PAD_CFG_GPI(SOUTH_GROUP0_GPIO_4, NONE, PWROK),
	// SMB3_IE0_CLK (GPIO_5) LFFF: M2B_CFGn : M2B_SATAn, input
	PAD_CFG_GPI(SOUTH_GROUP0_GPIO_5, NONE, PWROK),
	// SMB3_IE0_DATA (GPIO_6) NC
	PAD_CFG_NF(SOUTH_GROUP0_GPIO_6, NONE, PWROK, NF1),
	// SMB3_IE0_ALERT_N (GPIO_7) NC
	PAD_CFG_NF(SOUTH_GROUP0_GPIO_7, NONE, PWROK, NF1),
	// SATA0_LED (GPIO_90) SATA_LED0 Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SATA0_LED_N, NONE, PWROK, NF1),
	// SATA1_LED (GPIO_91) SATA_LED1 Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SATA1_LED_N, NONE, PWROK, NF1),
	// SATA_PDETECT0 (GPIO_92) Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SATA_PDETECT0, NONE, PWROK, NF2),
	// SATA_PDETECT1 (GPIO_93) Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_SATA_PDETECT1, NONE, PWROK, NF2),
	// UART1_RTS (GPIO_94) NC (Possible Pull Up)
	PAD_CFG_NF(SOUTH_GROUP0_SATA0_SDOUT, NONE, PWROK, NF1),
	// UART1_CTS (GPIO_95) NC (Possible Pull Up)
	PAD_CFG_NF(SOUTH_GROUP0_SATA1_SDOUT, NONE, PWROK, NF1),
	// UART1_RXD (GPIO_96) NC
	PAD_CFG_NF(SOUTH_GROUP0_UART1_RXD, NONE, PWROK, NF1),
	// UART1_TXD (GPIO_97) NC
	PAD_CFG_NF(SOUTH_GROUP0_UART1_TXD, NONE, PWROK, NF1),
	// SMB6_CSME1_DATA (GPIO_8) LFFF: DVT_GPIO<1> : Baud select, input
	PAD_CFG_GPI(SOUTH_GROUP0_GPIO_8, NONE, PWROK),
	// SMB6_CSME1_CLK (GPIO_9) LFFF: DVT_GPIO<2> : Verbose Traces, input
	PAD_CFG_GPI(SOUTH_GROUP0_GPIO_9, NONE, PWROK),
	// TCK (GPIO_141) n/a NC
	PAD_CFG_NF(SOUTH_GROUP0_TCK, NONE, PWROK, NF1),
	// TRST_N (GPIO_142) n/a NC
	PAD_CFG_NF(SOUTH_GROUP0_TRST_N, NONE, PWROK, NF1),
	// TMS (GPIO_143) n/a NC
	PAD_CFG_NF(SOUTH_GROUP0_TMS, NONE, PWROK, NF1),
	// TDI (GPIO_144) n/a NC
	PAD_CFG_NF(SOUTH_GROUP0_TDI, NONE, PWROK, NF1),
	// TDO (GPIO_145) n/a NC
	PAD_CFG_NF(SOUTH_GROUP0_TDO, NONE, PWROK, NF1),
	// CX_PRDY_N (GPIO_146) NC
	PAD_CFG_NF(SOUTH_GROUP0_CX_PRDY_N, NONE, PWROK, NF1),
	// CX-PREQ_N (GPIO_147) Pull Up
	PAD_CFG_NF(SOUTH_GROUP0_CX_PREQ_N, NONE, PWROK, NF1),
	// ME_RECVR_HDR (GPIO_148) ME_RECVR Pull Up
/*ME	PAD_NC_PWROK(SOUTH_GROUP0_CTBTRIGINOUT, NONE), */
	// ADV_DBG_DFX_HDR (GPIO_149) NC
	PAD_NC_PWROK(SOUTH_GROUP0_CTBTRIGOUT, NONE),
	// LAD2_SPI_IRQ_N (GPIO_150) NC
	PAD_NC_PWROK(SOUTH_GROUP0_DFX_SPARE2, NONE),
	// SMB_PECI_ALRT_N (GPIO_151) Pull Up
	PAD_NC_PWROK(SOUTH_GROUP0_DFX_SPARE3, NONE),
	// SMB_CSME1_ALRT_N (GPIO_152) NC
	PAD_NC_PWROK(SOUTH_GROUP0_DFX_SPARE4, NONE),
	// SUSPWRDNACK (GPIO_79) SUSPWRDNACK Pull Up
	PAD_CFG_NF(SOUTH_GROUP1_SUSPWRDNACK, NONE, PWROK, NF1),
	// PMU_SUSCLK (GPIO_80) PMU_SUSCLK
	PAD_CFG_NF(SOUTH_GROUP1_PMU_SUSCLK, NONE, PWROK, NF1),
	// ADR_TRIGGER_N (GPIO_81) Pull Down
	PAD_CFG_NF(SOUTH_GROUP1_ADR_TRIGGER, NONE, PWROK, NF1),
	// PMU_SLP_S45_N (GPIO_82) SLP_S45n
	PAD_CFG_NF(SOUTH_GROUP1_PMU_SLP_S45_N, NONE, PWROK, NF1),
	// PMU_SLP_S3_N (GPIO_83) SLP_S3n
	PAD_CFG_NF(SOUTH_GROUP1_PMU_SLP_S3_N, NONE, PWROK, NF1),
	// PMU_WAKE_N (GPIO_84) PMU_WAKEn Pull Up
	PAD_CFG_NF(SOUTH_GROUP1_PMU_WAKE_N, NONE, PWROK, NF1),
	// PMU_PWRBTN_N (GPIO_85) PWNBTNn
	PAD_CFG_NF(SOUTH_GROUP1_PMU_PWRBTN_N, NONE, PWROK, NF1),
	// PMU_RESETBUTTON_N (GPIO_86) RSTBTNn
	PAD_CFG_NF(SOUTH_GROUP1_PMU_RESETBUTTON_N, NONE, PWROK, NF1),
	// PMU_PLTRST_N (GPIO_87) PLTRSTn
	PAD_CFG_NF(SOUTH_GROUP1_PMU_PLTRST_N, NONE, PWROK, NF1),
	// PMU_SUS_STAT_N (GPIO_88) SUS_STATn
	PAD_CFG_NF(SOUTH_GROUP1_SUS_STAT_N, NONE, PWROK, NF1),
	// TDB_CIO_PLUG_EVENT (GPIO_89) NC
	PAD_NC_PWROK(SOUTH_GROUP1_SLP_S0IX_N, NONE),
	// SPI_CS0_N (GPIO_72) SPI_CS0
	PAD_CFG_NF(SOUTH_GROUP1_SPI_CS0_N, NONE, PWROK, NF1),
	// SPI_CS1_N (GPIO_73) NC
	PAD_CFG_NF(SOUTH_GROUP1_SPI_CS1_N, NONE, PWROK, NF1),
	// SPI_MOSI_IO0 (GPIO_74) SPI_MOSI
	PAD_CFG_NF(SOUTH_GROUP1_SPI_MOSI_IO0, NONE, PWROK, NF1),
	// SPI_MISO_IO1 (GPIO_75) SPI_MISO
	PAD_CFG_NF(SOUTH_GROUP1_SPI_MISO_IO1, NONE, PWROK, NF1),
	// SPI_IO2 (GPIO_76) NC
	PAD_CFG_NF(SOUTH_GROUP1_SPI_IO2, NONE, PWROK, NF1),
	// SPI_IO3 (GPIO_77) NC
	PAD_CFG_NF(SOUTH_GROUP1_SPI_IO3, NONE, PWROK, NF1),
	// SPI_CLK (GPIO_78) SPI_CLK
	PAD_CFG_NF(SOUTH_GROUP1_SPI_CLK, NONE, PWROK, NF1),
	// LPC_AD0 (GPIO_64) NC
	PAD_CFG_NF(SOUTH_GROUP1_ESPI_IO0, NONE, PWROK, NF2),
	// LPC_AD1 (GPIO_65) NC
	PAD_CFG_NF(SOUTH_GROUP1_ESPI_IO1, NONE, PWROK, NF2),
	// LPC_AD2 (GPIO_66) NC
	PAD_CFG_NF(SOUTH_GROUP1_ESPI_IO2, NONE, PWROK, NF2),
	// LPC_AD3 (GPIO_67) NC
	PAD_CFG_NF(SOUTH_GROUP1_ESPI_IO3, NONE, PWROK, NF2),
	// LPC_FRAME_N (GPIO_68) NC
	PAD_CFG_NF(SOUTH_GROUP1_ESPI_CS0_N, NONE, PWROK, NF2),
	// LPC_CLKOUT0 (GPIO_69) NC
	PAD_CFG_NF(SOUTH_GROUP1_ESPI_CLK, NONE, PWROK, NF2),
	// LPC_CLKOUT1 (GPIO_70) NC
	PAD_CFG_NF(SOUTH_GROUP1_ESPI_RST_N, NONE, PWROK, NF2),
	// LPC_CLKRUN_N (GPIO_71) Pull Up
	PAD_CFG_NF(SOUTH_GROUP1_ESPI_ALRT0_N, NONE, PWROK, NF2),
	// MFG_MODE_HDR (GPIO_10) MFG_MODE Pull Up
	PAD_NC_PWROK(SOUTH_GROUP1_GPIO_10, NONE),
	// LPC_SERIRQ (GPIO_11) NC
	PAD_CFG_NF(SOUTH_GROUP1_GPIO_11, NONE, PWROK, NF2),
	// EMMC-CMD (GPIO_123) NC
	PAD_CFG_NF(SOUTH_GROUP1_EMMC_CMD, NONE, PWROK, NF1),
	// EMMC-CSTROBE (GPIO_124) NC
	PAD_CFG_NF(SOUTH_GROUP1_EMMC_STROBE, NONE, PWROK, NF1),
	// EMMC-CLK (GPIO_125) NC
	PAD_CFG_NF(SOUTH_GROUP1_EMMC_CLK, NONE, PWROK, NF1),
	// EMMC-D0 (GPIO_126) NC
	PAD_CFG_NF(SOUTH_GROUP1_EMMC_D0, NONE, PWROK, NF1),
	// EMMC-D1 (GPIO_127) NC
	PAD_CFG_NF(SOUTH_GROUP1_EMMC_D1, NONE, PWROK, NF1),
	// EMMC-D2 (GPIO_128) NC
	PAD_CFG_NF(SOUTH_GROUP1_EMMC_D2, NONE, PWROK, NF1),
	// EMMC-D3 (GPIO_129) NC
	PAD_CFG_NF(SOUTH_GROUP1_EMMC_D3, NONE, PWROK, NF1),
	// EMMC-D4 (GPIO_130) NC
	PAD_CFG_NF(SOUTH_GROUP1_EMMC_D4, NONE, PWROK, NF1),
	// EMMC-D5 (GPIO_131) NC
	PAD_CFG_NF(SOUTH_GROUP1_EMMC_D5, NONE, PWROK, NF1),
	// EMMC-D6 (GPIO_132) NC
	PAD_CFG_NF(SOUTH_GROUP1_EMMC_D6, NONE, PWROK, NF1),
	// EMMC-D7 (GPIO_133) NC
	PAD_CFG_NF(SOUTH_GROUP1_EMMC_D7, NONE, PWROK, NF1),
	// IE_ROM GPIO (GPIO_3) HS_TSO NC (Possible Pull Up)
	PAD_CFG_GPO(SOUTH_GROUP1_GPIO_3, 0, PWROK),
};
#endif

#endif /* _MAINBOARD_GPIO_H */