/*
    TODO altitude.c: altitude display module

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
#include <openchronos.h>

// Driver
#include <drivers/display.h>
#include <drivers/vti_ps.h>


// *************************************************************************************************
// Defines section


// *************************************************************************************************
// Global Variable section

struct {

	// Pressure (Pa)
	uint32_t pressure;

	// Temperature (°K)
	uint16_t temperature;

	// Altitude (m)
	int16_t	altitude;

	// Altitude offset stored during calibration
	int16_t	altitude_offset;

	// Timeout
	uint16_t timeout;
	
} altitude;

// Flag from vti_ps.
extern uint8_t ps_ok;


// *************************************************************************************************
// Extern section

void start_altitude_measurement(void)
{
	// Return if pressure sensor was not initialised properly
	if (!ps_ok) return;
	
	// Start pressure sensor
	ps_start();
}

void stop_altitude_measurement(void)
{
	// Return if pressure sensor was not initialised properly
	if (!ps_ok) return;

	// Stop pressure sensor
	ps_stop();
}

void altitude_measurement(void)
{
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

void altitude_init(void)
{
	if (!ps_ok) return;
	
	// Initialise pressure table
	init_pressure_table();
}

void altitude_reconfigure()
{
	// TODO If altitude_offset changed then update the table
	
	// Apply calibration offset and recalculate pressure table
	/*
	if (altitude.altitude_offset != 0)
	{
		altitude.altitude += altitude.altitude_offset;
		update_pressure_table(altitude.altitude, altitude.pressure, altitude.temperature);
	}
	*/
}


// *************************************************************************************************
// @fn          display_altitude
// @brief       Common display routine
// @return      none
// *************************************************************************************************
void display_altitude()
{
	display_chars(0, LCD_SEG_L1_3_0, _sprintf("%3s", altitude.altitude), SEG_SET);
}


// *************************************************************************************************
// @fn          ps_event
// @brief      	Event driven routine for the pressure sensor
// @return      none
// *************************************************************************************************
static void ps_event(enum sys_message msg)
{
	// Get the altitude from the sensor
	altitude_measurement();
	
	// Reconfigure the sensor if needed
	altitude_reconfigure();
	
	// Display new stuff on the screen
	display_altitude();
}


// *************************************************************************************************
// @fn          altitude_activate
// @brief       Altitude screen activation. Display defaul stuff, register the mesuring loop.
// @return      none
// *************************************************************************************************
static void altitude_activate()
{	
	// Display the title of this module at the bottom
	display_chars(0, LCD_SEG_L2_3_0, "ALTI", SEG_SET);

	// Display something while a measure is not performed
	display_chars(0, LCD_SEG_L1_2_0, " - ", SEG_ON);
	
	// Register an event
	sys_messagebus_register(&ps_event, SYS_MSG_PS_INT);

	// Init the sensor
	altitude_init();
			
	// Start the sensor
	start_altitude_measurement();
}


// *************************************************************************************************
// @fn          altitude_deactivate
// @brief       Altitude screen desactivation. Clear the screen, reset the events & values.
// @return      none
// *************************************************************************************************
static void altitude_deactivate()
{
	// Unregister the event
	sys_messagebus_unregister(&ps_event);
	
	// Stop the sensor
	stop_altitude_measurement();
	
	// Cleanup screens
	display_clear(0, 1);
}


// *************************************************************************************************
// @fn          mod_altiude_init
// @brief       Init the module. Sets default values, register menu entry.
// @return      none
// *************************************************************************************************
void mod_altitude_init(void)
{			
	menu_add_entry(" ALTI",
		NULL, NULL, NULL,
		NULL, NULL, NULL,
		&altitude_activate,
		&altitude_deactivate);
}
