#!/usr/bin/env python2
# encoding: utf-8
# vim: set ts=4 :

###################################################################################################
# config.py
# Tool to generate the config.h file
# 
###################################################################################################
version = "0.1"
# Changelog:
#   0.1 - first version
###################################################################################################

import urwid
import urwid.raw_display
import sys
import re
import random

import cfg_reader

from lib import field_encodings
from lib.sorteddict import SortedDict

###################################################################################################

DATA = SortedDict()
WIDMAP = {}

HEADER = """\
/**
	@file		config.h
	@brief		Configuration header file
	
	@warning	GENERATED FILE, DO NOT EDIT !
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

// *************************************************************************************************
// Include section


// *************************************************************************************************
// Defines section

"""

FOOTER = """

#endif /* __CONFIG_H__ */
"""

###################################################################################################

# Generate System config
for key,field in cfg_reader.read_system_config():
	DATA[key] = field

# Display Modules section
DATA["TEXT_MODULES"] = {
	"name": "Modules",
	"type": "info"
}

# Generate modules config
for key,field in cfg_reader.read_modules_config():
	DATA[key] = field

###################################################################################################

class HelpListWalker(urwid.SimpleListWalker):
	def __init__(self, app, *args, **kwargs):
		self.app = app
		super(HelpListWalker, self).__init__(*args, **kwargs)

	def set_focus(self, focus):
		if not hasattr(self[focus], "_widget"):
			self.app.help_widget.set_text("")
		else:
			wid = self[focus]._widget
			if "help" in wid._datafield:
				self.app.help_widget.set_text(wid._datafield["help"])
		return super(HelpListWalker, self).set_focus(focus)

def widget_changed_callback(wid, state):
	global WIDMAP, DATA

	cfgname = wid._datakey

	deps = []
	for key,field in DATA.iteritems():
		if not 'depends' in field:
			continue

		if cfgname in field['depends']:
			deps.append(key)

	changeddeps = []
	for depkey in deps:
		depwid = WIDMAP[depkey]
		if isinstance(depwid, urwid.Padding):
			depwid = depwid.original_widget

		psel = depwid.original_widget.selectable()
		if state and wid.selectable():
			depwid.set_attr_map({None: 'opt'})
			depwid.original_widget._selectable = True
		else:
			depwid.set_attr_map({None: 'optd'})
			depwid.original_widget._selectable = False
			changeddeps.append( depwid.original_widget )

		if psel != depwid.original_widget.selectable():
			changeddeps.append( depwid.original_widget )

	# recursively disable widgets depending on the ones we just disabled
	for depwid in changeddeps:
		if isinstance(depwid, urwid.Edit):
			val = depwid.get_edit_text()
		elif isinstance(depwid, urwid.CheckBox):
			val = depwid.get_state()
		else:
			continue
		widget_changed_callback(depwid, val)

###################################################################################################

class CheckBoxWidget(urwid.CheckBox):
	def __init__(self, *args, **kwargs):
		self._selectable = True
		urwid.CheckBox.__init__(self, *args, **kwargs)

	def selectable(self):
		return self._selectable

###################################################################################################

class EditWidget(urwid.Edit):
	def __init__(self, *args, **kwargs):
		self._selectable = True
		urwid.Edit.__init__(self, *args, **kwargs)

	def selectable(self):
		return self._selectable

###################################################################################################

class OpenChronosConfigApp(object):
	def main(self):
		global DATA

		text_header = (u"OpenChronos config  \u2503  "
					   u"UP / DOWN / PAGE UP / PAGE DOWN scroll.  F8 aborts.")

		self.list_content = list_content = []
		for key,field in DATA.iteritems():
			# generate gui forms depending on type
			self.generate_widget(key,field)

		# rescan widgets and disable/enable them based on their dependencies
		for key,wid in WIDMAP.iteritems():
			if not hasattr(wid, '_widget'):
				continue
			wid = wid._widget
			if isinstance(wid, urwid.Edit):
				val = wid.get_edit_text()
			elif isinstance(wid, urwid.CheckBox):
				val = wid.get_state()
			else:
				continue
			widget_changed_callback(wid, val)

		def ok_pressed(*args, **kwargs):
			raise urwid.ExitMainLoop()

		def abort_pressed(*args, **kwargs):
			sys.exit(0)

		list_content.append(urwid.Divider(div_char=u"\u2550", top=1, bottom=1))
		list_content.append(
		urwid.Padding(urwid.GridFlow(
			[urwid.AttrWrap(urwid.Button("Save", ok_pressed), 'opt','optsel'),
			 urwid.AttrWrap(urwid.Button("Abort", abort_pressed), 'opt','optsel')],
			15, 4, 4, 'center'),
			('fixed left',4), ('fixed right',3)))


		header = urwid.AttrWrap(urwid.Text(text_header), 'header')
		walker = HelpListWalker(self, list_content)
		listbox = urwid.ListBox(walker)
		self.help_widget = urwid.Text("")
		footer = urwid.AttrWrap(self.help_widget, 'footer')
		frame = urwid.Frame(urwid.AttrWrap(listbox, 'body'), header=header, footer=footer)

		screen = urwid.raw_display.Screen()
		palette = [
			('header',	'white',	'dark red', 	'bold'),
			('info',	'black',	'dark cyan'),
			('body',	'black',	'dark gray'),
			('optsel',	'white',	'dark blue'),
			('opt',		'black',	'light gray'),
			('optd',	'black',	'dark gray'),
			]


		def unhandled(key):
			if key == 'f8':
				#raise urwid.ExitMainLoop()
				sys.exit(0)

		urwid.MainLoop(frame, palette, screen,
			unhandled_input=unhandled).run()

	def generate_widget(self, key, field):
		global WIDMAP

		if field.get("type", "bool") == "bool":
			wid = CheckBoxWidget(field["name"], field["value"])
			urwid.connect_signal(wid, "change", widget_changed_callback)
			wid._datakey = key
			wid._datafield = field
			f = urwid.AttrWrap(wid, 'opt','optsel')
			
			if field.has_key('ischild') and field['ischild']:
				f = urwid.Padding(f, width=77, left=3)
			
			f._widget = wid
			WIDMAP[key] = f
			self.list_content.append(f)

		elif field["type"] == "text":
			wid = EditWidget("%s: "%field["name"], str(field["value"]))
			urwid.connect_signal(wid, "change", widget_changed_callback)
			wid._datakey = key
			wid._datafield = field
			f = urwid.AttrWrap(wid, 'opt', 'optsel')
			
			if field.has_key('ischild') and field['ischild']:
				f = urwid.Padding(f, width=77, left=3)

			f._widget = wid
			WIDMAP[key] = f
			self.list_content.append(f)

		elif field["type"] == "info":
			wid = urwid.Text(field["name"])
			wid._datakey = key
			wid._datafield = field
			f = urwid.AttrWrap(wid, 'info', 'info')
			f._widget = wid
			WIDMAP[key] = f
			self.list_content.append(f)

	def get_config(self):
		return DATA

	def save_config(self):
		global WIDMAP, DATA

		for key,field in WIDMAP.iteritems():
			if not hasattr(field, '_widget'):
				continue

			wid = field._widget

			if isinstance(wid, urwid.Edit):
				DATA[key]["value"] = wid.get_edit_text()
			elif isinstance(wid, urwid.CheckBox):
				DATA[key]["value"] = wid.get_state()

		fp = open("config/config.h", "w")

		fp.write(HEADER)

		for key,dat in DATA.iteritems():
			
			if not "value" in dat:
				continue
			
			if "type" in dat and dat["type"] == "info":
				continue
			
			if "encoding" in dat:
				fun = getattr(field_encodings, dat['encoding'])
				dat["value"] = fun(dat["value"], True)
			
			if DATA[key].get("ifndef", False):
				fp.write("#ifndef %s\n" %key)
			
			if isinstance(dat["value"], bool):
				if dat["value"]:
					fp.write("#define %s\n" %key)
				else:
					fp.write("// %s is not set\n" %key)
			else:
				fp.write("#define %s %s\n" %(key, dat["value"]))
			
			if DATA[key].get("ifndef", False):
				fp.write("#endif // %s\n" %key)

		fp.write(FOOTER)


	def load_config(self):
		global DATA

		def set_default():
			for key,dat in DATA.iteritems():
				if not "value" in dat and "default" in dat:
					dat["value"] = dat["default"]

		try:
			fp = open("config/config.h")
		except (OSError, IOError):
			set_default()
			return
		
		match = re.compile('^[\t ]*#[\t ]*define[\t ]+([a-zA-Z0-9_]+)[\t ]*(.*)$')
		match2 = re.compile('^// ([a-zA-Z0-9_]+) is not set$')

		for line in fp:
			m = match.search(line)
			if m:
				m = m.groups()
				if not m[0] in DATA:
					continue
				if m[1] == "":
					DATA[m[0]]["value"] = True
				else:
					try:
						value = int(m[1])
					except ValueError:
						value = m[1]
					DATA[m[0]]["value"] = value
				if "encoding" in DATA[m[0]]:
					fun = getattr(field_encodings, DATA[m[0]]['encoding'])
					DATA[m[0]]["value"] = fun(DATA[m[0]]["value"], False)
			else:
				m = match2.search(line)
				if m and m.groups()[0] in DATA:
					m = m.groups()
					DATA[m[0]]["value"] = False

		set_default()

###################################################################################################
# Main

if __name__ == "__main__":

	App = OpenChronosConfigApp()
	App.load_config()
	App.main()
	App.save_config()

