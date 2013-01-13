/*
    altitude.c: altitude display module

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
#include <core/openchronos.h>

// Driver
#include <drivers/display.h>
#include <drivers/vti_ps.h>


// *************************************************************************************************
// Defines section

#define ALTITUDE_METRIC_M		0
#define ALTITUDE_METRIC_FT		1
#define ALTITUDE_METRIC_BOTH	2

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
	
} altitude;

// Flag from vti_ps.
extern uint8_t ps_ok;

uint8_t need_reconfigure;

#if CONFIG_MOD_ALTITUDE_METRIC == ALTITUDE_METRIC_FT
uint8_t display_altitude_metric = 1;
#else
uint8_t display_altitude_metric = 0;	// Meter by default for both mode
#endif


// *************************************************************************************************
// Extern section

void altitude_measurement(void)
{
	// Get temperature (format is *10?K) from sensor
	altitude.temperature = ps_get_temp();

	// Get pressure (format is 1Pa) from sensor
	altitude.pressure = ps_get_pa();
	
#ifdef CONFIG_FIXEDPOINT_MATH
	altitude.altitude = conv_pa_to_altitude(altitude.pressure, altitude.temperature);
	// FIXME Not working properly when using update_pa_table()
#else
	altitude.altitude = conv_pa_to_meter(altitude.pressure, altitude.temperature);
#endif

}

void altitude_reconfigure()
{
	// Stop the sensor
	ps_stop();
		
	// Apply calibration offset and recalculate pressure table
	
	int16_t alt = altitude.altitude;
	alt += altitude.altitude_offset;

	update_pressure_table(alt, altitude.pressure, altitude.temperature);
	
	//Restart the sensor when calibration is done
	ps_start();
}

#if CONFIG_MOD_ALTITUDE_METRIC != ALTITUDE_METRIC_M
// *************************************************************************************************
// @fn          conv_m_to_ft
// @brief       Convert meters to feet
// @param       u16 m		Meters
// @return      u16		Feet
// *************************************************************************************************
int16_t convert_m_to_ft(int16_t m)
{
	return (((int16_t)328*m)/100);
}
#endif

// *************************************************************************************************
// @fn          display_alt_symbols
// @brief       
// @return      none
// *************************************************************************************************
void display_alt_symbols(int8_t disp)
{
	display_symbol(0, LCD_UNIT_L1_FT, 
		(disp && display_altitude_metric == ALTITUDE_METRIC_FT)  ? SEG_ON : SEG_OFF
	);
	
	display_symbol(0, LCD_UNIT_L1_M,
		(disp && display_altitude_metric == ALTITUDE_METRIC_M) ? SEG_ON : SEG_OFF
	);
}


// *************************************************************************************************
// @fn          display_altitude
// @brief       Common display routine
// @return      none
// *************************************************************************************************
void display_altitude()
{
	int16_t alt = altitude.altitude;
		
#if CONFIG_MOD_ALTITUDE_METRIC == ALTITUDE_METRIC_FT
	// Convert from meters to feet
	alt = convert_m_to_ft(alt);
#endif

#if CONFIG_MOD_ALTITUDE_METRIC == ALTITUDE_METRIC_BOTH
	if(display_altitude_metric == ALTITUDE_METRIC_FT)
	{
 		// Convert from meters to feet
		alt = convert_m_to_ft(alt);
	}
#endif

	display_chars(0, LCD_SEG_L1_3_0, _sprintf("%3s", alt), SEG_SET);
}


// BEGIN - EDIT_MODE_SECTION ***********************************************************************

static void edit_offset_sel(void)
{
	// Display "OFST" as title
	display_chars(1, LCD_SEG_L2_3_0, "OFST", SEG_SET);
	
	// Display the current offset
	display_chars(1, LCD_SEG_L1_3_0, _sprintf("%3s", altitude.altitude + altitude.altitude_offset), SEG_SET); //FIXME
}

static void edit_offset_dsel(void)
{
	// Nothing to do here
}

static void edit_offset_set(int8_t step)
{
	// Edit the offset by 10 each time (faster)
	altitude.altitude_offset += step * 10;
	
	// Display the current offset
	display_chars(1, LCD_SEG_L1_3_0, _sprintf("%3s", altitude.altitude + altitude.altitude_offset), SEG_SET); //FIXME

}

// *************************************************************************************************

#if CONFIG_MOD_ALTITUDE_METRIC == ALTITUDE_METRIC_BOTH

static void edit_metric_sel(void)
{
	// Clear symbols
	display_alt_symbols(0);
	
	// Display "MTRC" as title
	display_chars(1, LCD_SEG_L2_3_0, "MTRC", SEG_SET);
	
	// Display the current metric system used
	display_chars(1, LCD_SEG_L1_3_0, (display_altitude_metric == 0 ? "METR " : "FEET"), SEG_SET);
}
static void edit_metric_dsel(void)
{
	// Show symbols
	display_alt_symbols(1);
}
static void edit_metric_set(int8_t step)
{
	// Switch from M to FT
	display_altitude_metric = (step == 1) ? 0 : 1;

	// Display the current metric system used
	display_chars(1, LCD_SEG_L1_3_0, (display_altitude_metric == 0 ? "METR " : "FEET"), SEG_SET);
}

#endif

// *************************************************************************************************
// @fn          edit_save
// @brief       Stuff to do when we exit the edit mode
// @return      none
// *************************************************************************************************
static void edit_save()
{
	altitude_reconfigure();
	
	// Disable blinking for the offset value
	display_chars(1, LCD_SEG_L1_3_0, NULL, BLINK_OFF);
	
	// Revert to the default screen (0)
	lcd_screen_activate(0);
}

// END - EDIT_MODE_SECTION *************************************************************************


// *************************************************************************************************
// @fn          ps_event
// @brief      	Event driven routine for the pressure sensor
// @return      none
// *************************************************************************************************
static void ps_event(enum sys_message msg)
{
	// Get the altitude from the sensor
	altitude_measurement();
	
	// altitude_reconfigure();
	
	// Display new stuff on the screen
	display_altitude();
}


// *************************************************************************************************
// Edit menu for this module
// *************************************************************************************************
static struct menu_editmode_item edit_items[] = {
	{&edit_offset_sel, &edit_offset_dsel, &edit_offset_set},
#if CONFIG_MOD_ALTITUDE_METRIC == ALTITUDE_METRIC_BOTH
	{&edit_metric_sel, &edit_metric_dsel, &edit_metric_set},
#endif
	{ NULL },
};

// *************************************************************************************************
// @fn          mod_temperature_init
// @brief       Init the module. Sets default values, register menu entry.
// @return      none
// *************************************************************************************************
static void altitude_edit(void)
{	
	// Switch to the edit screen (1)
	lcd_screen_activate(1);
	
	// Enable blinking for the offset value
	display_chars(1, LCD_SEG_L1_3_0, NULL, BLINK_ON);
	
	// We go into edit mode
	menu_editmode_start(&edit_save, edit_items);
}


// *************************************************************************************************
// @fn          altitude_activate
// @brief       Altitude screen activation. Display defaul stuff, register the mesuring loop.
// @return      none
// *************************************************************************************************
static void altitude_activate()
{
	//need_reconfigure = 1;

	// Create two screens, the first is always the active one
	lcd_screens_create(2);
	
	// Display the title of this module at the bottom
	display_chars(0, LCD_SEG_L2_3_0, "ALTI", SEG_SET);

	// Is the sensor working ?
	if(ps_ok)
	{
		// Display something while a measure is not performed
		display_chars(0, LCD_SEG_L1_2_0, " - ", SEG_ON);
		
		display_alt_symbols(1);
		
		// Register an event
		sys_messagebus_register(&ps_event, SYS_MSG_PS_INT);
	
		// Init the sensor
		init_pressure_table();
				
		// Start the sensor
		ps_start();
	}
	else
	{
		// Display an error message
		display_chars(0, LCD_SEG_L1_2_0, "Err", SEG_ON);
	}
}


// *************************************************************************************************
// @fn          altitude_deactivate
// @brief       Altitude screen desactivation. Clear the screen, reset the events & values.
// @return      none
// *************************************************************************************************
static void altitude_deactivate()
{
	// Is the sensor working ?
	if(ps_ok)
	{
		// Unregister the event
		sys_messagebus_unregister(&ps_event);
	
		// Stop the sensor
		ps_stop();
	}
	
	// Cleanup screens
	display_clear(0, 1);
	display_clear(0, 2);
	
	// Cleanup symbols
	display_alt_symbols(0);
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
		&altitude_edit,
		NULL, NULL,
		&altitude_activate,
		&altitude_deactivate);
}
