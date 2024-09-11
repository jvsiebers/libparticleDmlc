# Generic makefile for a library
# 
# June 5, 2015: JVS: Created from libjvs makefile
#
########
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
LIBRARY_NAME = $(current_dir)
#######
SRC_DIR = ./
########
$(warning WARNING All .cc and .cpp files in $(SRC_DIR) will be included in the library $(LIBRARY_NAME))
LIB_SRCS := $(wildcard *.cc *.cpp)
########
# Special header defines for this library
LIB_HEADER_DIRS = # No extra headers needed now
LIB_DEBUG_FLAGS = #-DDEBUG
########
include ../commonLibraryMakefile.mk
########
