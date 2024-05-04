#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = d_web

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	ExamplePluginWebMeters.cpp

FILES_UI  = \
	ExampleUIWebMeters.cpp

# --------------------------------------------------------------
# Do some magic

# MACOS_NO_DEAD_STRIP = true
UI_TYPE = web
include ../../Makefile.plugins.mk

# --------------------------------------------------------------
# Enable all possible plugin types

ifeq ($(HAVE_OPENGL),true)

TARGETS += jack

ifneq ($(MACOS_OR_WINDOWS),true)
ifeq ($(HAVE_LIBLO),true)
TARGETS += dssi
endif # HAVE_LIBLO
endif # MACOS_OR_WINDOWS

TARGETS += lv2_sep
TARGETS += vst2
TARGETS += vst3
TARGETS += clap
TARGETS += au

endif # HAVE_OPENGL

all: $(TARGETS)

# --------------------------------------------------------------
