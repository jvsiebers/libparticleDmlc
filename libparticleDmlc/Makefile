# libparticleDmlc.a and libparticleDmlc.so Makefile
#
# 10/23/2008: SW: compile and install libparticleDmlc
#                 Modified from Makefile under 
#                 ~/code/mcv/source/particleDmlc/pdmlc++
# 11/05/2008: JVS: Minor modifications, obj's in obj/$UNAME
# 01/05/2011: JVS: Update to use common makefile like other libraries....
# 12/16/2014: JVS: Add in dynamicDelivery.cc
# 06/05/2015: JVS: Make distribution
# 9/11/2024: JVS:  Update prior to making git repo
#
#
##################################################
# Define special things for the compilation of this library
#########
LIBRARY_NAME = libparticleDmlc
#########
# Important to have SRC_DIR defined BEFORE including the commonLibraryMakefile.mk so it knows what is is
SRC_DIR = ./srcs/
INC_DIR = ./include/
#########
LIB_SRCS = scatter.cc readMLC.cc dmlcRoutines.cc createTipTable.cc particleDmlcClass.cc dynamicDelivery.cc
#########
# Special header defines for this library
LIB_HEADER_DIRS = -I$(INC_DIR) -I../libUVA 
LIB_SO_LINK_FLAGS = -L $(LIB_SYS)
LIB_DEBUG_FLAGS = -DC_CLASS 
###################################################
COMMON_MAKE = commonLibraryMakefile.mk
MyNull=$(wildcard $(COMMON_MAKE))
ifeq ($(wildcard $(COMMON_MAKE)),)
#$(warning TRUE)
include ../commonLibraryMakefile.mk
else
#$(warning FALSE)
include commonLibraryMakefile.mk
endif
##########
#
# For creating distribution
###########################
DIST_FILES =  Makefile 

DIST_BASE_FILES = ../commonLibraryMakefile.mk

DIST_HEADERS = include/controlPoint.h \
include/dynamicDelivery.h \
include/mc.h \
include/mcrand.h \
include/mcrng.h \
include/particleDmlcClass.h \
include/particleDmlc.h \
include/typesClass.h 

DMLC_CONFIGS=120lowerhalf_v2.table  120upperhalf_v2.table  mlctable.txt  mlcTungstenAttenuation.dat  VCUparticleMLC.config

LIB_UVA_FILES := $(wildcard ../libjvs/*.cc  ../libUVA/*.cc ../libmcv/*.cc ../libjvs/*.h ../libUVA/*.h ../libmcv/*.h ../include/*.h )
# LIB_UVA_REMOVE = ClifRead.cc fixBadLine.cc prototype_for_new_clif_read.cc testfile.cc max-t.cc
LIB_UVA_REMOVE = ClifRead.cc  prototype_for_new_clif_read.cc testfile.cc
###########################
LIB_UVA = libUVA
DIST_BASE    = ~/tmp/distribution
DISTRIBUTION = $(DIST_BASE)/${LIBRARY_NAME}

report:
	@echo UVA_DATA = ${UVA_DATA}

distribution:
	@echo Creating distribution
	mkdir -p ${DISTRIBUTION}/${SRC_DIR}
	mkdir -p ${DISTRIBUTION}/${INC_DIR}
	mkdir -p ${DIST_BASE}/${LIB_UVA}
	mkdir -p ${DIST_BASE}/uva/data/dmlc
	for SRC in $(DIST_BASE_FILES); do cp -p $${SRC} ${DIST_BASE}; done
	for SRC in $(LIB_SRCS); do cp -p ${SRC_DIR}/$${SRC} ${DISTRIBUTION}/${SRC_DIR}; done
	for SRC in $(LIB_UVA_FILES); do cp -p $${SRC} ${DIST_BASE}/${LIB_UVA}; done
	for HDR in $(DIST_HEADERS); do cp -p $${HDR} ${DISTRIBUTION}/${INC_DIR}; done
	for FIL in $(DIST_FILES); do cp -p $${FIL} ${DISTRIBUTION}; done
	for FIL in $(LIB_UVA_REMOVE); do rm ${DIST_BASE}/${LIB_UVA}/$${FIL}; done
	for FIL in $(DMLC_CONFIGS); do cp -p ${UVA_DATA}/dmlc/$$FIL ${DIST_BASE}/uva/data/dmlc; done
	cp -p ../libUVA/Makefile ${DIST_BASE}/${LIB_UVA}
	cp -p aReadMe.particleDmlc++ ${DIST_BASE}

#	@echo Completed copying required files to ${DISTRIBUTION}
#	tar -czvf ${DISTRIBUTION}.tar.gz ${DISTRIBUTION}
##############################
