/*
    temperature.c: temperature display module

<<<<<<< HEAD
    Copyright (C) 2012 	Matthew Excell <matt@excellclan.com>
    					Stanislas Bach <stanislasbach@gmail.com>
=======
    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2012 Matthew Excell <matt@excellclan.com>
>>>>>>> e029f212bac889dd49284466274cbb85f4f99130

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

<<<<<<< HEAD
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
#include "drivers/display.h"
#include "drivers/temperature.h"


// *************************************************************************************************
// Defines section


// *************************************************************************************************
// Global Variable section

uint8_t temp_edit = 0;


// *************************************************************************************************
// Extern section


// *************************************************************************************************
// @fn          display_temperature
// @brief       Common display routine for metric and English units.
// @return      none
// *************************************************************************************************
void display_temperature()
{
	int16_t temperature;

	// Display °C / °F
	display_symbol(0,LCD_SEG_L1_DP1, SEG_ON);
	display_symbol(0,LCD_UNIT_L1_DEGREE, SEG_ON);

	display_clear(0, 1);
	
#ifdef CONFIG_TEMPERATURE_METRIC_ONLY
	display_char(0, LCD_SEG_L1_0, 'C', SEG_ON);
#else
	if (sTemp.is_c) {
		display_char(0, LCD_SEG_L1_0, 'C', SEG_ON);
	} else {
		display_char(0, LCD_SEG_L1_0, 'F', SEG_ON);
	}
#endif


	// When using English units, convert °C to °F (temp*1.8+32)
#ifdef CONFIG_TEMPERATURE_METRIC_ONLY
	temperature = sTemp.degrees;
#else

	if (!sTemp.is_c) {
		temperature = convert_C_to_F(sTemp.degrees);
	} else {
		temperature = sTemp.degrees;
	}

#endif


	// Indicate temperature sign through arrow up/down icon
	if (temperature < 0) {
		// Convert negative to positive number
		temperature = ~temperature;

		display_symbol(0, LCD_SYMB_ARROW_UP, SEG_OFF);
		display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_ON);
	} else { // Temperature is >= 0
		display_symbol(0, LCD_SYMB_ARROW_UP, SEG_ON);
		display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_OFF);
	}


	// Display result in xx.x format
	display_chars(0, LCD_SEG_L1_3_1, _sprintf("%2s", temperature), SEG_ON);
	display_symbol(0, LCD_SEG_L1_DP1, SEG_ON);
}


// *************************************************************************************************
// @fn          display_offset
// @brief       Offset display routine for metric and English units.
// @return      none
// *************************************************************************************************
void display_offset()
{	
	int16_t offset;
	
	display_clear(0, 1);
	
#ifdef CONFIG_TEMPERATURE_METRIC_ONLY
	display_char(0, LCD_SEG_L1_0, 'C', SEG_ON);
#else
	if (sTemp.is_c) {
		display_char(0, LCD_SEG_L1_0, 'C', SEG_ON);
	} else {
		display_char(0, LCD_SEG_L1_0, 'F', SEG_ON);
	}
#endif

#ifdef CONFIG_TEMPERATURE_METRIC_ONLY
	offset = sTemp.offset;
#else
	if (!sTemp.is_c) {
		offset = convert_C_to_F(sTemp.offset);
	} else {
		offset = sTemp.offset;
	}
#endif

	display_chars(0, LCD_SEG_L1_3_1, _sprintf("%2s", offset/10), SEG_ON);
}


// *************************************************************************************************
// @fn          clear_temperature
// @brief       Clear the previous value of the temperature on the screen.
// @return      none
// *************************************************************************************************
void clear_temperature()
{
	display_symbol(0, LCD_SYMB_ARROW_UP, SEG_OFF);
	display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_OFF);

	display_clear(0, 1);

	if(!temp_edit)
	{
		display_symbol(0,LCD_SEG_L1_DP1, SEG_OFF);
		display_symbol(0,LCD_UNIT_L1_DEGREE, SEG_OFF);		
		display_chars(0,LCD_SEG_L1_3_0,NULL,BLINK_OFF);
	}
}

// *************************************************************************************************
// @fn          measure_temp
// @brief       Temperature display routine. Mesure and parse the temperature.
// @return      none
// *************************************************************************************************
static void measure_temp(enum sys_message msg)
{
	if (temp_edit)
		return;
=======
#include <openchronos.h>

/* drivers */
#include "drivers/display.h"
#include "drivers/temperature.h"

static void display_temperature(void)
{
	int16_t temp;
#ifdef CONFIG_TEMPERATURE_METRIC
	temperature_get_C(&temp);
#else
	temperature_get_F(&temp);
#endif
	_printf(0, LCD_SEG_L2_3_0, "%03s", temperature.offset);
	_printf(0, LCD_SEG_L1_3_1, "%2s", temp/10);
	display_char(0, LCD_SEG_L1_0, (temp%10)+48, SEG_SET);
}

static void measure_temp(enum sys_message msg)
{
	temperature_measurement();
	display_temperature();
}

/********************* edit mode callbacks ********************************/
>>>>>>> e029f212bac889dd49284466274cbb85f4f99130

static void edit_offset_sel(void)
{
	display_chars(0, LCD_SEG_L2_3_0, NULL, BLINK_ON);
}
static void edit_offset_dsel(void)
{
	display_chars(0, LCD_SEG_L2_3_0, NULL, BLINK_OFF);
}
static void edit_offset_set(int8_t step)
{
	temperature.offset += step;
	display_temperature();
}


<<<<<<< HEAD
// *************************************************************************************************
// @fn          temperature_activate
// @brief       Temperature screen activation. Display defaul stuff, register the mesuring loop.
// @return      none
// *************************************************************************************************
static void temperature_activate()
{
	display_chars(0, LCD_SEG_L2_3_0, "TEMP", SEG_SET);
	display_temperature();
	sys_messagebus_register(&measure_temp, SYS_MSG_TIMER_4S);
}


// *************************************************************************************************
// @fn          temperature_deactivate
// @brief       Temperature screen desactivation. Clear the screen, reset the events & values.
// @return      none
// *************************************************************************************************
static void temperature_deactivate()
{
	/* cleanup screen */
	sys_messagebus_unregister(&measure_temp);
	display_clear(0, 2);
	display_chars(0,LCD_SEG_L1_3_0,NULL,BLINK_OFF);
	temp_edit = 0;
	clear_temperature();
}


// *************************************************************************************************
// @fn          temp_change_units
// @brief       Switch the display unit for the temperature.
// @return      none
// *************************************************************************************************
static void temp_change_units()
{
#ifndef CONFIG_TEMPERATURE_METRIC_ONLY
	sTemp.is_c = !sTemp.is_c;
#endif
}
=======
static void edit_save()
{
	/* turn off blinking segments */
	display_chars(0, LCD_SEG_L2_3_0, NULL, BLINK_OFF);
}

static struct menu_editmode_item edit_items[] = {
	{&edit_offset_sel, &edit_offset_dsel, &edit_offset_set},
	{ NULL },
};

/************************** menu callbacks ********************************/
>>>>>>> e029f212bac889dd49284466274cbb85f4f99130

static void temperature_activate(void)
{
	/* display static elements */
	display_symbol(0, LCD_UNIT_L1_DEGREE, SEG_ON);
	display_symbol(0, LCD_SEG_L1_DP0, SEG_ON);
#ifdef CONFIG_TEMPERATURE_METRIC
	display_char(0, LCD_SEG_L2_4, 'C', SEG_SET);
#else
	display_char(0, LCD_SEG_L2_4, 'F', SEG_SET);
#endif
	
	/* display -- symbol while a measure is not performed */
	display_chars(0, LCD_SEG_L1_2_0, "---", SEG_ON);

<<<<<<< HEAD
// *************************************************************************************************
// @fn          temp_button_up
// @brief       Routine when the up button is pressed in edit mode.
//				Increment the temperature's offset by (+ 10) (=1.0°C)
// @return      none
// *************************************************************************************************
static void temp_button_up()
{
	if (temp_edit) {
		sTemp.offset = sTemp.offset+10;
		display_offset();
	} else {
		temp_change_units();
	}
}


// *************************************************************************************************
// @fn          temp_button_down
// @brief       Routine when the down button is pressed in edit mode.
//				Increment the temperature's offset by (- 10) (=1.0°C)
// @return      none
// *************************************************************************************************
static void temp_button_down()
{
	if (temp_edit) {
		sTemp.offset = sTemp.offset-10;
		display_offset();
	} else {
		temp_change_units();
	}
}


// *************************************************************************************************
// @fn          edit_temp_offset
// @brief       Edit mode routine. Allows to change the offset's value.
// @return      none
// *************************************************************************************************
static void edit_temp_offset()
{
	/* We go into edit mode  */
	temp_edit = !temp_edit;
	
	display_chars(0,LCD_SEG_L1_3_0,NULL,temp_edit ? BLINK_ON : BLINK_OFF);
	display_chars(0, LCD_SEG_L2_3_0, temp_edit ? "OFST" : "TEMP", SEG_SET);
	
	if(temp_edit)
	{
		clear_temperature();
		display_offset();
	}
}


// *************************************************************************************************
// @fn          mod_temperature_init
// @brief       Init the module. Sets default values, register menu entry.
// @return      none
// *************************************************************************************************
void mod_temperature_init(void)
{
	temp_edit = 0;
	
	menu_add_entry(" TEMP",
			&temp_button_up,
			&temp_button_down,
			NULL, NULL,
			&edit_temp_offset,
			NULL,
			&temperature_activate,
			&temperature_deactivate);
=======
	sys_messagebus_register(&measure_temp, SYS_MSG_TIMER_4S);
}

static void temperature_deactivate(void)
{
	sys_messagebus_unregister(&measure_temp);
	
	/* cleanup screen */
	display_symbol(0, LCD_UNIT_L1_DEGREE, SEG_OFF);
	display_symbol(0, LCD_SEG_L1_DP0, SEG_OFF);
}

static void temperature_edit(void)
{
	/* We go into edit mode  */
	menu_editmode_start(&edit_save, edit_items);
}

void mod_temperature_init(void)
{
	menu_add_entry(" TEMP", NULL, NULL,
		NULL, &temperature_edit, NULL, NULL,
		&temperature_activate, &temperature_deactivate);
>>>>>>> e029f212bac889dd49284466274cbb85f4f99130
}
