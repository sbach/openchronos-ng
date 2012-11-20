/*
    TODO altitude.c: Altitude driver

    Copyright (C) 2012 Stanislas Bach <stanislasbach@gmail.com>

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


#include "altitude.h"

#include "ports.h"
#include "display.h"
#include "timer.h"
#include "vti_ps.h"

// Flag from vti_ps.
extern uint8_t ps_ok;

void altitude_init(void)
{
	if (ps_ok)
	{
		// Initialise pressure table
		init_pressure_table();

		// Do single conversion
		start_altitude_measurement();
		stop_altitude_measurement();

		// Apply calibration offset and recalculate pressure table
		if (altitude.altitude_offset != 0)
		{
			altitude.altitude += altitude.altitude_offset;
			update_pressure_table(altitude.altitude, altitude.pressure, altitude.temperature);
		}
	}
}

void start_altitude_measurement(void)
{
	// Return if pressure sensor was not initialised properly
	if (!ps_ok) return;
	
	// Enable DRDY IRQ on rising edge
	PS_INT_IFG &= ~PS_INT_PIN;
	PS_INT_IE |= PS_INT_PIN;

	// Start pressure sensor
	ps_start();

	// FIXME Need stuff below ?
		
	// Get updated altitude
	//while ((PS_INT_IN & PS_INT_PIN) == 0);

	//altitude_measurement();
}

void stop_altitude_measurement(void)
{
	// Return if pressure sensor was not initialised properly
	if (!ps_ok) return;

	// Stop pressure sensor
	ps_stop();

	// Disable DRDY IRQ
	PS_INT_IE  &= ~PS_INT_PIN;
	PS_INT_IFG &= ~PS_INT_PIN;
}

void altitude_measurement(void)
{
	// FIXME Needed ?
	// If sensor is not ready, skip data read
	//if ((PS_INT_IN & PS_INT_PIN) == 0)	return;
	
	// Get temperature (format is *10?K) from sensor
	altitude.temperature = ps_get_temp();

	// Get pressure (format is 1Pa) from sensor
	altitude.pressure = ps_get_pa();
	
#ifdef CONFIG_FIXEDPOINT_MATH
	altitude.altitude = conv_pa_to_altitude(altitude.pressure, altitude.temperature);
#else
	altitude.altitude = conv_pa_to_meter(altitude.pressure, altitude.temperature);
#endif
}

