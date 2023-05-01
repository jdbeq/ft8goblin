all: world

PREFIX ?= /usr/local
PULSEAUDIO=n
ALSA=y

bins := ft8cli ft8decoder
bin_install_path := ${PREFIX}/bin
etc_install_path ?= /etc/ft8cli

ext_libs := ft8_lib
libs += ncurses yajl

ifeq (${PULSEAUDIO},y)
libs += pulse
endif
ifeq (${ALSA},y)
libs += asound
endif

CFLAGS := -O2 -Iext/ft8_lib
LDFLAGS := -Lext/ft8_lib $(foreach x,${libs},-l${x}) -lft8 -I./ft8_lib
subdirs += ext/ft8_lib

###########
# FT8 TUI #
###########
# Configuration parser
ft8cli_objs += config.o

# ncurses user interface
ft8cli_objs += ui.o

# watch lists / alerts
ft8cli_objs += watch.o

# ipc
ft8cli_objs += ipc.o

###############
# FT8 Decoder #
###############
# ALSA audio support (if you're using ncurses, you might well not have pulse/pipewire either)
ft8decoder_objs += alsa.o

ft8decoder_objs += config.o

# interface to the FT8 library
ft8decoder_objs += ft8lib.o

# Interface around GNU radio
# XXX: This is a stub for now, feel free to write it :P
ft8decoder_objs += pulse.o

# Source for UDP audio frames, such as from SDR software
ft8decoder_objs += udp_src.o

# Source for the main decoder process
ft8decoder_objs += decoder.o

# ipc
ft8decoder_objs += ipc.o

##############################################################
##############################################################

# prepend obj/ to the obj names
ft8cli_real_objs := $(foreach x,${ft8cli_objs},obj/${x})
ft8decoder_real_objs := $(foreach x,${ft8decoder_objs},obj/${x})

# Build all subdirectories first, then our binary
world: subdirs-world ${bins}

install: subdirs-install
	install -m 0755 ${bins} ${bin_install_path}/

.PHONY: clean subdirs-world
clean:
	@echo "Cleaning..."

# Try to enforce cleaning before other rules
ifneq ($(filter clean,$(MAKECMDGOALS)),)
	$(shell ${RM} -f ${bins} ${ft8cli_real_objs} ${ft8decoder_real_objs})
	@${MAKE} subdirs-clean
endif

subdirs-clean:
	@echo "Cleaning subdirectories..."
	@for i in ${subdirs}; do ${MAKE} -C $$i clean ; done

subdirs-install:
	@echo "Install subdirectories..."
	@for i in ${subdirs}; do ${MAKE} -C $$i install ; done

subdirs-world:
	@echo "Building subdirectories..."
	@for i in ${subdirs}; do ${MAKE} -C $$i ; done


ft8cli: ${ft8cli_real_objs}
	${CC} -o $@ $^ ${LDFLAGS} 

ft8decoder: ${ft8decoder_real_objs}
	${CC} -o $@ $^ ${LDFLAGS}

obj/%.o: %.c
	@echo "[CC] $< -> $@"
	@${CC} ${CFLAGS} -o $@ -c $<

