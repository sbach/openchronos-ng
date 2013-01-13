#!/usr/bin/env python2
# encoding: utf-8
# vim: set ts=4 :

###################################################################################################
# make_modinit.py
# Tool to generate the modinit.c file
# (C) 2012, Angelo Arrifano <miknix@gmail.com>
# 
# *************************************************************************************************
# This file is part of OpenChronos. This file is free software: you can
# redistribute it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation, version 2.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
# 
# *************************************************************************************************
#
###################################################################################################
version = "0.1"
# Changelog:
#   0.1 - first version
###################################################################################################

import cfg_reader

###################################################################################################

initcode = """\
/**
	@file		modinit.c
	@brief		Module init
	
	@warning	GENERATED FILE, DO NOT EDIT !
 */


// *************************************************************************************************
// Include section


// *************************************************************************************************
// Defines section


// *************************************************************************************************
// Global Variable section


// *************************************************************************************************
// Extern section

//* ************************************************************************************************
/// @fn		mod_init(void)
/// @brief	Call the init function of each module.
/// @return	none
//* ************************************************************************************************
void mod_init(void)
{
"""

###################################################################################################
# Main

from config import OpenChronosConfigApp

app = OpenChronosConfigApp()
app.load_config()

cfg = app.get_config()

f = open('config/modinit.c', 'w')

f.write(initcode)

for mod in cfg_reader.get_modules():
	MOD = mod.upper()
	try:
		if cfg["CONFIG_MOD_%s" % MOD]["value"]:
			f.write("\tmod_%s_init();\n" % (mod) )
	except KeyError:
		pass

f.write("}\n")
f.close()