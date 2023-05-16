#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################################################################

from waflib.TaskGen import extension, feature
from waflib.Task import Task

import waflib #TODO

import os
import sys
import common_waf
from common_waf import show

###############################################################################

def prerequisites(ctx):
	user = common_waf.common_prerequisites(ctx)
	ctx.to_log('Adding user "{}" to group "dialout"...\n'.format(user))
	ctx.exec_command2('sudo usermod -a -G dialout ' + user)
	ctx.to_log('Installing avrdude...\n')
	ctx.exec_command2('sudo apt -y install avrdude')
	#TODO For AVR Dragon programmer.
	#with open('/etc/udev/rules.d/50-avrdragon.rules', 'w') as f:
	#	f.write('SUBSYSTEM=="usb", ATTR{idVendor}=="03eb", ATTR{idProduct}=="2107", MODE="0660", GROUP="plugdev"')
	#ctx.exec_command('/etc/init.d/udev restart')

###############################################################################

def configure(cfg):
	pl = cfg.env.AVR_PATH + cfg.environ.get('PATH', '').split(os.pathsep)
	cfg.find_program(
		'avrdude',
		path_list = pl,
		var = 'AVRDUDE'
	)
	mcu = cfg.env.MCU
	if mcu == 'attiny13a':
		mcu = 'attiny13'
	cfg.env.append_value(
		'AVRDUDEFLAGS',
		[
			'-p', mcu,
			'-c', cfg.env.PROGRAMMER
		]
	)

	if cfg.env.PROGRAMMER_SPEED:
		cfg.env.AVRDUDEFLAGS += ['-b', str(cfg.env.PROGRAMMER_SPEED)]

	#TODO Take cfg.env.FREQ in consideration.
	#freq_kHz = 100
	#FIXME Need long time and MCU stay in reset.
	#if cfg.env.PROGRAMMER == 'dragon_jtag':
	#	# With this it exists without error.
	#	cfg.env.AVRDUDEFLAGS += ['-B', '{}kHz'.format(freq_kHz)]

	if cfg.env.PROGRAMMER == 'arduino':
		if not cfg.env.PROGRAMMER_PORT:
			if sys.platform.startswith('linux'):
				cfg.env.PROGRAMMER_PORT = '/dev/ttyUSB0'
			else:
				cfg.env.PROGRAMMER_PORT = 'COM2'

	#cfg.env.AVRDUDEFLAGS += ['-vv']

class avrdude_read_fuses(Task):
	def run(self):
		cmd = self.env.AVRDUDE
		cmd += self.env.AVRDUDEFLAGS
		#TODO Pipe it. Parse it. Process nice.
		#cmd.append('-Uhfuse:r:-:b')
		#cmd.append('-Ulfuse:r:-:b')
		ret = self.exec_command2(cmd)
		if ret:
			return ret

@feature('avrdude_read_fuses')
def avrdude_read_fuses__feature(tg):
	t = tg.create_task('avrdude_read_fuses', None, None)
	t.always_run = True


class avrdude_write_fuses(Task):
	def run(self):
		cmd = self.env.AVRDUDE
		cmd += self.env.AVRDUDEFLAGS
		if not(self.env.LFUSE or self.env.HFUSE or self.env.EFUSE):
			print('No fuse to write!')
			return 0
		if self.env.LFUSE:
			cmd.append('-Ulfuse:w:0x{:x}:m'.format(self.env.LFUSE))
		if self.env.HFUSE:
			cmd.append('-Uhfuse:w:0x{:x}:m'.format(self.env.HFUSE))
		if self.env.EFUSE:
			cmd.append('-Uefuse:w:0x{:x}:m'.format(self.env.EFUSE))
		ret = self.exec_command2(cmd)
		if ret:
			return ret

@feature('avrdude_write_fuses')
def avrdude_write_fuses__feature(tg):
	t = tg.create_task('avrdude_write_fuses', None, None)
	t.always_run = True



class avrdude_upload(Task):
	always_run = True
	ignore_error = False
	verify_only = False
	def run(self):
		for src in self.inputs:
			cmd = self.env.AVRDUDE
			cmd += self.env.AVRDUDEFLAGS
			if self.env.PROGRAMMER_PORT:
				cmd += ['-P', self.env.PROGRAMMER_PORT]
			if self.verify_only:
				u = 'v'
			else:
				u = 'w'
			cmd.append('-Uflash:{}:{}:i'.format(u, src.relpath()))
			r = self.exec_command2(cmd)
			if self.ignore_error:
				return 0
			else:
				return r
@extension('.hex')
def avrdude_hex_hook(tg, node):
	t = tg.create_task('avrdude_upload', node, None)
	for a in ['ignore_error', 'verify_only']:
		if hasattr(tg, a):
			setattr(t, a, getattr(tg, a))

class pre_avrdude_upload(Task):
	before = 'avrdude_upload'
	always_run = True
	color = 'RED'
	def run(self):
		return self.exec_command2(self.cmd)
@feature('pre_avrdude_upload')
def pre_avrdude_upload__feature(self):
	t = self.create_task('pre_avrdude_upload')
	t.cmd = self.cmd

class post_avrdude_upload(Task):
	after = 'avrdude_upload'
	always_run = True
	color = 'RED'
	def run(self):
		return self.exec_command2(self.cmd)
@feature('post_avrdude_upload')
def post_avrdude_upload__feature(self):
	t = self.create_task('post_avrdude_upload')
	t.cmd = self.cmd

###############################################################################
