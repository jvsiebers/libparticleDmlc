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
# Sep 13, 2020: JVS: Include libiaea, simplify makefile (get rid of garbage not needed)
#
#
########################################
# $(warning Use >make shared=true to use iaea source, which is required for arcs)
shared=true
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


DEBUG_FLAGS =  # -DDEBUG

LOCAL_LIB_DIR =
LIB_HOME_DIR = ../
LIB_SYS = ../$(ARCH)
IAEA_LIB = -I$(LIB_HOME_DIR)/libiaea
LIBRARY = $(LIB_SYS)/libparticleDmlc.$(LIBE) $(LIB_SYS)/libUVA.$(LIBE) -l$(LIB_SYS)/iaea_phsp.$(LIBE)
HEADER_DIRS  = -I./ -I../libUVA -I../libparticleDmlc/include -I$(LIB_HOME_DIR)/include  $(LIB_HEADER_DIRS)  $(IAEA_LIB)
# -I../iaea_phsp
ifdef shared
LIBRARY = -L$(LIB_SYS) -lUVA -lparticleDmlc -liaea_phsp
endif
#
CFLAGS  = $(HEADER_DIRS) $(SYSTEM_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) $(PROFILE_FLAGS) $(FEATURE_FLAGS) # -O
CCFLAGS = $(CFLAGS) -DCC $(PROFILE_FLAGS) -DC_CLASS
# Link Flags
LFLAGS = -lm 
FFLAGS = -O 

EXECUTE = particleDmlc++Demo

all: $(EXECUTE)

###################################
# new explicit rule with library
%.o : %.c
	$(CC) -c $(CFLAGS)  $< -o $@

# new explicit rule with library
%.o : %.cc
	$(CC) -c $(CCFLAGS)  $< -o $@

particleDmlc++: particleDmlc++.o $(PDMLC_HEADERS) Makefile $(SHARED_LIBRARY_NAME) # $(LIBRARY)
	$(CC) $<  $(CFLAGS) $(LIBRARY) $(LFLAGS) -o $@

particleDmlc++Demo: particleDmlc++Demo.o $(PDMLC_HEADERS) Makefile $(SHARED_LIBRARY_NAME) # $(LIBRARY) 
	$(CC) $<  $(CFLAGS) $(LIBRARY) $(LFLAGS) -o $@

######################################
######################################

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
############## for creating dependencies #####################
CDEPEND=$(CC) $(CFLAGS) -M  # command to list dependencies
DEPEND=./depend.$(ARCH)     # file to put dependencies in
$(DEPEND):
	@echo "Generating Depend file"
	for j in *.cpp; do $(CDEPEND) $$j; done >  $(DEPEND); \
	for j in *.cc;  do $(CDEPEND) $$j; done >> $(DEPEND); \

include $(DEPEND)
############## Done for creating dependencies #################

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
	@echo IAEA_LIB = $(IAEA_LIB)
	@echo LIB_EGS_DIR = $(LIB_EGS_DIR)
	@echo HEADER_DIRS = $(HEADER_DIRS)
	@echo LIBRARY = $(LIBRARY)
	@echo CFLAGS = $(CFLAGS)
	@echo LFLAGS = $(LFLAGS)
	@echo LIB_SYS_DIR = $(LIB_SYS_DIR)


##############################
