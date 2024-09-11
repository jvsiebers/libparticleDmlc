##################################################
# Nov 11, 2010: JVS
# Common part to library makefiles
#  Modification History:
#    Dec 11, 2010:  JVS: Add valgrind section (-O0 -g)
#    Jan 3, 2011:  JVS: Get rid of COM_FLAG and add LINUX_x86_32mp section
#    Jan 6, 2011: JVS: Update SUN4SOL2 section (Horse compiles)
#    Mar 9, 2011: JVS: Add install_headers section and sysinstall
#    April 4, 2011: JVS: lib32 install enabled for LINUX_x86_32mp
#                        for libraries required by vmc++
#    Dec 2, 2011: JVS: Add general parts to compile doxygen documentation
#   April 15, 2013: JVS: Change from -O2 to -O3
#   Jan 12, 2015: JVS: Add wildcard and local_headers
#    Jan 30, 2015: JVS: Add -I /opt/csw/include for solaris compile (yak)
#    June 3, 2015: JVS: Add ability to do g++ compile on solaris (add to install path)...
#    June 9, 2015: JVS: Add ability to create +/-stlport4 compiles on solaris
#                       removed SGI and sparc-solaris options
#                       removed 32bit options
#   Aug 2015: JVS: Use g++-3.2 for 32bit compile (like vmc++)
#                    
# To do system install, use command
# >sudo -E make sysinstall
#
# Note, on solaris, sudo make sysinstall does not work.
# On solaris, directly log in as root and to the make sysinstall
#################################################
# COM_FLAG = 0
#CC = g++44
CC = g++
#CC = insure
FC = g77
AR = ar

ifeq ($(OS),Windows_NT)
UNAME=$(shell uname)
endif

UNAME=$(shell uname)

ARCH = x86_64-linux
ifeq ($(strip $(ARCH)),)
  UNAME = $(shell echo `uname -p`-$(OSTYPE))
  $(warning setting UNAME = $(UNAME), ARCH = $(ARCH))
endif

$(info $$ARCH is [${ARCH}])
$(info $$UNAME is [${UNAME}])

# WFLAGS = -Wall -Wshadow -Wtraditional -Wconversion -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline 

WFLAGS = -Wall -Wshadow  -Wconversion -Waggregate-return -Wredundant-decls -Winline -Wextra -pedantic
# -Wstrict-prototypes -Wmissing-prototypes  

# 
ifdef 32bit
   ARCH = $(UNAME)_32
   LIB_BITS = 32
  $(warning UNAME = $(UNAME), ARCH = $(ARCH))
  CC = g++-3.2 -m32
  # g++-3.2 has restricted set -f -W flags
  # Ignore -Waggregate-return
  WFLAGS = -Wall -Wshadow  -Wconversion -Wredundant-decls -Winline -pedantic
  myFLAGS=-Dgcc3
endif
# 		
ifdef local
LIB_HOME_DIR    = ../
HEADER_DIRS  =  $(LIB_HEADER_DIRS) -I./ -I$(LIB_HOME_DIR)/include -I$(LIB_HOME_DIR)/libjvs 
else
LIB_HOME_DIR = /usr/local/uva
HEADER_DIRS  =  $(LIB_HEADER_DIRS) -I./ -I$(LIB_HOME_DIR)/include 
endif
# 
LIB_LOCAL = ../$(ARCH)/$(COMPILER_OPTIONS_ADDED_PATH)
LIB_BASE = /usr/local/uva/
# $(COMPILER_OPTIONS_ADDED_PATH)
LIB_SYS = $(LIB_BASE)/lib$(LIB_BITS)/
LIB_SYS_INCLUDE	 = $(LIB_BASE)/include/

# Reset optimization levels for valgrind
ifdef valgrind
OPT_FLAGS = -g -O0
else
OPT_FLAGS = -O3
endif

DEBUG_FLAGS = $(LIB_DEBUG_FLAGS) # -DDEBUG -DDEBUG_SSD  -DDEBUG_RAYTRACE
FFLAGS = -O 
# Note, not all flags are defined for all libraries (like PCOM_FLAGS), but then they evalute to blanks
CFLAGS = $(HEADER_DIRS) $(OPT_FLAGS) $(SYSTEM_FLAGS) $(LIB_CFLAGS) $(DEBUG_FLAGS) $(PCOM_CFLAGS) $(SPECIAL_FLAGS) $(myFLAGS)
CCFLAGS = $(CFLAGS) -DmyCC $(WFLAGS) 
SO_LINK_FLAGS = -shared -fPIC
ARFLAGS = -cr  
DEPEND_FLAGS = -M

ifeq ($(ARCH), CYGWIN_NT-10.0)
# CC=x86_64-w64-mingw32-gcc.exe
# AR=x86_64-w64-mingw32-ar.exe
SYSTEM_FLAGS    = -fPIC
CFLAGS += -m64
endif

ifeq ($(ARCH), x86_64-linux-gnu)
SYSTEM_FLAGS    = -fPIC
CFLAGS += -m64
endif

ifeq ($(ARCH), x86_64-linux)
SYSTEM_FLAGS    = -fPIC  
CFLAGS += -m64
#LIB_SYS = /usr/local/uva/lib
endif
ifeq ($(ARCH), x86_64-linux_32)
SYSTEM_FLAGS    = -fPIC  
CFLAGS += -m32
#LIB_SYS = /usr/local/uva/lib
endif


#x86 solaris compilation, 64 bit
ifdef useGnu
$(warning useGnu Flag detected)
CC=g++
CC = g++
AR = gar
LIBRARY_OPTION = .g++
COMPILER_OPTIONS_ADDED_PATH += g++/
SYSTEM_FLAGS    = -fPIC  
CFLAGS += -m64
#LIB_SYS = /usr/local/uva/$(COMPILER_OPTIONS_ADDED_PATH)/lib/
else
# Use SUN compiler --- which does cause some problems
# stlport4 = true 
ifeq ($(ARCH), i386-solaris)
CC = CC 
WFLAGS = +w2
SYSTEM_FLAGS = -m64 #
CCFLAGS+=-Kpic -I /opt/csw/include
SO_LINK_FLAGS = -G  -Kpic # -fPIC # -G implies -dy and -R, see documentation for -xcode part
DEPEND_FLAGS = -xM1
AR = CC -xar        # Sun documentation recommends using CC instead of ar directly
ARFLAGS = -cr -o    # Sun needs the -o
# AR = /usr/ccs/bin/ld 
LD = /usr/ccs/bin/amd64/ld
AR = CC
ARFLAGS =  -xar -m64 -o    # Sun needs the -o
#LIB_SYS = /usr/local/uva/lib   
# COM_FLAG = 1
ifdef stlport4
# ARFLAGS
SO_LINK_FLAGS+=-library=stlport4 -xmodel=medium
# -xmodel=medium required to avoid R_AMD64_32 relocation errors: see http://developers.sun.com/solaris/articles/mmodel.html
CCFLAGS+=-library=stlport4 -xmodel=medium
LIBRARY_OPTION=.stlport4
COMPILER_OPTIONS_ADDED_PATH += stlport4/
endif
endif
endif
############################### Above in myCommonMakefile.mk

# default
#ifeq ($(COM_FLAG),0)
## -g flag is for debugging
#SYSTEM_FLAGS =  -g # -ansi # -pedantic # $(WFLAGS)
#endif

OBJ_DIR=obj/$(ARCH)/$(COMPILER_OPTIONS_ADDED_PATH)
LIB_DIR=lib/$(ARCH)/$(COMPILER_OPTIONS_ADDED_PATH)

# new explicit rule with library
$(OBJ_DIR)%.o : $(SRC_DIR)%.c Makefile
	$(CC) -c $(CFLAGS)  $< -o $@

$(OBJ_DIR)%.o : $(SRC_DIR)%.cc Makefile
	$(CC) -c $(CCFLAGS)  $< -o $@

$(OBJ_DIR)%.o : $(SRC_DIR)%.cpp Makefile
	$(CC) -c $(CCFLAGS)  $< -o $@

$(OBJ_DIR)%.o : $(SRC_DIR)%.for
	$(FC) -c $(FFLAGS)  $< -o $@

##################################################

LIB_OBJ_LIST := $(LIB_SRCS:%.cc=%.o)
LIB_OBJ_LIST := $(LIB_OBJ_LIST:%.c=%.o)
LIB_OBJ_LIST := $(LIB_OBJ_LIST:%.cpp=%.o)


LIB_OBJS := $(addprefix $(OBJ_DIR),$(LIB_OBJ_LIST))

STATIC_LIBRARY_NAME = $(strip $(LIB_DIR))/$(LIBRARY_NAME)$(LIBRARY_OPTION).a
ifneq ($(OS),Windows_NT)
SHARED_LIBRARY_NAME = $(strip $(LIB_DIR))/$(LIBRARY_NAME)$(LIBRARY_OPTION).so
endif

LIBRARIES = $(STATIC_LIBRARY_NAME) $(SHARED_LIBRARY_NAME)

HEADERS = 

all:    prep check $(STATIC_LIBRARY_NAME) install
static: prep $(STATIC_LIBRARY_NAME)
shared: prep $(SHARED_LIBRARY_NAME) 

$(STATIC_LIBRARY_NAME): $(LIB_OBJS) $(LIB_HEADERS) Makefile
	$(AR) $(ARFLAGS) $(STATIC_LIBRARY_NAME) $(LIB_OBJS) 

$(SHARED_LIBRARY_NAME): $(LIB_OBJS) $(LIB_HEADERS) Makefile
	$(CC) $(CCFLAGS) $(SO_LINK_FLAGS) $(LIB_OBJS)  $(LIB_SO_LINK_FLAGS) -o $(SHARED_LIBRARY_NAME)

.PHONY: clean
clean: 
	rm -f $(LIB_OBJS) 
	rm -f $(LIBRARIES)
	rm -r $(DEPEND)

update: prep $(LIBRARIES)
	${MAKE} ${MFLAGS} install

report: info

info:
	@echo --------------------------------
	@echo LIB_OBJ_LIST = $(LIB_OBJ_LIST)
	@echo LIB_OBJS = $(LIB_OBJS)
	@echo ARCH = $(ARCH)
	@echo CC = $(CC)
	@echo CCFLAGS = $(CCFLAGS)
	@echo SO_LINK_FLAGS = $(SO_LINK_FLAGS)
	@echo --------------------------------
	@echo LIB_SRCS = $(LIB_SRCS)
	@echo OBJ_DIR  = $(OBJ_DIR)
	@echo LIB_DIR  = $(LIB_DIR)
	@echo OS       = $(OS)
	@echo ARCH     = $(ARCH)

prep: 
	@echo Creating directories for making
	@echo -n; if [ -d $(OBJ_DIR) ]; then true; else mkdir -p $(OBJ_DIR); fi
	@echo -n; if [ -d $(LIB_DIR) ]; then true; else mkdir -p $(LIB_DIR); fi

doxygen:
	@echo Creating doxygen documentation
	@echo -n; if [ -e ./Doxyfile ]; then doxygen ./Doxyfile ; fi
	@echo -n; if [ -e ./doxygen.config ]; then doxygen ./doxygen.config ; fi

# Create dummy rule for check.  An individual makefile can overwrite it if localCheckDefined is defined
# see libEPID makefile for an example
ifeq ($(localCheckDefined), )
check:
	@echo -n
endif

local_headers := $(wildcard *.h *.hpp include/*.h include/*.hpp)

##### Installs into directory for the architecture #####
install: $(LIBRARIES)
	@echo For ARCH = $(ARCH)
	@echo installing libraries to $(LIB_LOCAL)
	@echo -n; if [ -d $(LIB_LOCAL) ]; then true; else mkdir $(LIB_LOCAL); fi
	@echo -n; for LNAME in $(LIBRARIES); do cp -p $${LNAME} ${LIB_LOCAL}; done
	@echo "   Copied files $(LIBRARIES) to $(LIB_LOCAL)"
	@echo Installation Complete


install_system: $(LIBRARIES)
	@echo For ARCH = $(ARCH)
	@echo installing libraries to $(LIB_SYS)
	if [ -d $(LIB_SYS) ]; then true; else mkdir $(LIB_SYS); fi
	@echo Copying Files to $(LIB_SYS)
	for LNAME in $(LIBRARIES); do cp -p $${LNAME} ${LIB_SYS}; done
	@echo Installation Complete
	@echo Copied files $(LIBRARIES) to $(LIB_SYS)

install_system_headers:
	@echo For ARCH = $(ARCH)
	@echo installing headers to $(LIB_SYS_INCLUDE)
	if [ -d $(LIB_SYS_INCLUDE) ]; then true; else mkdir $(LIB_SYS_INCLUDE); fi
	@echo Copying Files to $(LIB_SYS_INCLUDE)
	for HNAME in $(local_headers); do cp -p $${HNAME} ${LIB_SYS_INCLUDE}; done
	@echo Installation Complete
	@echo Copied files $(HEADERS) to $(LIB_SYS_INCLUDE)

# Install system and the include files
sysinstall: install_system install_system_headers
	@echo For ARCH = $(ARCH)
	@echo installing libraries to $(LIB_SYS)
	@echo installing headers to $(LIB_SYS_INCLUDE)

##### Checks that compiled for current architecture #####
doNothing:
	@echo "Nothing is being made here"

notNeedCheckCurrent:
	@echo Check Current is no longer needed.  Calls to check_current can be deleted from the makefile

check_current: 
	@echo "----Checking that Compiler is current----"
	if [ -r current_compile ];  then mv current_compile last_compile; fi
	@echo $(ARCH) > current_compile
	A=`fgrep -c $(ARCH) last_compile`;\
	if [ $$A -eq 0 ]; \
	then echo "----Cleaning---"; ${MAKE} clean;\
	else echo "---- Compiler is current ----";\
	fi

##############################################################
############## for creating dependancies #####################
CDEPEND=$(CC) $(CFLAGS) $(DEPEND_FLAGS)  # command to list dependancies
DEPEND=./depend.$(ARCH)     # file to put dependancies in

$(DEPEND):
	@echo "----------------------"
	@echo "Generating Depend file"
	@echo "CFLAGS = $(CFLAGS)"
	@echo -n; for j in *.cpp; do $(CDEPEND) $$j; done >  $(DEPEND); \
	for j in *.cc;  do $(CDEPEND) $$j; done >> $(DEPEND); \
	for j in $(LIB_SRCS);  do $(CDEPEND) $(SRC_DIR)/$$j; done >> $(DEPEND); \

include $(DEPEND)
############## Done for creating dependancies #################
