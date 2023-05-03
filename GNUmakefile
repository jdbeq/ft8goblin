all: world

PREFIX ?= /usr
PULSEAUDIO=n
ALSA=y

bins := ft8goblin ft8decoder
bin_install_path := ${PREFIX}/bin
etc_install_path ?= /etc/ft8goblin
lib_install_path := ${PREFIX}/lib
# required libraries (-l${x} will be expanded later)
libs += ncurses yajl ft8 ev termbox2 uhd rtlsdr

ifeq (${PULSEAUDIO},y)
libs += pulse
endif
ifeq (${ALSA},y)
libs += asound
endif

CFLAGS := -O2 -ggdb -I./ft8_lib -Wall
LDFLAGS := $(foreach x,${libs},-l${x})
subdirs += ft8_lib termbox2

##################
# Common Objects #
##################
common_objs += config.o
common_objs += ipc.o
common_objs += util.o

###########
# FT8 TUI #
###########
# SQL utilities (sqlite3 / postgis wrapper)
ft8goblin_objs += sql.o

# ncurses user interface
ft8goblin_objs += ui.o

# watch lists / alerts
ft8goblin_objs += watch.o

# FCC ULS database (US hams)
ft8goblin_objs += fcc-db.o

# QRZ XML API callsign lookups (paid)
ft8goblin_objs += qrz-xml.o

# Geographic Names Information System (GNIS) local database lookup for place names
ft8goblin_objs += gnis-lookup.o

# Utility functions for dealing with maidenhead coordinates
ft8goblin_objs += maidenhead.o

# for dealing with supervising capture and decode processes
ft8goblin_objs += subproc.o

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
ft8goblin_real_objs := $(foreach x,${ft8goblin_objs} ${common_objs},obj/${x})
ft8decoder_real_objs := $(foreach x,${ft8decoder_objs} ${common_objs},obj/${x})

# Build all subdirectories first, then our binary
world: subdirs-world subdirs-install-sudo ${bins}

# This will trigger subdirs-install if termbox2.so  is missing
subdirs-install-sudo:
	if [ ! -f ${lib_install_path}/libtermbox2.so.2.0.0 ]; then \
	   sudo ${MAKE} subdirs-install; \
	fi

install:
	@for i in ${bins}; do \
		install -m 0755 $$i ${bin_install_path}/$$i; \
	done
	install -m 0755 ft8capture.py ${bin_install_path}/ft8capture
.PHONY: clean subdirs-world

distclean: clean subdirs-clean

clean:
	@echo "Cleaning..."

# Try to enforce cleaning before other rules
ifneq ($(filter clean,$(MAKECMDGOALS)),)
	$(shell ${RM} -f ${bins} ${ft8goblin_real_objs} ${ft8decoder_real_objs})
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

ft8goblin: ${ft8goblin_real_objs}
	${CC} -o $@ $^ ${LDFLAGS} 

ft8decoder: ${ft8decoder_real_objs}
	${CC} -o $@ $^ ${LDFLAGS}

obj/%.o: %.c
	@echo "[CC] $< -> $@"
	@${CC} ${CFLAGS} -o $@ -c $<


qrztest: qrztest2.c
	gcc -o $@ $< -lxml2 -lcurl -I/usr/include/libxml2
