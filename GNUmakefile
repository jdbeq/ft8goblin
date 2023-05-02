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

CFLAGS := -O2 -ggdb -I./ft8_lib -Wall
LDFLAGS := -L./ft8_lib -L./termbox2 $(foreach x,${libs},-l${x}) -lft8 -ltermbox2
subdirs += ft8_lib termbox2

##################
# Common Objects #
##################
common_objs += config.o
common_objs += ipc.o

###########
# FT8 TUI #
###########
# SQL utilities (sqlite3 / postgis wrapper)
ft8cli_objs += sql.o

# ncurses user interface
ft8cli_objs += ui.o

# watch lists / alerts
ft8cli_objs += watch.o

# FCC ULS database (US hams)
ft8cli_objs += fcc-db.o

# QRZ XML API callsign lookups (paid)
ft8cli_objs += qrz-xml.o

# Geographic Names Information System (GNIS) local database lookup for place names
ft8cli_objs += gnis-lookup.o

# Utility functions for dealing with maidenhead coordinates
ft8cli_objs += maidenhead.o

###############
# FT8 Decoder #
###############
# ALSA audio support (if you're using ncurses, you might well not have pulse/pipewire either)
ft8decoder_objs += alsa.o

# uhd (USRP) devices
ft8decoder_objs += uhd.o

# interface to the FT8 library
ft8decoder_objs += ft8lib.o

# Interface around GNU radio
# XXX: This is a stub for now, feel free to write it :P
ft8decoder_objs += pulse.o

# Source for UDP audio frames, such as from SDR software
ft8decoder_objs += udp_src.o

# Source for the main decoder process
ft8decoder_objs += decoder.o

##############################################################
##############################################################

# prepend obj/ to the obj names
ft8cli_real_objs := $(foreach x,${ft8cli_objs} ${common_objs},obj/${x})
ft8decoder_real_objs := $(foreach x,${ft8decoder_objs} ${common_objs},obj/${x})

# Build all subdirectories first, then our binary
world: subdirs-world ${bins}

install: subdirs-install
	@for i in ${bins}; do \
		install -m 0755 $$i ${bin_install_path}/$$i; \
	done
	install -m 0755 ft8capture.py ${bin_install_path}/ft8capture
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


qrztest: qrztest2.c
	gcc -o $@ $< -lxml2 -lcurl -I/usr/include/libxml2
