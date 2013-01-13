/**
	@file	drivers/adc12.c
	@brief	ADC12 functions
 */

// *************************************************************************************************
//
//	Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
//
//
//	  Redistribution and use in source and binary forms, with or without
//	  modification, are permitted provided that the following conditions
//	  are met:
//
//	    Redistributions of source code must retain the above copyright
//	    notice, this list of conditions and the following disclaimer.
//
//	    Redistributions in binary form must reproduce the above copyright
//	    notice, this list of conditions and the following disclaimer in the
//	    documentation and/or other materials provided with the
//	    distribution.
//
//	    Neither the name of Texas Instruments Incorporated nor the names of
//	    its contributors may be used to endorse or promote products derived
//	    from this software without specific prior written permission.
//
//	  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//	  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//	  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//	  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//	  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//	  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//	  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//	  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//	  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//	  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *************************************************************************************************


// *************************************************************************************************
// Include section

// System
#include <core/openchronos.h>

// Driver
#include "adc12.h"
#include "timer.h"


// *************************************************************************************************
// Defines section


// *************************************************************************************************
// Global Variable section

uint16_t adc12_result;
uint8_t  adc12_data_ready;


// *************************************************************************************************
// Extern section


//* ************************************************************************************************
/// @brief	Init ADC12. Do single conversion. Turn off ADC12.
//* ************************************************************************************************
uint16_t adc12_single_conversion(uint16_t ref, uint16_t sht, uint16_t channel)
{
	// Initialize the shared reference module
	REFCTL0 |= REFMSTR + ref + REFON;    		// Enable internal reference (1.5V or 2.5V)

	// Initialize ADC12_A
	ADC12CTL0 = sht + ADC12ON;					// Set sample time
	ADC12CTL1 = ADC12SHP;                     	// Enable sample timer
	ADC12MCTL0 = ADC12SREF_1 + channel;  		// ADC input channel
	ADC12IE = 0x001;                          	// ADC_IFG upon conv result-ADCMEMO

	// Wait 2 ticks (66us) to allow internal reference to settle
	timer0_delay(66,LPM3_bits);

	// Start ADC12
	ADC12CTL0 |= ADC12ENC;

	// Clear data ready flag
	adc12_data_ready = 0;

	// Sampling and conversion start
	ADC12CTL0 |= ADC12SC;

	// Wait until ADC12 has finished
	timer0_delay(170, LPM3_bits);

	uint8_t loops = 0;

	//We were going away and the watchdog was tripping - this should reduce the instances of that.
	while (!adc12_data_ready && loops++ < 30) {
		timer0_delay(66, LPM3_bits);
	}

	// Shut down ADC12
	ADC12CTL0 &= ~(ADC12ENC | ADC12SC | sht);
	ADC12CTL0 &= ~ADC12ON;

	// Shut down reference voltage
	REFCTL0 &= ~(REFMSTR + ref + REFON);

	ADC12IE = 0;

	// Return ADC result
	return (adc12_result);
}

//* ************************************************************************************************
/// Pfs wrapped the following to accommodate mspgcc compiler.
//* ************************************************************************************************
#ifdef __GNUC__
__attribute__((interrupt(ADC12_VECTOR)))
#else
#pragma vector = ADC12_VECTOR
__interrupt
#endif

//* ************************************************************************************************
/// @brief		Store ADC12 conversion result. Set flag to indicate data ready.
/// @return		none
//* ************************************************************************************************
void ADC12ISR(void)
{
	switch (__even_in_range(ADC12IV, 34))
	{
		case  0:						// Vector  0:  No interrupt
			break;

		case  2:						// Vector  2:  ADC overflow
			break;

		case  4:						// Vector  4:  ADC timing overflow
			break;

		case  6:						// Vector  6:  ADC12IFG0
			
			// Move results, IFG is cleared
			adc12_result = ADC12MEM0;
			adc12_data_ready = 1;

			// Exit active CPU
			_BIC_SR_IRQ(LPM3_bits);
			
			break;

		case  8:						// Vector  8:  ADC12IFG1
			break;
			
		case 10:						// Vector 10:  ADC12IFG2
			break;

		case 12:						// Vector 12:  ADC12IFG3
			break;

		case 14:						// Vector 14:  ADC12IFG4
			break;

		case 16:						// Vector 16:  ADC12IFG5
			break;

		case 18:						// Vector 18:  ADC12IFG6
			break;

		case 20:						// Vector 20:  ADC12IFG7
			break;

		case 22:						// Vector 22:  ADC12IFG8
			break;

		case 24:						// Vector 24:  ADC12IFG9
			break;

		case 26:						// Vector 26:  ADC12IFG10
			break;

		case 28:						// Vector 28:  ADC12IFG11
			break;

		case 30:						// Vector 30:  ADC12IFG12
			break;

		case 32:						// Vector 32:  ADC12IFG13
			break;

		case 34:						// Vector 34:  ADC12IFG14
			break;

		default:
			break;
	}
}