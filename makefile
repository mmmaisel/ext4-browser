########################################################################
# ext4 browser
# makefile
# v 0.0.1
########################################################################

# Source definitions
TARGET_NAME=ext4-browser
DEBUG_BUILD=no

# APPLE, LINUX or MINGW
TARGET_SYSTEM=MINGW
BUILD_PATH=build-mingw

LWEXT4_PATH	= lwext4

# Build path must be first otherwise precompiled header does not work
EXTRAINCDIRS = $(BUILD_PATH)/src/ src/

CPPSRC= \
	src/main.cpp \
	src/gui/mainframe.cpp \
	src/gui/backend.cpp \
	src/gui/newImageDlg.cpp \
	src/gui/diskDlg.cpp \
	src/gui/writeImageDlg.cpp \
	src/gui/fileExistsDlg.cpp \
	src/gui/fileTree.cpp \
	src/gui/aboutDlg.cpp \
	src/archive.cpp \
	src/ext4ImgArchive.cpp \
	src/blockdev.cpp \
	src/directoryLister.cpp \
	src/physicalDevice.cpp \
	src/elevation.cpp \
	src/resource/messages.cpp

CSRC =

RCSRC=src/resource/ext4-browser.rc

PREC_HEADER=src/stdafx.h

DIRS= \
	$(BUILD_PATH) \
	$(BUILD_PATH)/src \
	$(BUILD_PATH)/src/gui \
	$(BUILD_PATH)/src/resource

# Platform specific paths
LINUX_INC_PATH = \
	-I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 \
	-I/usr/include/wx-3.0 \
	-I$(LWEXT4_PATH)/build-linux/include \
	-I$(LWEXT4_PATH)/build-linux/include/generated
LINUX_LIB_PATH = \
	-L/usr/lib/x86_64-linux-gnu \
	-L$(LWEXT4_PATH)/build-linux/src
LINUX_WX_LIBS = \
	-lwx_baseu-3.0 \
	-lwx_gtk2u_core-3.0 \
	-lwx_gtk2u_ribbon-3.0 \
	-lwx_gtk2u_adv-3.0

APPLE_LIB_PATH = \
	-L/Volumes/Disk/wxWidgets-3.0.2/lib \
	-L$(LWEXT4_PATH)/build-osx/src
APPLE_INC_PATH = \
	-I/Volumes/Disk/wxWidgets-3.0.2/include \
	-I/Volumes/Disk/wxWidgets-3.0.2/lib/wx/include/osx_cocoa-unicode-3.0 \
	-I$(LWEXT4_PATH)/build-osx/include \
	-I$(LWEXT4_PATH)/build-osx/include/generated
APPLE_WX_LIBS = \
	-lwx_baseu-3.0.0.2.0 \
	-lwx_osx_cocoau_core-3.0.0.2.0 \
	-lwx_osx_cocoau_ribbon-3.0.0.2.0 \
	-lwx_osx_cocoau_adv-3.0.0.2.0

MINGW_LIB_PATH = \
	-L/var/opt/i686-w64-mingw32/wxWidgets-3.0.2/lib \
	-L$(LWEXT4_PATH)/build-win32/src
MINGW_INC_PATH = \
	-I/var/opt/i686-w64-mingw32/wxWidgets-3.0.2/include \
	-I/var/opt/i686-w64-mingw32/wxWidgets-3.0.2/lib/wx/include/i686-w64-mingw32-msw-unicode-3.0 \
	-I$(LWEXT4_PATH)/build-win32/include \
	-I$(LWEXT4_PATH)/build-win32/include/generated
MINGW_WX_LIBS = \
	-lwx_baseu-3.0-i686-w64-mingw32 \
	-lwx_mswu_core-3.0-i686-w64-mingw32 \
	-lwx_mswu_ribbon-3.0-i686-w64-mingw32 \
	-lwx_mswu_adv-3.0-i686-w64-mingw32

########################################################################

ifeq ($(TARGET_SYSTEM), MINGW)
MKDIR_P=mkdir -p
CXX=i686-w64-mingw32-g++-posix
CC=i686-w64-mingw32-gcc-posix
RC=i686-w64-mingw32-windres
_TARGET=$(TARGET_NAME).exe
else ifeq ($(TARGET_SYSTEM), APPLE)
MKDIR_P=mkdir -p
CXX=clang++
CC=clang
_TARGET=$(TARGET_NAME).app
else ifeq ($(TARGET_SYSTEM), LINUX)
MKDIR_P=mkdir -p
CXX=g++
CC=gcc
_TARGET=$(TARGET_NAME)
endif

all: directories $(BUILD_PATH)/$(PREC_HEADER).gch $(BUILD_PATH)/$(_TARGET)

DEFINES= -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL
INC_PATH= $(patsubst %,-I%,$(EXTRAINCDIRS))

ifeq ($(TARGET_SYSTEM), MINGW)
DEFINES+= -DUNICODE=1 -D_UNICODE=1 -D__WXMSW__
LIB_PATH= \
	$(MINGW_LIB_PATH) \
	-Wl,--subsystem,windows
LIBS= $(MINGW_WX_LIBS) \
	-lws2_32 \
	-lole32 \
	-loleaut32 \
	-llwext4
INC_PATH+= $(MINGW_INC_PATH)
else ifeq ($(TARGET_SYSTEM), APPLE)
DEFINES += -D__WXMAC__ -D__WXOSX__ -D__WXOSX_COCOA__ -DHAVE_TYPE_TRAITS
LIB_PATH= $(APPLE_LIB_PATH)
LIBS= $(APPLE_WX_LIBS) \
	-llwext4 \
	-lc++ \
	-framework CoreFoundation \
	-framework IOKit \
	-framework Security
INC_PATH+= $(APPLE_INC_PATH)
else ifeq ($(TARGET_SYSTEM), LINUX)
DEFINES+= -D__WXGTK__
LIB_PATH= $(LINUX_LIB_PATH)
LIBS= $(LINUX_WX_LIBS) \
	-llwext4
INC_PATH+= $(LINUX_INC_PATH)
endif

C_FLAGS=-Wall -c -fmessage-length=0 -std=c99
CPP_FLAGS=-Wall -c -fmessage-length=0 -std=c++11
DEP_FLAGS=-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

ifeq ($(DEBUG_BUILD), yes)
OPT_FLAGS=-O0 -g3
DEFINES+=-D_DEBUG
else
OPT_FLAGS=-O3
endif

OBJ_FILES=$(patsubst %,$(BUILD_PATH)/%,$(CPPSRC:.cpp=.o) $(CSRC:.c=.o))
DEP_FILES=$(patsubst %,$(BUILD_PATH)/%,$(CPPSRC:.cpp=.d) $(CSRC:.c=.d))

ifeq ($(TARGET_SYSTEM), MINGW)
OBJ_FILES+=$(patsubst %,$(BUILD_PATH)/%,$(RCSRC:.rc=.o))
CPP_FLAGS+= -mthreads -mwindows
else ifeq ($(TARGET_SYSTEM), APPLE)
CPP_FLAGS+= -stdlib=libc++
endif

########################################################################

directories:
	@-$(MKDIR_P) $(DIRS)

$(BUILD_PATH)/$(_TARGET): $(OBJ_FILES)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	$(CXX) -pthread -o $@ $(OBJ_FILES) $(LIB_PATH) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

$(BUILD_PATH)/%.o: %.rc
	$(RC) -i "$<" -o "$@"
    
$(BUILD_PATH)/%.h.gch: %.h
	@echo
	@echo Precompiling Header $<
	$(CXX) $(DEFINES) $(INC_PATH) $(CPP_FLAGS) $(OPT_FLAGS) $< -o $@
    
$(BUILD_PATH)/%.o: %.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(CXX) $(DEFINES) $(INC_PATH) $(CPP_FLAGS) $(DEP_FLAGS) $(OPT_FLAGS) -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
	
$(BUILD_PATH)/%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	$(CC) $(DEFINES) $(INC_PATH) $(C_FLAGS) $(DEP_FLAGS) $(OPT_FLAGS) -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

# Other Targets
.PHONY: clean
clean:
	rm -f $(BUILD_PATH)/$(_TARGET)
	rm -f $(BUILD_PATH)/$(PREC_HEADER).gch
	rm -f $(OBJ_FILES)
	rm -f $(DEP_FILES)

