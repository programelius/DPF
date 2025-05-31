#!/usr/bin/make -f
# Makefile for MyTestABC (Python Plugin Test - Hardcoded Include Path)

NAME = MyTestABC_PyPlugin
DPF_PATH = ../..

FILES_DSP = MyTestABC_Plugin.cpp
USE_PYTHON = true
PY_SOURCES = main_looper_script.py
OS = windows
DPF_MIDI_EFFECT = true

# --------------------------------------------------------------
# Hardcode the include path directly
# This bypasses $(DPF_PATH) variable for the include itself
include ../../Makefile.plugins.mk
# --------------------------------------------------------------

TARGETS = vst3
all: $(TARGETS)

print_vars:
# TAB
	@echo "--- DPF Python Test Variables (MyTestABC_PyPlugin/Makefile - Hardcoded Include) ---"
# TAB
	@echo "USE_PYTHON is: [$(USE_PYTHON)]"
# TAB
	@echo "OS is: [$(OS)]"
# TAB
	@echo "COMPILER_TYPE (from DPF): [$(COMPILER_TYPE)]"
# TAB
	@echo "DPF_MAKEFILE_TARGETS (from DPF): [$(DPF_MAKEFILE_TARGETS)]"

.DEFAULT_GOAL := print_vars