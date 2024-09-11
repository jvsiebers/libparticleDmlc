export CD = cd
export MFLAGS =
# Type "make" to see what can be made by this Makefile.
# Note- if run insure then does not compile with -static
# 
# 10/13/98: JVS: MCVMC pinnacle-monte carlo routines
# 11/12/98: JVS: Add PinnComm stuff
# Feb 2, 1999: JVS: Add wait_routines
# May 12, 1999: JVS: Add depend (pjk changed -M to -MM from jvs email)
#                                pjk also added depend to clean list 
#                                pjk added lib_check to install
# June 28, 1999: JVS: Add mcomputer
# Oct 1999: JVS: Add get_pinn_block_ids and compensator program
# Nov 1999: PJK: Add dmlc
# Nov 1999: PJK: Added -I$(HOME_DIR)/library/libpjk
# Nov 1999: PJK: Added libpjk.a to LIBRARY and LIB_NAMES
# Dec 9, 1999: JVS: Add system flags so can compile on xena
# Jan 27, 2000: PJK: added write_dml_file
# May 30, 2000: PJK: added gatedmlc
# Nov 10, 2005: JVS: Add shared library information
# Feb 8, 2007: JVS: Make shared library with common source (from previous directory)
# Jan 15, 2008: JVS: Add SWISS_DIR stuff and libmkfSwiss
# Nov 11, 2008: JVS: Makefile pared down to only compile particleDmlc++
# Nov 12, 2008: JVS: Add rule for making libmkf
# Feb 21, 2013: JVS: Switch over to libmkfSM
# Sept 4, 2014: JVS: Add writeMLCAperture
# Jan 15, 2015: JVS: Add access to IAEA source
# Jun  5, 2015: JVS: Add distribution and ability to compile off-site
# Sep 11, 2024: JVS: Adapt for github repo of libparticleDmlc
#
#
########################################
$(warning Use >make shared=true to use iaea source, which is required for arcs)
#######################################
CC = g++

UNAME = x86_64-linux

ARCH     = $(UNAME)
ifeq ($(strip $(ARCH)),)
  UNAME = $(shell echo `uname -p`-$(OSTYPE))
  $(warning setting UNAME = $(UNAME), ARCH = $(ARCH))
endif
# 
LIBE = a  # suffix for the libraries to use (a = static library)
ifdef shared
# echo Note: the lib_check will need to use the .so here too
LIBE = so  # suffix for the libraries to use (so = shared object)
endif
ifdef static
# echo Note: the lib_check will need to use the .so here too
LIBE = a  # suffix for the libraries to use (so = shared object)
endif
#
FC = g77
AR = ar
ARFLAGS = -cr 
PROFILE_FLAGS = # -pg

ifeq ($(UNAME), SGI64)
SYSTEM_FLAGS =   -O2 -static -mips4 -64 -r10000
else
SYSTEM_FLAGS =  -g -Wall -ansi -Wextra -Wno-long-long -pedantic # -static # -pedantic $(WFLAGS)
endif

ifeq ($(UNAME), LINUX)
SYSTEM_FLAGS    = -g -Wall -ansi # -O3 -march=i686 #  --ffast_math # -static # -O3
OPT_FLAGS = # -O3 --fast-math -march=i686
endif

ifeq ($(UNAME), LINUX_x86_64)
SYSTEM_FLAGS    =  -Wall -ansi # -g -O3 -march=i686 #  --ffast_math # -static # -O3
OPT_FLAGS = -O3 --fast-math # -march=i686
endif

ifeq ($(UNAME), LINUX_x86_64mp)
SYSTEM_FLAGS    =  -Wall -ansi # -g -O3 -march=i686 #  --ffast_math # -static # -O3
OPT_FLAGS = -O3 --fast-math # -march=i686
endif

ifeq ($(UNAME), x86_64-linux)
SYSTEM_FLAGS    =  -Wall -ansi # -g -O3 -march=i686 #  --ffast_math # -static # -O3
OPT_FLAGS = -O3 --fast-math # -march=i686
endif

ifeq ($(UNAME), X86SOL2)
SYSTEM_FLAGS    = -pentium 
COM_FLAG = 1
endif

ifdef valgrind 
OPT_FLAGS = -g -O0 
endif 

uvaCompile = on
ifdef uvaCompile
$(warning compiling at UVA)
endif

# LOCAL HACK TO GET THIS GOING QUICKLY
HEN_HOUSE=/mnt/d/jvsData/code/EGSnrc/HEN_HOUSE


LOCAL_LIB_DIR = ../../../library
IAEA_LIB = -I$(HEN_HOUSE)/iaea_phsp
# IAEA_LIB = -I$(LOCAL_LIB_DIR)/libiaea

$(info $$LIB_HOME_DIR = [${LIB_HOME_DIR}] )
ifdef local
LIB_HOME_DIR = $(LOCAL_LIB_DIR)
HEADER_DIRS  = -I./ -I$(LIB_HOME_DIR)/include -I$(LIB_HOME_DIR)/libjvs -I$(LIB_HOME_DIR)/libpjk -I$(LIBMKF_INCLUDE) -I$(LIB_HOME_DIR)/libparticleDmlc/include $(IAEA_LIB)
LIB_SYS_DIR  = $(LIB_HOME_DIR)/$(UNAME)
LIB_EGS_DIR  = $(HEN_HOUSE)/egs++/dso/$(myEGSConf)
else
LIB_HOME_DIR = /usr/local/uva
HEADER_DIRS  = -I./ -I./include -I$(LIB_HOME_DIR)/include -I$(LIBMKF_INCLUDE) $(LIB_HEADER_DIRS)  $(IAEA_LIB)
LIB_SYS_DIR  = ${LIB_HOME_DIR}/lib
LIB_EGS_DIR  = $(LIB_SYS_DIR)
endif
LIB_SYS      = $(LIB_SYS_DIR)
#
BIN_DIR = $(UVA_BIN)/$(UNAME)
# LIB_DIR = $(LIB_HOME_DIR)
# LIB_SYS = $(LIB_DIR)/$(UNAME)

FEATURE_FLAGS = # -DNO_VECTOR_TALLY   # Turn on NO_VECTOR_TALLY
ifdef oldlibmkf
LIBMKF = libmkf
LIBMKF_DIR = $(LIB_HOME_DIR)/libmkf
LIBMKF_INCLUDE = $(LIBMKF_DIR)/include
else
LIBMKF = libmkfSM
LIBMKF_DIR = $(LOCAL_LIB_DIR)/SourceModel2008/src/libmkf
LIBMKF_INCLUDE = $(LIBMKF_DIR)
endif

# OLD_LIBMKF is used to read the phase space file
DEBUG_FLAGS =  # -D OLD_LIBMKF #-DSOURCE_SIZE # -DDEBUG

LIBRARY = $(LIB_SYS)/libparticleDmlc.$(LIBE)  $(LIB_SYS)/$(LIBMKF).$(LIBE) $(LIB_SYS)/libpjk.$(LIBE)  $(LIB_SYS)/libjvs.$(LIBE) $(LIB_SYS)/libmcv.$(LIBE) $(LIB_SYS)/libpin.$(LIBE) $(LIB_EGS_DIR)/libiaea_phsp.so
LIB_NAMES =  libpjk.$(LIBE)  libjvs.$(LIBE) libmcv.$(LIBE) libpjk.$(LIBE) libpin.$(LIBE) libparticleDmlc.$(LIBE)  $(LIBMKF).$(LIBE)
ifdef shared
LIBRARY = -L$(LIB_SYS_DIR) -lmkfSM -lpjk -ljvs -lmcv -lpin -lparticleDmlc -liaea_phsp
endif
#
WLIBRARY = $(LIB_SYS)/libparticleDmlc.$(LIBE) $(LIB_SYS)/libpjk.$(LIBE)  $(LIB_SYS)/libjvs.$(LIBE) $(LIB_SYS)/libmcv.$(LIBE) $(LIB_SYS)/libpin.$(LIBE) 
#
# 
# End of section for UVA compile
ifndef uvaCompile
# This section is for non-UVA compile
# end of section for non-UVA compile
$(warning Set to compile outside of UVA)
# Disable several independent librarys, they are all bundled in libUVA for particleDmlc
LOCAL_LIB_DIR =
LIB_HOME_DIR = ../
LIB_SYS = ../$(ARCH)
LIBMKF =
LIBMKF_DIR =
LIBMKF_INCLUDE =
LIBRARY = $(LIB_SYS)/libparticleDmlc.$(LIBE) $(LIB_SYS)/libUVA.$(LIBE) -liaea_phsp
HEADER_DIRS  = -I./ -I../libUVA -I../libparticleDmlc/include -I$(LIB_HOME_DIR)/include  $(LIB_HEADER_DIRS)  $(IAEA_LIB)
# -I../iaea_phsp
ifdef shared
LIBRARY = -L$(LIB_SYS_DIR) -lUVA -lparticleDmlc -liaea_phsp
endif
BIN_DIR = ../bin
endif
#
CFLAGS  = $(HEADER_DIRS) $(SYSTEM_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) $(PROFILE_FLAGS) $(FEATURE_FLAGS) # -O
CCFLAGS = $(CFLAGS) -DCC $(PROFILE_FLAGS) -DC_CLASS
# Link Flags
LFLAGS = -lm 
FFLAGS = -O 

EXECUTE = particleDmlc++ 

#particleDmlc++Demo

all: $(EXECUTE)

###################################
# new explicit rule with library
%.o : %.c
	$(CC) -c $(CFLAGS)  $< -o $@

# new explicit rule with library
%.o : %.cc
	$(CC) -c $(CCFLAGS)  $< -o $@

EXECUTEClass = particleDmlcClass

particleDmlc++: particleDmlc++.o $(PDMLC_HEADERS) Makefile $(SHARED_LIBRARY_NAME) # $(LIBRARY) 
	$(CC) $<  $(CFLAGS) $(LIBRARY) $(LFLAGS) -o $@

particleDmlc++Demo: particleDmlc++Demo.o $(PDMLC_HEADERS) Makefile $(SHARED_LIBRARY_NAME) # $(LIBRARY) 
	$(CC) $<  $(CFLAGS) $(LIBRARY) $(LFLAGS) -o $@

writeMLCAperture: writeMLCAperture.cc $(PDMLC_HEADERS) Makefile $(SHARED_LIBRARY_NAME) # $(LIBRARY) 
	$(CC) $<  $(CFLAGS) $(WLIBRARY) $(LFLAGS) -o $@


######################################
######################################

update: $(LIBRARIES)
	${MAKE} ${MFLAGS} install

install: $(EXECUTE)
	@echo Installing to $(BIN_DIR)
	if [ -d $(BIN_DIR) ]; then true; else mkdir $(BIN_DIR); fi
	@echo Copying File
	for ENAME in $(EXECUTE); do cp $${ENAME} ${BIN_DIR}; done
	@echo Installation Complete
	@echo Copied files $(EXECUTE) to $(BIN_DIR)

######### Take care of dependancies on libraries   ##########
#lib_dirs = ${LIB_NAMES:.$(LIBE)=}
lib_dirs = ${LIB_NAMES:.a=}
lib_check :
	@echo " "
	@echo "----Checking that libraries are up to date ----"
	for LIBNAME in $(lib_dirs); \
	do  ${CD} ${LIB_DIR}/$${LIBNAME} ; \
	${MAKE} ${MFLAGS} update; \
	done ;
	@echo "-----Done Checking Libaries ------"
	@echo " "
##############################################################
############## for creating dependancies #####################
CDEPEND=$(CC) $(CFLAGS) -M  # command to list dependancies
DEPEND=./depend.$(ARCH)     # file to put dependancies in
$(DEPEND):
	@echo "Generating Depend file"
	for j in *.cpp; do $(CDEPEND) $$j; done >  $(DEPEND); \
	for j in *.cc;  do $(CDEPEND) $$j; done >> $(DEPEND); \

include $(DEPEND)
############## Done for creating dependancies #################

.PHONY: clean
clean:
	rm -f $(DMLC_OBJS) $(RSC_OBJS) $(COMMON_OBJS) $(DEPEND)
	rm -f $(EXECUTE) $(PDMLC_OBJS) $(FFT_OBJS)
	rm -f $(STATIC_LIBRARY_NAME) $(SHARED_LIBRARY_NAME) $(FFT_LIBRARY)
	for ENAME in $(EXECUTE); do rm -f $${ENAME}.o; done
#############################################################

report:
	@echo UNAME = $(UNAME)
	@echo LIB_SYS = $(LIB_SYS)
	@echo LOCAL_LIB_DIR = $(LOCAL_LIB_DIR)
	@echo LIBMKF_DIR = $(LIBMKF_DIR)
	@echo LIBMKF_INCLUDE = $(LIBMKF_INCLUDE)
	@echo IAEA_LIB = $(IAEA_LIB)
	@echo LIB_EGS_DIR = $(LIB_EGS_DIR)
	@echo HEADER_DIRS = $(HEADER_DIRS)
	@echo LIBRARY = $(LIBRARY)

#	@echo IAEA_SRCS = $(IAEA_SRCS)

#help:
#	@echo Requires that HEN_HOUSE is defined 
#	@echo Requires \$HEN_HOUSE/iaea_phsp is compiled
#
#
# For creating distribution
###########################
DIST_FILES = Makefile \
             particleDmlc++Demo.cc

DIST_HEADERS =  $(LOCAL_LIB_DIR)/libjvs/printRunTimeInformation.h 

myIAEA_DIR = $(HEN_HOUSE)/iaea_phsp
IAEA_SRCS := $(wildcard $(myIAEA_DIR)/*.cpp  $(myIAEA_DIR)/*.h  $(myIAEA_DIR)/Makefile)
#
#
TEST_FILES = conformalArc.dml  fixedBeam.dml   smartArc.dml  stepAndShoot.dml \
             inputPhaseSpace.egsphsp1  runTests 


###########################
DIST_BASE = ~/tmp/distribution/
DISTRIBUTION=$(DIST_BASE)/particleDmlc++Demo

distribution:
	@echo Creating distribution
	mkdir -p ${DISTRIBUTION}/${SRC_DIR}
	mkdir -p ${DISTRIBUTION}/${INC_DIR}
	mkdir -p ${DIST_BASE}/iaea_phsp
	mkdir -p ${DIST_BASE}/testRun
	for SRC in $(LIB_SRCS); do cp -p ${SRC_DIR}/$${SRC} ${DISTRIBUTION}/${SRC_DIR}; done
	for SRC in $(IAEA_SRCS); do cp -p $${SRC} ${DIST_BASE}/iaea_phsp; done
	for HDR in $(DIST_HEADERS); do cp -p $${HDR} ${DISTRIBUTION}/${INC_DIR}; done
	for FIL in $(DIST_FILES); do cp -p $${FIL} ${DISTRIBUTION}; done
	for FIL in $(TEST_FILES); do cp -p testRun/$${FIL} ${DIST_BASE}/testRun; done
	cp -p /usr/local/uva/include/egs_config1.h   $(DISTRIBUTION)/${SRC_DIR}
	cp -p Makefile.demo $(DISTRIBUTION)/${SRC_DIR}/Makefile

##############################
