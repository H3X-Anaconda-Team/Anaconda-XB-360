# ============================================================
# Anaconda XB 360 - libxenon build
# ============================================================

DEVKITXENON ?= /usr/local/xenon

CC  = $(DEVKITXENON)/bin/xenon-gcc
CXX = $(DEVKITXENON)/bin/xenon-g++
LD  = $(DEVKITXENON)/bin/xenon-ld

INCLUDES = -IInclude -ISource -ISource/ThirdParty/miniz -I$(DEVKITXENON)/usr/include -I$(DEVKITXENON)/xenon/include

CXXFLAGS = -O2 -Wall -std=c++11 -D_XBOX $(INCLUDES)
CFLAGS   = -O2 -Wall -D_XBOX $(INCLUDES)

LDFLAGS = -L$(DEVKITXENON)/lib -L$(DEVKITXENON)/usr/lib -lxenon -lm

TARGET = Build/AnacondaXB360.elf
XEX    = Build/AnacondaXB360.xex

CXX_SRC = \
    Source/App/main.cpp \
    Source/Core/Log/Log.cpp \
    Source/Core/Ini/Ini.cpp \
    Source/Core/Config/Config.cpp \
    Source/Network/Http/Http.cpp \
    Source/Network/Repo/Repo.cpp \
    Source/Install/Installer.cpp \
    Source/Ui/Ui.cpp \
    Source/Ui/UiMenu.cpp \
    Source/Ui/UiDialog.cpp \
    Source/Platform/Xbox360/PlatformXbox.cpp

C_SRC = \
    Source/ThirdParty/miniz/miniz.c

OBJ = $(CXX_SRC:.cpp=.o) $(C_SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p Build
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

xex: $(TARGET)
	xenon-elf2xex $(TARGET) $(XEX)
	@echo "Built: $(XEX)"

clean:
	rm -f $(OBJ) $(TARGET) $(XEX)
	rm -rf Build

.PHONY: all xex clean
