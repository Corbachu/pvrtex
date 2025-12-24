#Uncomment for debug build
#DEBUGBUILD = true

TARGET = pvrtex
OBJS = elbg.o mem.o log.o bprint.o avstring.o lfg.o crc.o md5.o stb_image_impl.o \
	stb_image_write_impl.o stb_image_resize_impl.o optparse_impl.o pvr_texture.o \
	dither.o tddither.o vqcompress.o mycommon.o palette.o file_common.o \
	file_pvr.o file_tex.o file_dctex.o pvr_texture_encoder.o pvr_texture_decoder.o main.o

ifeq ($(DEBUGBUILD), true)
OPTMODE= -Og -pg -g
else
OPTMODE= -O3 -flto=auto -march=native
endif

MYFLAGS=-Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Ilibavutil -I. -DCONFIG_MEMORY_POISONING=0 -DHAVE_FAST_UNALIGNED=0
MYCPPFLAGS=$(MYFLAGS)
MYCFLAGS=$(MYFLAGS) -Wno-pointer-sign


.PHONY: all clean install

%.o: %.c
	gcc $(CFLAGS) $(MYCFLAGS) $(OPTMODE) -c $< -o $@

%.o: %.cpp
	gcc $(CFLAGS) $(MYCPPFLAGS) $(CXXFLAGS) $(OPTMODE) -c $< -o $@


$(TARGET): $(OBJS)
	gcc $(OPTMODE) -o $(TARGET) \
		$(OBJS) $(PROGMAIN) -lm -lstdc++

clean:
	rm -f $(TARGET) $(OBJS) README

README: readme_unformatted.txt
	fmt -s readme_unformatted.txt > README

install:
	install ./pvrtex ~/.local/bin/pvrtex
	install pvrtex-completion.bash ~/.local/share/bash-completion/completions/pvrtex.bash

install-system:
	install ./pvrtex /usr/bin/pvrtex
	install pvrtex-completion.bash ~/.local/share/bash-completion/completions/pvrtex.bash

all: $(TARGET) README


