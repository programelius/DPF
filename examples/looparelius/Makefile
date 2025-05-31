#!/usr/bin/make -f

NAME = looparelius_minimal_py_test
DPF_PATH = ../.. # Relative path from DPF/examples/looparelius/ to DPF root

USE_PYTHON = true
PY_SOURCES = main_looper_script.py # Dummy Python script
FILES_DSP = MyMIDILooper.cpp   # Dummy C++ bridge

DPF_MIDI_EFFECT = true # Or comment out to see if it matters
OS = windows           # Explicit

# Attempt to include FIRST, before defining many targets
# This tests if the include itself is the problem with USE_PYTHON=true
include $(DPF_PATH)/Makefile.plugins.mk

# Minimal targets, let DPF defaults try to populate
all: $(DPF_MAKEFILE_TARGETS) $(DPF_MAKEFILE_EXTRA_TARGETS)

print_vars:
# TAB
	@echo "DPF_PATH was [$(DPF_PATH)] -> [$(realpath $(DPF_PATH))]"
# TAB
	@echo "TARGET_INCLUDE_FILE was [$(DPF_PATH)/Makefile.plugins.mk] -> [$(realpath $(DPF_PATH)/Makefile.plugins.mk)]"
# TAB
	@echo "USE_PYTHON is [$(USE_PYTHON)]"
# TAB
	@echo "DPF_MIDI_EFFECT is [$(DPF_MIDI_EFFECT)]"
# TAB
	@echo "OS is [$(OS)]"
# TAB
	@echo "COMPILER_TYPE from DPF: [$(COMPILER_TYPE)]"
# TAB
	@echo "DPF_MAKEFILE_TARGETS from DPF: [$(DPF_MAKEFILE_TARGETS)]"

.DEFAULT_GOAL := print_vars 