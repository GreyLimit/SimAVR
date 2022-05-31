//
//	Timer.cpp
//	=========
//
//	Static table elements of the timer module.
//

#include "Timer.h"

//
//	Pin Op descriptions.
//
static const char pin_op_b00[] = "Disconnect pin OC%d%c";
static const char pin_op_b01[] = "Toggle pin OC%d%c";
static const char pin_op_b10[] = "Clear pin OC%d%c";
static const char pin_op_b11[] = "Set pin OC%d%c";
//
Timer::ComOp Timer::_pin_op_mode[ Timer::pin_op_modes ] {
	{	PinOp_None,	pin_op_b00	},
	{	PinOp_Toggle,	pin_op_b01	},
	{	PinOp_Clear,	pin_op_b10	},
	{	PinOp_Set,	pin_op_b11	}
};
		
//
//	8 bit WGM table
//	---------------
//
//	WGMn2	WGMn1	 WGMn0	Timer/Counter Mode		TOP	Update of	TOVn Flag
//				of Operation				OCRnx at	Set on
//
//	0	0	0	Normal				0xFF	Immediate	MAX
//	0	0	1	PWM, Phase Correct		0xFF	TOP		BOTTOM
//	0	1	0	CTC				OCRA	Immediate	MAX
//	0	1	1	Fast PWM			0xFF	BOTTOM		MAX
//	1	0	0	(Reserved)			–	–		–
//	1	0	1	PWM, Phase Correct		OCRA	TOP		BOTTOM
//	1	1	0	(Reserved)			–	–		–
//	1	1	1	Fast PWM			OCRA	BOTTOM		TOP
//
//	Waveform description formatting data.
//
static const char waveform_b000[] = "WGM%d Normal";
static const char waveform_b001[] = "WGM%d Phase Correct PWM";
static const char waveform_b010[] = "WGM%d CTC (OCRA)";
static const char waveform_b011[] = "WGM%d Fast PWM";
static const char waveform_b100[] = "WGM%d (Reserved)";
static const char waveform_b101[] = "WGM%d Phase Correct PWM (OCRA)";
static const char waveform_b110[] = "WGM%d (Reserved)";
static const char waveform_b111[] = "WGM%d Fast PWM (OCRA)";
//
//	16 bit WGM table
//	----------------
//
//	WGMn3	WGMn2	WGMn1	 WGMn0	Timer/Counter Mode		TOP	Update of	TOVn Flag
//		(CTCn)	(PWMn1)	(PWMn0)	of Operation				OCRnx at	Set on
//
//	0	0	0	0	Normal				0xFFFF	Immediate	MAX
//	0	0	0	1	PWM, Phase Correct, 8-bit	0x00FF	TOP		BOTTOM
//	0	0	1	0	PWM, Phase Correct, 9-bit 	0x01FF	TOP		BOTTOM
//	0	0	1	1	PWM, Phase Correct, 10-bit 	0x03FF	TOP		BOTTOM
//	0	1	0	0	CTC				OCRA	Immediate	MAX
//	0	1	0	1	Fast PWM, 8-bit			0x00FF	BOTTOM		TOP
//	0	1	1	0	Fast PWM, 9-bit			0x01FF	BOTTOM		TOP
//	0	1	1	1	Fast PWM, 10-bit		0x03FF	BOTTOM		TOP
//
//	1	0	0	0	PWM, Phase and Freq Correct	ICR	BOTTOM		BOTTOM
//	1	0	0	1	PWM, Phase and Freq Correct	OCRA	BOTTOM		BOTTOM
//	1	0	1	0	PWM, Phase Correct		ICR	TOP		BOTTOM
//	1	0	1	1	PWM, Phase Correct		OCRA	TOP		BOTTOM
//	1	1	0	0	CTC				ICR	Immediate	MAX
//	1	1	0	1	(Reserved)			–	–		–
//	1	1	1	0	Fast PWM			ICR	BOTTOM		TOP
//	1	1	1	1	Fast PWM			OCRA	BOTTOM		TOP
//
static const char waveform_b0000[] = "WGM%d Normal";
static const char waveform_b0001[] = "WGM%d PWM, Phase Correct, 8-bit";
static const char waveform_b0010[] = "WGM%d PWM, Phase Correct, 9-bit";
static const char waveform_b0011[] = "WGM%d PWM, Phase Correct, 10-bit";
static const char waveform_b0100[] = "WGM%d CTC (OCRA)";
static const char waveform_b0101[] = "WGM%d Fast PWM, 8-bit";
static const char waveform_b0110[] = "WGM%d Fast PWM, 9-bit";
static const char waveform_b0111[] = "WGM%d Fast PWM, 10-bit";
static const char waveform_b1000[] = "WGM%d PWM, Phase and Frequency Correct (ICR)";
static const char waveform_b1001[] = "WGM%d PWM, Phase and Frequency Correct (OCRA)";
static const char waveform_b1010[] = "WGM%d PWM, Phase Correct (ICR)";
static const char waveform_b1011[] = "WGM%d PWM, Phase Correct (OCRA)";
static const char waveform_b1100[] = "WGM%d CTC (ICR)";
static const char waveform_b1101[] = "WGM%d (Reserved)";
static const char waveform_b1110[] = "WGM%d Fast PWM (ICR)";
static const char waveform_b1111[] = "WGM%d Fast PWM (OCRA)";
//

//
//	The combined 8 and 16 bit waveform table
//
Timer::WaveForm Timer::_waveform[ Timer::waveform_modes ] = {
	//
	//	eight	mode	maximum		loop_on		set_ocr		set_tov		up & down	desc
	//	-----	----	-------		-------		-------		-------		---------	----
	{	true,	0,	0xFF,		On_Fixed,	At_Imm,		At_Max,		false,		waveform_b000	}, // Normal
	{	true,	1,	0xFF,		On_Fixed,	At_Top,		At_Bottom,	true,		waveform_b001	}, // Phase Correct PWM
	{	true,	2,	0xFF,		On_OCRA,	At_Imm,		At_Max,		false,		waveform_b010	}, // CTC
	{	true,	3,	0xFF,		On_Fixed,	At_Bottom,	At_Max,		false,		waveform_b011	}, // Fast PWM
	{	true,	4,	0xFF,		On_Fixed,	At_Never,	At_Never,	false,		waveform_b100	}, // (Reserved)
	{	true,	5,	0xFF,		On_OCRA,	At_Top,		At_Bottom,	true,		waveform_b101	}, // Phase Correct PWM
	{	true,	6,	0xFF,		On_Never,	At_Never,	At_Never,	false,		waveform_b110	}, // (Reserved)
	{	true,	7,	0xFF,		On_OCRA,	At_Bottom,	At_Top,		false,		waveform_b111	}, // Fast PWM
	//
	{	false,	0,	0xFFFF,		On_Fixed,	At_Imm,		At_Max,		false,		waveform_b0000	}, // Normal
	{	false,	1,	0x00FF,		On_Fixed,	At_Top,		At_Bottom,	true,		waveform_b0001	}, // Phase Correct PWM 8-bit
	{	false,	2,	0x01FF,		On_Fixed,	At_Top,		At_Bottom,	true,		waveform_b0010	}, // Phase Correct PWM 9-bit
	{	false,	3,	0x03FF,		On_Fixed,	At_Top,		At_Bottom,	true,		waveform_b0011	}, // Phase Correct PWM 10-bit
	{	false,	4,	0xFFFF,		On_OCRA,	At_Imm,		At_Max,		false,		waveform_b0100	}, // CTC (OCRA)
	{	false,	5,	0x00FF,		On_Fixed,	At_Bottom,	At_Top,		false,		waveform_b0101	}, // Fast PWM 8-bit
	{	false,	6,	0x01FF,		On_Fixed,	At_Bottom,	At_Top,		false,		waveform_b0110	}, // Fast PWM 9-bit
	{	false,	7,	0x03FF,		On_Fixed,	At_Bottom,	At_Top,		false,		waveform_b0111	}, // Fast PWM 10-bit
	//
	{	false,	8,	0xFFFF,		On_ICR,		At_Bottom,	At_Bottom,	true,		waveform_b1000	}, // Phase and Frequency Correct PWM (ICR)
	{	false,	9,	0xFFFF,		On_OCRA,	At_Bottom,	At_Bottom,	true,		waveform_b1001	}, // Phase and Frequency Correct PWM (OCRA)
	{	false,	10,	0xFFFF,		On_ICR,		At_Top,		At_Bottom,	true,		waveform_b1010	}, // Phase Correct PWM (ICR)
	{	false,	11,	0xFFFF,		On_OCRA,	At_Top,		At_Bottom,	true,		waveform_b1011	}, // Phase Correct PWM (OCRA)
	{	false,	12,	0xFFFF,		On_ICR,		At_Imm,		At_Max,		false,		waveform_b1100	}, // CTC (ICR)
	{	false,	13,	0xFFFF,		On_Never,	At_Never,	At_Never,	false,		waveform_b1101	}, // (Reserved)
	{	false,	14,	0xFFFF,		On_ICR,		At_Bottom,	At_Top,		false,		waveform_b1110	}, // Fast PWM (ICR)
	{	false,	15,	0xFFFF,		On_OCRA,	At_Bottom,	At_Top,		false,		waveform_b1111	}  // Fast PWM (OCRA)
};

//
//	Define the data table that encode all the clock modes.
//
//	CSn2	CSn1	CSn0	Description
//	0	0	0	No clock source (Timer/Counter stopped).
//	0	0	1	clk I/O /1 (No prescaling)
//	0	1	0	clk I/O /8 (From prescaler)
//	0	1	1	clk I/O /64 (From prescaler)
//	1	0	0 	clk I/O /256 (From prescaler)
//	1	0	1	clk I/O /1024 (From prescaler)
//	1	1	0	External clock source on T1 pin. Clock on falling edge.
//	1	1	1	External clock source on T1 pin. Clock on rising edge.
//
static const char clock_mode_b000[] = "CS%d No clock source";
static const char clock_mode_b001[] = "CS%d System clock /1";
static const char clock_mode_b010[] = "CS%d System clock /8";
static const char clock_mode_b011[] = "CS%d System clock /64";
static const char clock_mode_b100[] = "CS%d System clock /256";
static const char clock_mode_b101[] = "CS%d System clock /1024";
static const char clock_mode_b110[] = "CS%d Falling external pin";
static const char clock_mode_b111[] = "CS%d Rising external pin";
//
Timer::ClockMode Timer::_clock_mode[ Timer::clock_modes ] = {
	//
	//	running		external	rising_edge	prescaler	desc
	//	-------		--------	-----------	---------	----
	//
	{	false,		false,		false,		1,		clock_mode_b000	},
	{	true,		false,		false,		1,		clock_mode_b001	},
	{	true,		false,		false,		8,		clock_mode_b010	},
	{	true,		false,		false,		64,		clock_mode_b011	},
	{	true,		false,		false,		256,		clock_mode_b100	},
	{	true,		false,		false,		1024,		clock_mode_b101	},
	{	true,		true,		false,		1,		clock_mode_b110	},
	{	true,		true,		true,		1,		clock_mode_b111	}
};

//
//	EOF
//
