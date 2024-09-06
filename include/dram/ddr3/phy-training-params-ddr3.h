/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2023 RnD Center "ELVEES", JSC */

#ifndef _PHY_TRAINING_PARAMS_DDR3_H
#define _PHY_TRAINING_PARAMS_DDR3_H

typedef struct _PMU_SMB_DDR3U_1D_t {
	/* Byte offset 0x00, CSR Addr 0x54000, Direction=In
	 *
	 * Reserved00[0:4] RFU, must be zero
	 * Reserved00[5] - Train vrefDAC0 During Read Deskew
	 *   0x1 - Read Deskew will begin by enabling and roughly training the PHY's per-lane
	 *         reference voltages. Training the vrefDACs CSRs will increase the maximum
	 *         1D training time by around half a millisecond, but will improve 1D training
	 *         accuracy on systems with significant voltage-offsets between lane read eyes.
	 *   0x0 - Read Deskew will assume the messageblock's phyVref setting will work for all
	 *         lanes.
	 * Reserved00[6] - Enable High Effort WrDQ1D
	 *   0x1 - WrDQ1D will conditionally retry training at several extra RxClkDly Timings.
	 *         This will increase the maximum 1D training time by up to 4 extra iterations
	 *         of WrDQ1D. This is only required in systems that suffer from very large,
	 *         asymmetric eye-collapse when receiving PRBS patterns.
	 *   0x0 - WrDQ1D assume rxClkDly values found by SI Friendly RdDqs1D will work for
	 *         receiving PRBS patterns
	 * Reserved00[7] - Optimize for the special hard macros in TSMC28
	 *   0x1 - Set if the PHY being trained was manufactured in any TSMC28 process node
	 *   0x0 - Otherwise, when not training a TSMC28 phy, leave this field as 0
	 */
	uint8_t Reserved00;

	/* Byte offset 0x01, CSR Addr 0x54000, Direction=In
	 *
	 * MsgMisc[0:4] - RFU, must be zero
	 * MsgMisc[5] - PerByteMaxRdLat
	 *   0x1 - Each DBYTE will return dfi_rddata_valid at the lowest possible latency.
	 *         This may result in unaligned data between bytes to be returned to the DFI.
	 *   0x0 - Every DBYTE will return  dfi_rddata_valid simultaneously. This will ensure
	 *         that data bytes will return aligned accesses to the DFI.
	 * MsgMisc[6] - PartialRank (DDR3 UDIMM and DDR4 UDIMM only, otherwise RFU, must be zero)
	 *   0x1 - Support rank populated with a subset of byte, but where even-odd pair of rank
	 *         support all the byte.
	 *   0x0 - All rank populated with all the byte (tyical configuration)
	 * MsgMisc[7] - RFU, must be zero
	 */
	uint8_t MsgMisc;

	/* Byte offset 0x02, CSR Addr 0x54001, Direction=Out
	 *
	 * PMU firmware revision ID. After training is run, this address will contain the revision
	 * ID of the firmware.
	 */
	uint16_t PmuRevision;

	/* Byte offset 0x04, CSR Addr 0x54002, Direction=In
	 *
	 * Must be set to the target Pstate to be trained
	 *   0x0 - Pstate 0
	 *   0x1 = Pstate 1
	 *   0x2 = Pstate 2
	 *   0x3 = Pstate 3
	 * All other encodings are reserved.
	 */
	uint8_t Pstate;

	/* Byte offset 0x05, CSR Addr 0x54002, Direction=In
	 *
	 * Set according to whether target Pstate uses PHY PLL bypass
	 *   0x0 - PHY PLL is enabled for target Pstate
	 *   0x1 - PHY PLL is bypassed for target Pstate
	 */
	uint8_t PllBypassEn;

	/* Byte offset 0x06, CSR Addr 0x54003, Direction=In
	 *
	 * DDR data rate for the target Pstate in units of MT/s.
	 * For example enter 0x0640 for DDR1600.
	 */
	uint16_t DRAMFreq;

	/* Byte offset 0x08, CSR Addr 0x54004, Direction=In
	 *
	 * Frequency ratio between DfiCtlClk and SDRAM memclk.
	 *   0x1 - 1:1
	 *   0x2 - 1:2
	 *   0x4 - 1:4
	 */
	uint8_t DfiFreqRatio;

	/* Byte offset 0x09, CSR Addr 0x54004, Direction=In
	 *
	 * Must be programmed to match the precision resistor connected to Phy BP_ZN
	 *   0x00 - Do not program. Use current CSR value.
	 *   0xf0 - 240 Ohm (recommended value)
	 *   0x78 - 120 Ohm
	 *   0x28 - 40 Ohm
	 * All other values are reserved.
	 */
	uint8_t BPZNResVal;

	/* Byte offset 0x0a, CSR Addr 0x54005, Direction=In
	 *
	 * Must be programmed to the termination impedance in ohms used by PHY during reads
	 *   0x0 - Firmware skips programming (must be manually programmed by user prior to
	 *         training start)
	 *   See PHY databook for legal termination impedance values.
	 */
	uint8_t PhyOdtImpedance;

	/* Byte offset 0x0b, CSR Addr 0x54005, Direction=In
	 *
	 * Must be programmed to the driver impedance in ohms used by PHY during writes for
	 * all DBYTE drivers (DQ/DM/DBI/DQS)
	 *   0x0 - Firmware skips programming (must be manually programmed by user prior to
	 *         training start)
	 *   See PHY databook for legal R_on driver impedance values.
	 */
	uint8_t PhyDrvImpedance;

	/* Byte offset 0x0c, CSR Addr 0x54006, Direction=In
	 *
	 * Must be programmed with the Vref level to be used by the PHY during reads
	 * The units of this field are a percentage of VDDQ according to the following equation:
	 * Receiver Vref = VDDQ*PhyVref[6:0]/128. For example to set Vref at 0.75*VDDQ, set this
	 * field to 0x60.
	 */
	uint8_t PhyVref;

	/* Byte offset 0x0d, CSR Addr 0x54006, Direction=In
	 *
	 * Module Type:
	 *   0x01 - DDR3 unbuffered
	 * All other values are reserved.
	 */
	uint8_t DramType;

	/* Byte offset 0x0e, CSR Addr 0x54007, Direction=In
	 *
	 * Bitmap to indicate which Dbyte are not connected (for DByte 0 to 7).
	 * Set DisabledDbyte[i] to 1 only to specify that DByte i not need to be trained
	 * (DByte 8 can be disabled via EnabledDQs setting)
	 */
	uint8_t DisabledDbyte;

	/* Byte offset 0x0f, CSR Addr 0x54007, Direction=In
	 *
	 * Total number of DQ bits enabled in PHY
	 */
	uint8_t EnabledDQs;

	/* Byte offset 0x10, CSR Addr 0x54008, Direction=In
	 *
	 * Indicates presence of DRAM at each chip select for PHY. Each bit corresponds to a
	 * logical CS.
	 * If the bit is set to 1, the CS is connected to DRAM.
	 * If the bit is set to 0, the CS is not connected to DRAM.
	 *   CsPresent[0] - CS0 is populated with DRAM
	 *   CsPresent[1] - CS1 is populated with DRAM
	 *   CsPresent[2] - CS2 is populated with DRAM
	 *   CsPresent[3] - CS3 is populated with DRAM
	 *   CsPresent[7:4] - Reserved (must be programmed to 0)
	 */
	uint8_t CsPresent;

	/* Byte offset 0x11, CSR Addr 0x54008, Direction=In
	 *
	 * The CS signals from field CsPresent that are routed to DIMM connector 0
	 *
	 */
	uint8_t CsPresentD0;

	/* Byte offset 0x12, CSR Addr 0x54009, Direction=In
	 *
	 * The CS signals from field CsPresent that are routed to DIMM connector 1
	 */
	uint8_t CsPresentD1;

	/* Byte offset 0x13, CSR Addr 0x54009, Direction=In
	 *
	 * Corresponds to CS[3:0]
	 *   1 - Address Mirror
	 *   0 - No Address Mirror
	 */
	uint8_t AddrMirror;

	/* Byte offset 0x14, CSR Addr 0x5400a, Direction=Out
	 *
	 * This field will be set if training fails on any rank.
	 *   0x0 - No failures
	 *   non-zero - one or more ranks failed training
	 */
	uint8_t CsTestFail;

	/* Byte offset 0x15, CSR Addr 0x5400a, Direction=In
	 *
	 * Additional mode bits. Bit fields:
	 *   [0] SlowAccessMode: 1 - 2T Address Timing, 0 - 1T Address Timing
	 *   [7:1] - RFU, must be zero
	 */
	uint8_t PhyCfg;

	/* Byte offset 0x16, CSR Addr 0x5400b, Direction=In
	 *
	 * Controls the training steps to be run. Each bit corresponds to a training step.
	 * If the bit is set to 1, the training step will run.
	 * If the bit is set to 0, the training step will be skipped.
	 *
	 * Training step to bit mapping:
	 *   SequenceCtrl[0] - Run DevInit - Device/phy initialization. Should always be set.
	 *   SequenceCtrl[1] - Run WrLvl - Write leveling
	 *   SequenceCtrl[2] - Run RxEn - Read gate training
	 *   SequenceCtrl[3] = Run RdDQS1D - 1d read dqs training
	 *   SequenceCtrl[4] = Run WrDQ1D - 1d write dq training
	 *   SequenceCtrl[7:5] - RFU, must be zero
	 *   SequenceCtrl[8] - Run RdDeskew - Per lane read dq deskew training
	 *   SequenceCtrl[9] - Run MxRdLat - Max read latency training
	 *   SequenceCtrl[15:10] - RFU, must be zero
	 */
	uint16_t SequenceCtrl;

	/* Byte offset 0x18, CSR Addr 0x5400c, Direction=In
	 *
	 * To control the total number of debug messages, a verbosity subfield HdtCtrl exists
	 * in the message block. Every message has a verbosity level associated with it, and
	 * as the HdtCtrl value is increased, less important messages stop being sent through
	 * the mailboxes. The meanings of several major HdtCtrl thresholds are explained below:
	 *   0x05 - Detailed debug messages (e.g. Eye delays)
	 *   0x0A - Coarse debug messages (e.g. rank information)
	 *   0xC8 - Stage completion
	 *   0xC9 - Assertion messages
	 *   0xFF - Firmware completion messages only
	 * See Training App Note for more detailed information on what messages are included for
	 * each threshold.
	 */
	uint8_t HdtCtrl;

	uint8_t Reserved19;
	uint8_t Reserved1A;

	/* Byte offset 0x1b, CSR Addr 0x5400d, Direction=In
	 *
	 * Bitmap that designates the phy's vref source for every pstate:
	 *   0x0 - after 2D training, pstate X will continue using the phyVref provided in pstate
	 *         X's 1D messageblock
	 *   0x1 - after 2D training, pstate X will use the per-lane VrefDAC0/1 CSRs trained by
	 *         2d training.
	 */
	uint8_t Share2DVrefResult;

	uint8_t Reserved1C;
	uint8_t Reserved1D;

	/* Byte offset 0x1e, CSR Addr 0x5400f, Direction=In
	 *
	 * Input for constraining the range of vref(DQ) values training will collect data for,
	 * usually reducing training time. However, too large of a voltage range may cause longer
	 * 2D training times while too small of a voltage range may truncate passing regions.
	 * When in doubt, leave this field set to 0.
	 * Used by 2D stages: Rd2D, Wr2D
	 *
	 * Reserved1A[3:0] - Rd2D Voltage Range
	 *   0x0 - Training will search all phy vref(DQ) settings
	 *   0x1 - Limit to +/-2 %VDDQ from phyVref
	 *   0x2 - Limit to +/-4 %VDDQ from phyVref
	 *   ...
	 *   0xf - Limit to +/-30% VDDQ from phyVref
	 *
	 * Reserved1A[7:4] - Wr2D Voltage Range
	 *   0x0 - Training will search all dram vref(DQ) settings
	 *   0x1 - Limit to +/-2 %VDDQ from MR14
	 *   0x2 - Limit to +/-4 %VDDQ from MR14
	 *   ...
	 *   0xf - Limit to +/-30% VDDQ from MR14
	 */
	uint8_t Reserved1E;

	uint8_t Reserved1F;
	uint8_t Reserved20;
	uint8_t Reserved21;

	/* Byte offset 0x22, CSR Addr 0x54011, Direction=In
	 *
	 * Override PhyConfig csr
	 *   0x0 - Use hardware csr value for PhyConfig (recommended)
	 *   Other values - Use value for PhyConfig instead of Hardware value.
	 */
	uint16_t PhyConfigOverride;

	/* Byte offset 0x24, CSR Addr 0x54012, Direction=In
	 *
	 * Margin added to smallest passing trained DFI Max Read Latency value, in units
	 * of DFI clocks. Recommended to be >= 1. See the Training App Note for more details
	 * on the training process and the use of this value.
	 */
	uint8_t DFIMRLMargin;

	/* Byte offsets 0x25 - 0x5c, Direction=Out
	 *
	 * RR/WW/RW/WR critical delay difference from cs X to cs Y.
	 * See PUB Databook section 8.2 for details on use of CDD values.
	 */
	int8_t CDD_RR_3_2;
	int8_t CDD_RR_3_1;
	int8_t CDD_RR_3_0;
	int8_t CDD_RR_2_3;
	int8_t CDD_RR_2_1;
	int8_t CDD_RR_2_0;
	int8_t CDD_RR_1_3;
	int8_t CDD_RR_1_2;
	int8_t CDD_RR_1_0;
	int8_t CDD_RR_0_3;
	int8_t CDD_RR_0_2;
	int8_t CDD_RR_0_1;
	int8_t CDD_WW_3_2;
	int8_t CDD_WW_3_1;
	int8_t CDD_WW_3_0;
	int8_t CDD_WW_2_3;
	int8_t CDD_WW_2_1;
	int8_t CDD_WW_2_0;
	int8_t CDD_WW_1_3;
	int8_t CDD_WW_1_2;
	int8_t CDD_WW_1_0;
	int8_t CDD_WW_0_3;
	int8_t CDD_WW_0_2;
	int8_t CDD_WW_0_1;
	int8_t CDD_RW_3_3;
	int8_t CDD_RW_3_2;
	int8_t CDD_RW_3_1;
	int8_t CDD_RW_3_0;
	int8_t CDD_RW_2_3;
	int8_t CDD_RW_2_2;
	int8_t CDD_RW_2_1;
	int8_t CDD_RW_2_0;
	int8_t CDD_RW_1_3;
	int8_t CDD_RW_1_2;
	int8_t CDD_RW_1_1;
	int8_t CDD_RW_1_0;
	int8_t CDD_RW_0_3;
	int8_t CDD_RW_0_2;
	int8_t CDD_RW_0_1;
	int8_t CDD_RW_0_0;
	int8_t CDD_WR_3_3;
	int8_t CDD_WR_3_2;
	int8_t CDD_WR_3_1;
	int8_t CDD_WR_3_0;
	int8_t CDD_WR_2_3;
	int8_t CDD_WR_2_2;
	int8_t CDD_WR_2_1;
	int8_t CDD_WR_2_0;
	int8_t CDD_WR_1_3;
	int8_t CDD_WR_1_2;
	int8_t CDD_WR_1_1;
	int8_t CDD_WR_1_0;
	int8_t CDD_WR_0_3;
	int8_t CDD_WR_0_2;
	int8_t CDD_WR_0_1;
	int8_t CDD_WR_0_0;

	uint8_t Reserved5D;

	/* Byte offset 0x5e - 0x62, CSR Addr 0x5402f - 0x54031, Direction=In
	 *
	 * Value of DDR mode register MRx for all ranks for current pstate
	 */
	uint16_t MR0;
	uint16_t MR1;
	uint16_t MR2;

	uint8_t Reserved64;
	uint8_t Reserved65;
	uint8_t Reserved66;
	uint8_t Reserved67;
	uint8_t Reserved68;
	uint8_t Reserved69;
	uint8_t Reserved6A;
	uint8_t Reserved6B;
	uint8_t Reserved6C;
	uint8_t Reserved6D;
	uint8_t Reserved6E;
	uint8_t Reserved6F;
	uint8_t Reserved70;
	uint8_t Reserved71;
	uint8_t Reserved72;
	uint8_t Reserved73;

	/* Byte offset 0x74 - 0x77, CSR Addr 0x5403a - 0x5403b, Direction=In
	 *
	 * ODT pattern for accesses targeting rank X. [3:0] is used for write ODT [7:4]
	 *  is used for read ODT
	 */
	uint8_t AcsmOdtCtrl0;
	uint8_t AcsmOdtCtrl1;
	uint8_t AcsmOdtCtrl2;
	uint8_t AcsmOdtCtrl3;

	/* Byte offset 0x78 - 0x7b, CSR Addr 0x5403c - 0x5403d, Direction=In
	 *
	 * This field is reserved and must be programmed to 0x00.
	 */
	uint8_t AcsmOdtCtrl4;
	uint8_t AcsmOdtCtrl5;
	uint8_t AcsmOdtCtrl6;
	uint8_t AcsmOdtCtrl7;
	uint8_t Reserved7C;
	uint8_t Reserved7D;
	uint8_t Reserved7E;
	uint8_t Reserved7F;
	uint8_t Reserved80;
	uint8_t Reserved81;
	uint8_t Reserved82;
	uint8_t Reserved83;
	uint8_t Reserved84;
	uint8_t Reserved85;
	uint8_t Reserved86;
	uint8_t Reserved87;
	uint8_t Reserved88;
	uint8_t Reserved89;
	uint8_t Reserved8A;
	uint8_t Reserved8B;
	uint8_t Reserved8C;
	uint8_t Reserved8D;
	uint8_t Reserved8E;
	uint8_t Reserved8F;
	uint8_t Reserved90;
	uint8_t Reserved91;
	uint8_t Reserved92;
	uint8_t Reserved93;
	uint8_t Reserved94;
	uint8_t Reserved95;
	uint8_t Reserved96;
	uint8_t Reserved97;
	uint8_t Reserved98;
	uint8_t Reserved99;
	uint8_t Reserved9A;
	uint8_t Reserved9B;
	uint8_t Reserved9C;
	uint8_t Reserved9D;
	uint8_t Reserved9E;
	uint8_t Reserved9F;
	uint8_t ReservedA0;
	uint8_t ReservedA1;
	uint8_t ReservedA2;
	uint8_t ReservedA3;
} __attribute__((packed)) PMU_SMB_DDR3U_1D_t;

#endif
