#!/usr/bin/make -f
# Makefile for LoopareliusCpp (Pure C++ DPF Plugin)

NAME = LoopareliusCpp
DPF_PATH = ../..

override BUILD_DIR = build
override BIN_DIR = bin
override TARGET_DIR = $(BIN_DIR)

OS = windows
COMPILER_TYPE = gcc
CC = gcc
CXX = g++

FILES_DSP = \
	LoopareliusPlugin.cpp \
	RetrospectiveMidiBuffer.cpp \
	MarkovModels.cpp

include $(DPF_PATH)/Makefile.plugins.mk

TARGETS =
TARGETS += vst3

all: $(TARGETS)

print_vars:
# TAB
	@echo "--- DPF Looparelius C++ Variables ---"
# TAB
	@echo "FILES_DSP is: [$(FILES_DSP)]"
# ... (rest of print_vars if you want to keep it for `make print_vars`) ...

# .DEFAULT_GOAL := print_vars # Make sure this is commented out for building