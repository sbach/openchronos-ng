/*
    temperature.c: temperature display module

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2012 Matthew Excell <matt@excellclan.com>
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
#include "drivers/display.h"
#include "drivers/temperature.h"


// *************************************************************************************************
// Defines section


// *************************************************************************************************
// Global Variable section

#if CONFIG_TEMPERATURE_METRIC == TEMPERATURE_DEGREES_BOTH

int8_t temp_display_metric = 0; // Degrees by default (put this in cfg maybe ?)

#endif

// *************************************************************************************************
// Extern section


//* ************************************************************************************************
/// @fn			display_temp_symbols(int8_t disp)
/// @brief		Display the current unit symbol & +/- value indicator.
/// @param		disp Toogle on/off the symbols
/// @return		none
//* ************************************************************************************************
void display_temp_symbols(int8_t disp)
{
	if(disp == 0) {
		display_symbol(0,LCD_UNIT_L1_DEGREE, SEG_OFF);
		display_symbol(0, LCD_SYMB_ARROW_UP, SEG_OFF);
		display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_OFF);
	} else {
		display_symbol(0,LCD_SEG_L1_DP1, SEG_ON);
		display_symbol(0,LCD_UNIT_L1_DEGREE, SEG_ON);
	}
}


//* ************************************************************************************************
/// @fn			display_temperature(void)
/// @brief		Common display routine for metric and English units.
/// @return		none
//* ************************************************************************************************
void display_temperature(void)
{
	int16_t temp_inmetric;

	// Clear the previous value displayed
	display_clear(0, 1);
	
	// Display '°' and '.'
	display_temp_symbols(1);
	
	// Display the proper metric unit
#if CONFIG_TEMPERATURE_METRIC == TEMPERATURE_DEGREES_C
	display_char(0, LCD_SEG_L1_0, 'C', SEG_ON);
#endif

#if CONFIG_TEMPERATURE_METRIC == TEMPERATURE_DEGREES_F
	display_char(0, LCD_SEG_L1_0, 'F', SEG_ON);
#endif

#if CONFIG_TEMPERATURE_METRIC == TEMPERATURE_DEGREES_BOTH
	if (temp_display_metric == TEMPERATURE_DEGREES_C) {
		display_char(0, LCD_SEG_L1_0, 'C', SEG_ON);
	} else {
		display_char(0, LCD_SEG_L1_0, 'F', SEG_ON);
	}
#endif


	// Get the right temperature in the defined metric
#if CONFIG_TEMPERATURE_METRIC == TEMPERATURE_DEGREES_C
	temperature_get_C(&temp_inmetric);
#endif

#if CONFIG_TEMPERATURE_METRIC == TEMPERATURE_DEGREES_F
	temperature_get_F(&temp_inmetric);
#endif

#if CONFIG_TEMPERATURE_METRIC == TEMPERATURE_DEGREES_BOTH
	if(temp_display_metric == TEMPERATURE_DEGREES_C) {
		temperature_get_C(&temp_inmetric);
	} else {
		temperature_get_F(&temp_inmetric);
	}
#endif


	// Indicate temperature sign through arrow up/down icon
	if (temp_inmetric < 0) {
		// Convert negative to positive number
		temp_inmetric = ~temp_inmetric;

		display_symbol(0, LCD_SYMB_ARROW_UP, SEG_OFF);
		display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_ON);
	} else { // Temperature is >= 0
		display_symbol(0, LCD_SYMB_ARROW_UP, SEG_ON);
		display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_OFF);
	}


	// Display result in xx.x format
	display_chars(0, LCD_SEG_L1_3_1, _sprintf("%2s", temp_inmetric), SEG_ON);
}


// *************************************************************************************************
// BEGIN - EDIT_MODE_SECTION ***********************************************************************
// *************************************************************************************************


//* ************************************************************************************************
/// @fn			edit_offset_sel(void)
/// @brief		Start routine of the offset menu item.
/// @return		none
//* ************************************************************************************************
static void edit_offset_sel(void)
{
	// Display "OFST" as title
	display_chars(1, LCD_SEG_L2_3_0, "OFST", SEG_SET);
	
	// Display the current offset
	display_chars(1, LCD_SEG_L1_3_0, _sprintf("%2s", temperature.offset / 10), SEG_SET);
}


//* ************************************************************************************************
/// @fn			edit_offset_dsel(void)
/// @brief		Exit routine of the offset menu item.
/// @return		none
//* ************************************************************************************************
static void edit_offset_dsel(void)
{
	// Nothing to do for now
}


//* ************************************************************************************************
/// @fn			edit_offset_set(int8_t step)
/// @brief		Change the current offset's value when up/down buttons active.
/// @return		none
//* ************************************************************************************************
static void edit_offset_set(int8_t step)
{
	// Edit the offset by 10 each time (faster)
	temperature.offset += step * 10;
	
	// Display the current offset
	display_chars(1, LCD_SEG_L1_3_0, _sprintf("%2s", temperature.offset / 10), SEG_SET);
}


// *************************************************************************************************
// [UP] Offset selection	-	[DOWN] Metric selection
// *************************************************************************************************

#if CONFIG_TEMPERATURE_METRIC == TEMPERATURE_DEGREES_BOTH

//* ************************************************************************************************
/// @fn			edit_metric_sel(void)
/// @brief		Start routine of the metric menu item.
/// @return		none
//* ************************************************************************************************
static void edit_metric_sel(void)
{
	// Display "MTRC" as title
	display_chars(1, LCD_SEG_L2_3_0, "MTRC", SEG_SET);
	
	// Display the current metric system used
	display_chars(1, LCD_SEG_L1_3_0, (temp_display_metric == 0 ? "CELS " : "FARH"), SEG_SET);
}


//* ************************************************************************************************
/// @fn			edit_metric_dsel(void)
/// @brief		Exit routine of the metric menu item.
/// @return		none
//* ************************************************************************************************
static void edit_metric_dsel(void)
{
	// Nothing to do for now
}


//* ************************************************************************************************
/// @fn			edit_metric_set(int8_t step)
/// @brief		Switch between °C and °F.
/// @return		none
//* ************************************************************************************************
static void edit_metric_set(int8_t step)
{
	// Switch from °C to °F
	temp_display_metric = (step == 1) ? 0 : 1;

	// Display the current metric system used
	display_chars(1, LCD_SEG_L1_3_0, (temp_display_metric == 0 ? "CELS " : "FARH"), SEG_SET);
}

#endif

//* ************************************************************************************************
/// @fn			edit_save(void)
/// @brief		Stuff to do when we exit the edit mode.
/// @return		none
//* ************************************************************************************************
static void edit_save(void)
{
	// Disable blinking for the offset value
	display_chars(1, LCD_SEG_L1_3_0, NULL, BLINK_OFF);
	
	// Revert to the default screen (0)
	lcd_screen_activate(0);
}


// *************************************************************************************************
// END - EDIT_MODE_SECTION *************************************************************************
// *************************************************************************************************


//* ************************************************************************************************
/// @fn			measure_temp(enum sys_message msg)
/// @brief		Temperature display routine. Mesure and parse the temperature.
/// @return		none
//* ************************************************************************************************
static void measure_temp(enum sys_message msg)
{
	// Call the driver to measure the temperature
	temperature_measurement();
	
	// Display new stuff on the screen
	display_temperature();
}


//* ************************************************************************************************
/// @fn			temperature_activate(void)
/// @brief		Temperature screen activation. Display defaul stuff, register the mesuring loop.
/// @return		none
//* ************************************************************************************************
static void temperature_activate(void)
{
	// Create two screens, the first is always the active one
	lcd_screens_create(2);
	
	// Display the title of this module at the bottom
	display_chars(0, LCD_SEG_L2_3_0, "TEMP", SEG_SET);

	// Display something while a measure is not performed
	display_chars(0, LCD_SEG_L1_2_0, " - ", SEG_ON);

	// Register an event
	sys_messagebus_register(&measure_temp, SYS_MSG_TIMER_4S);
}


//* ************************************************************************************************
/// @fn			temperature_deactivate(void)
/// @brief		Temperature screen desactivation. Clear the screen, reset the events & values.
/// @return		none
//* ************************************************************************************************
static void temperature_deactivate(void)
{
	// Unregister the event
	sys_messagebus_unregister(&measure_temp);
	
	// Cleanup screens
	display_clear(0, 1);
	display_clear(0, 2);
	
	// Cleanup symbols
	display_temp_symbols(0);
	display_symbol(0,LCD_SEG_L1_DP1, SEG_OFF);
}


//* ************************************************************************************************
/// Edit menu for this module
//* ************************************************************************************************
static struct menu_editmode_item edit_items[] = {
	{&edit_offset_sel, &edit_offset_dsel, &edit_offset_set},
#if CONFIG_TEMPERATURE_METRIC == TEMPERATURE_DEGREES_BOTH
	{&edit_metric_sel, &edit_metric_dsel, &edit_metric_set},
#endif
	{ NULL },
};


//* ************************************************************************************************
/// @fn			mod_temperature_init(void)
/// @brief		Init the module. Sets default values, register menu entry.
/// @return		none
//* ************************************************************************************************
static void temperature_edit(void)
{
	// display_temp_symbols(0);
	
	// Switch to the edit screen (1)
	lcd_screen_activate(1);
	
	// Enable blinking for the offset value
	display_chars(1, LCD_SEG_L1_3_0, NULL, BLINK_ON);
	
	// We go into edit mode
	menu_editmode_start(&edit_save, edit_items);
}


//* ************************************************************************************************
/// @fn			mod_temperature_init(void)
/// @brief		Init the module. Sets default values, register menu entry.
/// @return		none
//* ************************************************************************************************
void mod_temperature_init(void)
{			
	menu_add_entry(" TEMP",
		NULL, NULL, NULL,
		&temperature_edit,
		NULL, NULL,
		&temperature_activate,
		&temperature_deactivate);
}
