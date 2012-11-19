/*
    temperature.c: Temperature driver

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2012 Matthew Excell <matt@excellclan.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
    Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/


    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

      Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

      Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the
      distribution.

      Neither the name of Texas Instruments Incorporated nor the names of
      its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* driver */
#include "temperature.h"
#include "ports.h"
#include "display.h"
#include "adc12.h"
#include "timer.h"

/* The code below is optimized to this value, DO NOT CHANGE */
#define TEMPORAL_FILTER_WINDOW 4

static uint8_t adcresult[TEMPORAL_FILTER_WINDOW];
static uint8_t adcresult_idx = 0;

void temperature_init(void)
{
	temperature.value = adc12_single_conversion(REFVSEL_0,
						ADC12SHT0_8, ADC12INCH_10);
	temperature.offset = CONFIG_TEMPERATURE_OFFSET;

	adcresult[0] = temperature.value;
	adcresult[1] = temperature.value;
	adcresult[2] = temperature.value;
	adcresult[3] = temperature.value;
}


void temperature_measurement(void)
{
<<<<<<< HEAD
	uint16_t adc_result;
	volatile int32_t temperature;

	// Convert internal temperature diode voltage
	//adc_result = 0;
	adc_result = adc12_single_conversion(REFVSEL_0, ADC12SHT0_8, ADC12INCH_10);

	// Convert ADC value to "xx.x �C"
	// Temperature in Celsius
	// ((A10/4096*1500mV) - 680mV)*(1/2.25mV) = (A10/4096*667) - 302
	// = (A10 - 1855) * (667 / 4096)
	temperature = (((int32_t)((int32_t)adc_result - 1855)) * 667 * 10) / 4096;

	// Add base offset
#ifdef CONFIG_TEMPERATURE_DEFAULT_OFFSET
	temperature += CONFIG_TEMPERATURE_DEFAULT_OFFSET;
#endif

	// Add temperature offset - we do this at display - makes for easier editing
	temperature += sTemp.offset;

	// Limit min/max temperature to +/- 50 �C
	//if (temperature > 500) temperature = 500;
	//if (temperature < -500) temperature = -500;


	// Store measured temperature
	/*
	if (filter) {
		// Change temperature in 0.1� steps towards measured value
		if (temperature > sTemp.degrees)		sTemp.degrees += 1;
		else if (temperature < sTemp.degrees)	sTemp.degrees -= 1;
	} else {
		// Override filter
		sTemp.degrees = (int16_t)temperature;
	}
	*/
	
	sTemp.degrees = (int16_t)temperature;
=======
	/* Convert internal temperature diode voltage */
	adcresult[adcresult_idx++] = adc12_single_conversion(REFVSEL_0,
						ADC12SHT0_8, ADC12INCH_10);
	if (adcresult_idx == TEMPORAL_FILTER_WINDOW)
		adcresult_idx = 0;

	/* Calculate temporal mean value */
	temperature.value = (temperature.value & 0xff00)
		| (((uint16_t)adcresult[0] + (uint16_t)adcresult[1]
		+ (uint16_t)adcresult[2] + (uint16_t)adcresult[3]) >> 2);
>>>>>>> e029f212bac889dd49284466274cbb85f4f99130
}


void temperature_get_C(int16_t *temp)
{
	/* from page 67, slas554f.pdf:
	((A10/4096*1500mV) - 680mV)*(1/2.25mV)
	   = (A10/4096*667) - 302
	   = (A10 - 1855) * (667 / 4096) */
	*temp = (((int32_t)temperature.value + temperature.offset - 1855)
		* 667 * 10) / 4096;
}

void temperature_get_F(int16_t *temp)
{
	/* from page 67 (slas554f.pdf):
	offset:
	  680mV is 0C with 2.25mV/C
	  if 0F is -17.78C then 0F = (680-17.78*2.25) = 640mV
	scale:
	  if 2.25mV/C then 2.25*(5/9)=1.25mV/F

	((A10/4096*1500mV) - 640mV)*(1/1.25mV) =
	  = (A10/4096*1200) - 512
	  = (A10 - 1748) * (1200 / 4096) */
	*temp = (((int32_t)temperature.value + temperature.offset - 1748)
		* 1200 * 10) / 4096;
}

