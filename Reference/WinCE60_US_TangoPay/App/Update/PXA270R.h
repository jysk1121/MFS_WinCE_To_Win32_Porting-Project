/**
  GPIO Register Definitions
**/

typedef unsigned long          XLLP_UINT32_T,  *P_XLLP_UINT32_T;
typedef volatile XLLP_UINT32_T XLLP_VUINT32_T, *P_XLLP_VUINT32_T;
typedef struct 
{
    XLLP_VUINT32_T GPLR0;             /* Level Detect Reg. Bank 0 */
    XLLP_VUINT32_T GPLR1;             /* Level Detect Reg. Bank 1 */
    XLLP_VUINT32_T GPLR2;             /* Level Detect Reg. Bank 2 */

    XLLP_VUINT32_T GPDR0;            /* Data Direction Reg. Bank 0 */
    XLLP_VUINT32_T GPDR1;            /* Data Direction Reg. Bank 1 */
    XLLP_VUINT32_T GPDR2;            /* Data Direction Reg. Bank 2 */

    XLLP_VUINT32_T GPSR0;            /* Pin Output Set Reg. Bank 0 */
    XLLP_VUINT32_T GPSR1;            /* Pin Output Set Reg. Bank 1 */
    XLLP_VUINT32_T GPSR2;            /* Pin Output Set Reg. Bank 2 */
    XLLP_VUINT32_T GPCR0;            /* Pin Output Clr Reg. Bank 0 */
    XLLP_VUINT32_T GPCR1;            /* Pin Output Clr Reg. Bank 1 */
    XLLP_VUINT32_T GPCR2;            /* Pin Output Clr Reg. Bank 2 */
    XLLP_VUINT32_T GRER0;   /* Ris. Edge Detect Enable Reg. Bank 0 */
    XLLP_VUINT32_T GRER1;   /* Ris. Edge Detect Enable Reg. Bank 1 */
    XLLP_VUINT32_T GRER2;   /* Ris. Edge Detect Enable Reg. Bank 2 */
    XLLP_VUINT32_T GFER0;   /* Fal. Edge Detect Enable Reg. Bank 0 */
    XLLP_VUINT32_T GFER1;   /* Fal. Edge Detect Enable Reg. Bank 1 */
    XLLP_VUINT32_T GFER2;   /* Fal. Edge Detect Enable Reg. Bank 2 */
    XLLP_VUINT32_T GEDR0;       /* Edge Detect Status Reg. Bank 0 */
    XLLP_VUINT32_T GEDR1;       /* Edge Detect Status Reg. Bank 1 */
    XLLP_VUINT32_T GEDR2;       /* Edge Detect Status Reg. Bank 2 */
    XLLP_VUINT32_T GAFR0_L;  /* Alt. Function Select Reg.[  0:15 ] */
    XLLP_VUINT32_T GAFR0_U;  /* Alt. Function Select Reg.[ 16:31 ] */
    XLLP_VUINT32_T GAFR1_L;  /* Alt. Function Select Reg.[ 32:47 ] */
    XLLP_VUINT32_T GAFR1_U;  /* Alt. Function Select Reg.[ 48:63 ] */
    XLLP_VUINT32_T GAFR2_L;  /* Alt. Function Select Reg.[ 64:79 ] */
    XLLP_VUINT32_T GAFR2_U;  /* Alt. Function Select Reg.[ 80:95 ] */
    XLLP_VUINT32_T GAFR3_L;  /* Alt. Function Select Reg.[ 96:111] */
    XLLP_VUINT32_T GAFR3_U;  /* Alt. Function Select Reg.[112:120] */
    XLLP_VUINT32_T  RESERVED1[35];    /* addr. offset 0x074-0x0fc */
    XLLP_VUINT32_T GPLR3;             /* Level Detect Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED2[2];      /* addr. offset 0x104-0x108 */
    XLLP_VUINT32_T GPDR3;            /* Data Direction Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED3[2];      /* addr. offset 0x110-0x114 */
    XLLP_VUINT32_T GPSR3;            /* Pin Output Set Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED4[2];      /* addr. offset 0x11c-0x120 */
    XLLP_VUINT32_T GPCR3;            /* Pin Output Clr Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED5[2];      /* addr. offset 0x128-0x12c */
    XLLP_VUINT32_T GRER3;   /* Ris. Edge Detect Enable Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED6[2];      /* addr. offset 0x134-0x138 */
    XLLP_VUINT32_T GFER3;   /* Fal. Edge Detect Enable Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED7[2];      /* addr. offset 0x140-0x144 */
    XLLP_VUINT32_T GEDR3;       /* Edge Detect Status Reg. Bank 3 */

} XLLP_GPIO_T, *P_XLLP_GPIO_T;

typedef XLLP_GPIO_T BULVERDE_GPIO_REG;
typedef XLLP_GPIO_T *PBULVERDE_GPIO_REG;



#define BULVERDE_BASE_REG_PA_PERIPH     0x40000000
/////////////////////////////////////////////////////////////////////////////////////////
/* PERIPHERAL OFFSETS */
/////////////////////////////////////////////////////////////////////////////////////////

#define DMAC_OFFSET                     0x0             // DMA CONTROLLER
#define FFUART_OFFSET                   0x00100000      // Full-Feature UART
#define BTUART_OFFSET                   0x00200000      // BlueTooth UART
#define I2C_OFFSET                      0x00300000      // I2C
#define I2S_OFFSET                      0x00400000      // I2S
#define AC97_OFFSET                     0x00500000      // AC97
#define UDC_OFFSET                      0x00600000      // UDC (usb client)
#define STUART_OFFSET                   0x00700000      // Standard UART
#define FIR_OFFSET                      0x00800000      // FIR
#define RTC_OFFSET                      0x00900000      // real time clock
#define OST_OFFSET                      0x00A00000      // OS Timer
#define PWM0_2_OFFSET                   0x00B00000      // PWM 0 (pulse-width mod)
#define PWM1_3_OFFSET                   0x00C00000      // PWM 1 (pulse-width mod)
#define INTC_OFFSET                     0x00D00000      // Interrupt controller
#define GPIO_OFFSET                     0x00E00000      // GPIO
#define PWR_OFFSET                      0x00F00000      // Power Manager and Reset Control
#define SSP1_OFFSET                     0x01000000      // SSP 1
#define MMC_OFFSET                      0x01100000      // MMC
#define CLKMGR_OFFSET                   0x01300000      // Clock Manager
#define BB_OFFSET                       0x01400000      // Baseband Interface
#define KEYPAD_OFFSET                   0x01500000      // Keypad Interface
#define USIM_OFFSET                     0x01600000      // USIM
#define SSP2_OFFSET                     0x01700000      // SSP 2
#define MEMSTK_OFFSET                   0x01800000      // Memory Stick
#define SSP3_OFFSET                     0x01900000      // SSP 3


#define BULVERDE_BASE_REG_PA_GPIO       (BULVERDE_BASE_REG_PA_PERIPH + GPIO_OFFSET)
/* Pin  93  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_DKIN0			(XLLP_BIT_26)
#define XLLP_GPIO_AF_BIT_KP_DKIN0_MASK		(0x3u  << 26)

/* Pin  94  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_DKIN1			(XLLP_BIT_28)
#define XLLP_GPIO_AF_BIT_KP_DKIN1_MASK		(0x3u  << 28)
/* GPIO Pin Bank 2 */
#define XLLP_GPIO_BIT_L_DD6					( XLLP_BIT_0 )
#define XLLP_GPIO_BIT_L_DD7					( XLLP_BIT_1 )
#define XLLP_GPIO_BIT_L_DD8					( XLLP_BIT_2 )
#define XLLP_GPIO_BIT_L_DD9					( XLLP_BIT_3 )
#define XLLP_GPIO_BIT_L_DD10				( XLLP_BIT_4 )
#define XLLP_GPIO_BIT_L_DD11				( XLLP_BIT_5 )
#define XLLP_GPIO_BIT_L_DD12				( XLLP_BIT_6 ) 
#define XLLP_GPIO_BIT_L_DD13				( XLLP_BIT_7 )
#define XLLP_GPIO_BIT_L_DD14				( XLLP_BIT_8 )
#define XLLP_GPIO_BIT_L_DD15				( XLLP_BIT_9 )
#define XLLP_GPIO_BIT_L_FCLK				( XLLP_BIT_10 )
#define XLLP_GPIO_BIT_L_LCLK				( XLLP_BIT_11 )
#define XLLP_GPIO_BIT_L_PCLK				( XLLP_BIT_12 )
#define XLLP_GPIO_BIT_L_BIAS				( XLLP_BIT_13 )
#define XLLP_GPIO_BIT_nCS2					( XLLP_BIT_14 )
#define XLLP_GPIO_BIT_PCMCIA_PSKTSEL		( XLLP_BIT_15 )
#define XLLP_GPIO_BIT_nCS4					( XLLP_BIT_16 )
#define XLLP_GPIO_BIT_BB_OB_DAT0			( XLLP_BIT_17 )
#define XLLP_GPIO_BIT_BB_IB_DAT0			( XLLP_BIT_18 )
#define XLLP_GPIO_BIT_BB_IB_CLK				( XLLP_BIT_19 )
#define XLLP_GPIO_BIT_BB_IB_STB				( XLLP_BIT_20 )
#define XLLP_GPIO_BIT_BB_IB_WAIT			( XLLP_BIT_21 )
#define XLLP_GPIO_BIT_PCMCIA_nPCE1			( XLLP_BIT_21 )
#define XLLP_GPIO_BIT_L_DD16				( XLLP_BIT_22 )
#define XLLP_GPIO_BIT_PCMCIA_nPCE1_1        ( XLLP_BIT_22 )
#define XLLP_GPIO_BIT_L_DD17				( XLLP_BIT_23 )
#define XLLP_GPIO_BIT_PCMCIA_nPCE1_2        ( XLLP_BIT_23 )
#define XLLP_GPIO_BIT_USBHPWR0				( XLLP_BIT_24 )
#define XLLP_GPIO_BIT_SSPFRM2               ( XLLP_BIT_24 )
#define XLLP_GPIO_BIT_USBHPEN0				( XLLP_BIT_25 )
#define XLLP_GPIO_BIT_URST					( XLLP_BIT_26 )
#define XLLP_GPIO_BIT_CIF_DD4               ( XLLP_BIT_26 )
#define XLLP_GPIO_BIT_UCLK					( XLLP_BIT_27 )
#define XLLP_GPIO_BIT_CIF_DD5               ( XLLP_BIT_27 )
#define XLLP_GPIO_BIT_MMDAT0				( XLLP_BIT_28 )
#define XLLP_GPIO_BIT_MSBS					( XLLP_BIT_28 )
#define XLLP_BIT_29   ( 1u << 29 )
#define XLLP_BIT_30   ( 1u << 30 )
#define XLLP_GPIO_BIT_KP_DKIN0				( XLLP_BIT_29 )
#define XLLP_GPIO_BIT_KP_DKIN1				( XLLP_BIT_30 )
#define XLLP_GPIO_BIT_KP_MKIN6              ( XLLP_BIT_31 )

typedef struct
{
	XLLP_VUINT32_T kpControlReg;
	XLLP_VUINT32_T	rsvd1;
	XLLP_VUINT32_T kpDirectKeyReg;
	XLLP_VUINT32_T	rsvd2;
	XLLP_VUINT32_T kpRotaryEncoderCountReg;
	XLLP_VUINT32_T	rsvd3;
	XLLP_VUINT32_T kpMatrixKeyReg;
	XLLP_VUINT32_T	rsvd4;
	XLLP_VUINT32_T kpAutomaticScanReg;
	XLLP_VUINT32_T	rsvd5;
	XLLP_VUINT32_T kpAutoScanMultiKeyPress0;
	XLLP_VUINT32_T	rsvd6;
	XLLP_VUINT32_T kpAutoScanMultiKeyPress1;
	XLLP_VUINT32_T	rsvd7;
	XLLP_VUINT32_T kpAutoScanMultiKeyPress2;
	XLLP_VUINT32_T	rsvd8;
	XLLP_VUINT32_T kpAutoScanMultiKeyPress3;
	XLLP_VUINT32_T	rsvd9;
	XLLP_VUINT32_T kpKeyDebounceInterval;

} XLLP_KEYPAD_REGS;

typedef XLLP_KEYPAD_REGS BULVERDE_KEYPAD_REG;
typedef XLLP_KEYPAD_REGS *PBULVERDE_KEYPAD_REG;
// maximum number of keys that can be detected simultaneously
#define		MAX_KEYS							1

// Bit Definitions for KeyPad Interface Control Register

#define		DIRECT_KP_INTR_ENABLE				(0x1	<<	0)
#define		DIRECT_KP_ENABLE					(0x1	<<	1)
#define		ROTARY_ENCODER_0_ENABLE				(0x1	<<	2)
#define		ROTARY_ENCODER_1_ENABLE				(0x1	<<	3)
#define		ROTARY_ENCODER_ZERO_DEB				(0x1	<<	4)
#define		DIRECT_INTR_BIT						(0x1	<<	5)
#define		DIRECT_DEBOUNCE_BIT					(0x1	<<	9)
#define		MATRIX_INTR_ENABLE					(0x1	<<	11)
#define		MATRIX_KP_ENABLE					(0x1	<<	12)
#define		IGNORE_MULTIPLE_KEY_PRESS			(0x1	<<	21)
#define		MATRIX_KP_NUMBER_OF_COLUMNS			(0x7	<<  26)
#define		MATRIX_KP_NUMBER_OF_ROWS			(0x6	<<  23)
#define		MATRIX_INTR_BIT						(0x1	<<	22)
#define		AUTO_SCAN_ON_ACTIVITY				(0x1	<<	29)
#define		AUTO_SCAN_BIT						(0x1	<<	30)
#define		MAT_SCAN_LINE0						(0x1	<<	13)
#define		MAT_SCAN_LINE1						(0x1	<<	14)
#define		MAT_SCAN_LINE2						(0x1	<<	15)
#define		MAT_SCAN_LINE3						(0x1	<<	16)
#define		MAT_SCAN_LINE4						(0x1	<<	17)
#define		MAT_SCAN_LINE5						(0x1	<<	18)
#define		MAT_SCAN_LINE6						(0x1	<<	19)
#define		MAT_SCAN_LINE7						(0x1	<<	20)

// Bit Definitions for KeyPad Interface Direct Key Register

#define		DIRECT_KEY_PRESSED					(0x1	<<	31)
#define		DIRECT_KEY_IN_7						(0x1	<<	7)
#define		DIRECT_KEY_IN_6						(0x1	<<	6)
#define		DIRECT_KEY_IN_5						(0x1	<<	5)
#define		DIRECT_KEY_IN_4						(0x1	<<	4)
#define		DIRECT_KEY_IN_3						(0x1	<<	3)
#define		DIRECT_KEY_IN_2						(0x1	<<	2)
#define		ROTARY_ENC_0_SENSOR_B				(0x1	<<	1)
#define		ROTARY_ENC_0_SENSOR_A				(0x1	<<	0)
#define		DIRECT_KEY_NUMS						(0x7	<<	6)   // 8 Direct Key i.e 

// Bit Definitions for KeyPad Encoder Count Register

#define		UNDERFLOW_ROTARY_ENC_0				(0x1	<<	14)
#define		OVERFLOW_ROTARY_ENC_0				(0x1	<<	15)

// Bit Definitions for KeyPad Interface Matrix Key Register

#define		MATRIX_KEY_PRESSED					(0x1	<<	31)
#define		MATRIX_ROW_MASK						 0xFF

// Bit Definitions for KeyPad Interface Automatic Scan Register

#define		SCAN_ON_BIT							(0x1	<<	31)
#define		ROW_SELECTED_MASK					 0xF0
#define		COL_SELECTED_MASK					 0x0F
#define		MULTI_KEYS_PRESS					 0x7C000000
#define		SINGLE_KEY_PRESS					(0x1	<<	26)

// Bit Definitions for KeyPad Interface Automatic Scan Multiple Key Press Register 0,1,2,3

#define		MATRIX_KP_COL_EVEN_MASK				0x0000FF
#define		MATRIX_KP_COL_ODD_MASK				0xFF0000					

// Bit Definitions for KeyPad Interface Key Debounce Interval Register

#define		MAX_KEY_DEBOUNCE_INTERVAL			0x0000FF

// Debeounce Interval for the keyPad on MainStone is < 12 millisecs

#define		MAINSTONE_KP_DEBOUNCE_INTERVAL		0x00000C

// KeyPad controller supports 3 modes of operation. All three will be tested but the default
// mode shall be AUTO_SCAN_ON_ACT
#define		MANUAL_MATRIX_SCAN					0
#define		AUTO_SCAN_ON_ACT					1			
#define		AUTO_SCAN_BY_AS_BIT					2

#define		NOT_AVAILABLE						0
#define		AVAILABLE							1
#define		COUNT_MASK							0xFF
#define		START_VALUE							0x7F
#define		SCAN_CODE_MASK						0xFF
#define		NO_KEY								0xFF
#define		SCAN_CODE_SCROLL_UP					0xA
#define		SCAN_CODE_SCROLL_DOWN				0xB
#define		SCAN_CODE_ACTION					0xC

#define		DISABLE_DIRECT_KEYS_INTR()			v_pKeyPadRegs->kpControlReg &= ~DIRECT_KP_INTR_ENABLE 				
#define		DISABLE_MAT_KEYS_INTR()				v_pKeyPadRegs->kpControlReg &= ~MATRIX_INTR_ENABLE 
#define		EN_DIRECT_KEYS_INTR()				v_pKeyPadRegs->kpControlReg |=	DIRECT_KP_INTR_ENABLE 				
#define		EN_MAT_KEYS_INTR()					v_pKeyPadRegs->kpControlReg |=	MATRIX_INTR_ENABLE 

// Clock Manager (CLKMGR) Register Bank
//
typedef struct
{
    XLLP_VUINT32_T    cccr;         	// Core Clock Configuration register
    XLLP_VUINT32_T    cken;         	// Clock Enable register
    XLLP_VUINT32_T    oscc;         	// Oscillator Configuration register
    XLLP_VUINT32_T    ccsr;         	// Core Clock Status register
 } XLLP_CLKMGR_T, *P_XLLP_CLKMGR_T;  

//
typedef XLLP_CLKMGR_T BULVERDE_CLKMGR_REG;
typedef XLLP_CLKMGR_T *PBULVERDE_CLKMGR_REG;


//
// Clock Enable Register (CLKEN) Bits
//
#define XLLP_CLKEN_PWM0_2	    (0x1u << 0)
#define XLLP_CLKEN_PWM1_3	    (0x1u << 1)
#define XLLP_CLKEN_AC97		    (0x1u << 2)
#define XLLP_CLKEN_SSP2		    (0x1u << 3)
#define XLLP_CLKEN_SSP3		    (0x1u << 4)
#define XLLP_CLKEN_STUART	    (0x1u << 5)
#define XLLP_CLKEN_FFUART	    (0x1u << 6)
#define XLLP_CLKEN_BTUART	    (0x1u << 7)
#define XLLP_CLKEN_I2S		    (0x1u << 8)
#define XLLP_CLKEN_OST		    (0x1u << 9)
#define XLLP_CLKEN_USBHOST	    (0x1u << 10)
#define XLLP_CLKEN_USBCLIENT    (0x1u << 11)
#define XLLP_CLKEN_MMC		    (0x1u << 12)
#define XLLP_CLKEN_ICP		    (0x1u << 13)
#define XLLP_CLKEN_I2C		    (0x1u << 14)
#define XLLP_CLKEN_PWRI2C	    (0x1u << 15)
#define XLLP_CLKEN_LCD		    (0x1u << 16)
#define XLLP_CLKEN_BASEBAND     (0x1u << 17)
#define XLLP_CLKEN_USIM		    (0x1u << 18)
#define XLLP_CLKEN_KEYPAD	    (0x1u << 19)
#define XLLP_CLKEN_MEMCLOCK     (0x1u << 20)
#define XLLP_CLKEN_MEMSTICK	    (0x1u << 21)
#define XLLP_CLKEN_MEMC		    (0x1u << 22)
#define XLLP_CLKEN_SSP1		    (0x1u << 23)
#define XLLP_CLKEN_CAMERA	    (0x1u << 24) // Camera Capture interface
#define XLLP_CLKEN_TPM		    (0x1u << 25) // Trusted Platform Module (Caddo)


//
// CLKEN register reserved and valid bits
//
#define XLLP_CLKEN_RESERVED_BITS  0xfc000000u
#define XLLP_CLKEN_MASK (~(XLLP_CLKEN_RESERVED_BITS))

//
// OST Register Definitions
//
typedef struct
{
    XLLP_VUINT32_T    osmr0;         	//OS timer match register 0
    XLLP_VUINT32_T    osmr1;         	//OS timer match register 1
    XLLP_VUINT32_T    osmr2;         	//OS timer match register 2
    XLLP_VUINT32_T    osmr3;          	//OS timer match register 3
    XLLP_VUINT32_T    oscr0;            //OS timer counter register 0(compatible)
    XLLP_VUINT32_T    ossr;             //OS timer status register
    XLLP_VUINT32_T    ower;          	//OS timer watchdog enable register
    XLLP_VUINT32_T    oier;           	//OS timer interrupt enable register
    XLLP_VUINT32_T    osnr;           	//OS timer snapshot register
    XLLP_VUINT32_T    reserved1[7];
    XLLP_VUINT32_T    oscr4;		//OS timer counter register 4
    XLLP_VUINT32_T    oscr5;		//OS timer counter register  5
    XLLP_VUINT32_T    oscr6;		//OS timer counter register  6
    XLLP_VUINT32_T    oscr7;		//OS timer counter register  7
    XLLP_VUINT32_T    oscr8;		//OS timer counter register  8
    XLLP_VUINT32_T    oscr9;		//OS timer counter register  9
    XLLP_VUINT32_T    oscr10;		//OS timer counter register  10
    XLLP_VUINT32_T    oscr11;		//OS timer counter register  11
    XLLP_VUINT32_T    reserved2[8];
    XLLP_VUINT32_T    osmr4;		//OS timer match register 4
    XLLP_VUINT32_T    osmr5;		//OS timer match register 5
    XLLP_VUINT32_T    osmr6;		//OS timer match register 6
    XLLP_VUINT32_T    osmr7;		//OS timer match register 7
    XLLP_VUINT32_T    osmr8;		//OS timer match register 8
    XLLP_VUINT32_T    osmr9;		//OS timer match register 9
    XLLP_VUINT32_T    osmr10;		//OS timer match register 10
    XLLP_VUINT32_T    osmr11;		//OS timer match register 11
    XLLP_VUINT32_T    reserved3[8];
    XLLP_VUINT32_T    omcr4;		//OS timer match control register 4
    XLLP_VUINT32_T    omcr5;		//OS timer match control register 5
    XLLP_VUINT32_T    omcr6;		//OS timer match control register 6
    XLLP_VUINT32_T    omcr7;		//OS timer match control register 7
    XLLP_VUINT32_T    omcr8;		//OS timer match control register 8
    XLLP_VUINT32_T    omcr9;		//OS timer match control register 9
    XLLP_VUINT32_T    omcr10;		//OS timer match control register 10
    XLLP_VUINT32_T    omcr11;		//OS timer match control register 11
} XLLP_OST_T, *P_XLLP_OST_T;


//
// Enumeration for compatible OST match registers
//
typedef enum 
{
    MatchReg0 = 0,
    MatchReg1,
    MatchReg2,
    MatchReg3,
    MatchReg4,
    MatchReg5,
    MatchReg6,
    MatchReg7,
    MatchReg8,
    MatchReg9,
    MatchReg10,
    MatchReg11
}XLLP_OST_MATCHREG;

//
// OST Bit Definitions
//

//
// OST Tick constants
//
#define XLLP_OST_TICKS_MS    3250          // 1ms in ticks (3.25x10^6tick/sec * 1/1000sec/msec)
#define XLLP_OST_TICKS_US    3             // 1usec in ticks (3.25x10^6tick/sec * 1/1000000sec/usec)

//
// OSSR Bits
//
#define XLLP_OSSR_M0			(0x1 << 0)
#define XLLP_OSSR_M1			(0x1 << 1)
#define XLLP_OSSR_M2			(0x1 << 2)
#define XLLP_OSSR_M3			(0x1 << 3)
#define XLLP_OSSR_M4			(0x1 << 4)
#define XLLP_OSSR_M5			(0x1 << 5)
#define XLLP_OSSR_M6			(0x1 << 6)
#define XLLP_OSSR_M7			(0x1 << 7)
#define XLLP_OSSR_M8			(0x1 << 8)
#define XLLP_OSSR_M9			(0x1 << 9)
#define XLLP_OSSR_M10			(0x1 << 10)
#define XLLP_OSSR_M11			(0x1 << 11)

#define XLLP_OSSR_RESERVED_BITS (0xFFFFF000)



typedef XLLP_OST_T BULVERDE_OST_REG;
typedef XLLP_OST_T *PBULVERDE_OST_REG;    
