#!/usr/bin/make -f
# Makefile for DISTRHO Plugins # (MidiThrough Example - WITH DEBUG PRINT_VARS)
# ---------------------------- #
# Created by falkTX
#

OS = windows

# --------------------------------------------------------------
# Project name, used for binaries
NAME = d_midiThrough

# --------------------------------------------------------------
# DPF Path (added for clarity in print_vars, matches implicit include)
DPF_PATH = ../..

# --------------------------------------------------------------
# Files to build
FILES_DSP = \
	MidiThroughExamplePlugin.cpp

# --------------------------------------------------------------
# Do some magic
include $(DPF_PATH)/Makefile.plugins.mk # Changed to use DPF_PATH for consistency with print_vars
# --------------------------------------------------------------

# Enable all possible plugin types (original from example)
TARGETS += jack
TARGETS += lv2_dsp
TARGETS += vst2
TARGETS += vst3
TARGETS += clap
TARGETS += au

all: $(TARGETS) # Original 'all' target

# --- DEBUGGING print_vars TARGET ---
print_vars:
# TAB below
	@echo "--- MidiThrough C++ Example - DPF Variables ---"
# TAB below
	@echo "NAME: [$(NAME)]"
# TAB below
	@echo "DPF_PATH: [$(DPF_PATH)] -> [$(realpath $(DPF_PATH))]"
# TAB below
	@echo "FILES_DSP: [$(FILES_DSP)]"
# TAB below
	@echo "OS (from DPF): [$(OS)]"
# TAB below
	@echo "COMPILER_TYPE (from DPF): [$(COMPILER_TYPE)]"
# TAB below
	@echo "DPF_MAKEFILE_TARGETS (from DPF): [$(DPF_MAKEFILE_TARGETS)]"
# TAB below
	@echo "OUR EXPLICIT TARGETS in this Makefile: [$(TARGETS)]"
# TAB below
	@echo "CC (from DPF or default): [$(CC)]"
# TAB below
	@echo "CXX (from DPF or default): [$(CXX)]"
# TAB below
	@echo "GCC (boolean flag from DPF): [$(GCC)]"
# TAB below
	@echo "CLANG (boolean flag from DPF): [$(CLANG)]"
# TAB below
	@echo "---------------------------------------------"

# Make 'print_vars' the default goal for this test
.DEFAULT_GOAL := print_vars
# --------------------------------------------------------------