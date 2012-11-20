/*
    TODO altitude.h: Altitude driver header

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

#ifndef ALTITUDE_H_
#define ALTITUDE_H_

// *************************************************************************************************
// Include section

#include <openchronos.h>


// *************************************************************************************************
// Defines section




// *************************************************************************************************
// Prototypes section

void altitude_init(void);
void start_altitude_measurement(void);
void stop_altitude_measurement(void);
void altitude_measurement(void);


// *************************************************************************************************
// Global Variable section

struct {

	// Pressure (Pa)
	uint32_t		pressure;

	// Temperature (°K)
	uint16_t		temperature;

	// Altitude (m)
	int16_t		altitude;

	// Altitude offset stored during calibration
	int16_t		altitude_offset;

	// Timeout
	uint16_t		timeout;
	
} altitude;

#endif /* __ALTITUDE_H__ */
