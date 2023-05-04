# Adventurous, huh? So there's some weird things happening here to make sure parellel builds work..
# Try it! 'make -j40 clean world' or similar should successfully build

all: world

PREFIX ?= /usr
PULSEAUDIO=n
ALSA=y

bins := ft8goblin ft8decoder ft8encoder sigcapd

bin_install_path := ${PREFIX}/bin
etc_install_path ?= /etc/ft8goblin
lib_install_path := ${PREFIX}/lib

# required libraries: -l${x} will be expanded later...
common_libs += yajl ev
ft8goblin_libs += ncurses termbox2
ft8coder_libs += ft8 m
sigcapd_libs += uhd rtlsdr uhd rtlsdr

ifeq (${PULSEAUDIO},y)
libs += pulse
endif
ifeq (${ALSA},y)
libs += asound
endif

ERROR_FLAGS := -Werror -Wno-error=int-conversion -Wno-missing-braces
WARN_FLAGS := -pedantic -Wno-unused-variable -Wno-unused-function #-Wno-missing-braces
CFLAGS += -fsanitize=address -O2 -ggdb3 -std=gnu11 -I./ -I./include -Wall ${WARN_FLAGS} ${ERROR_FLAGS}
LDFLAGS += $(foreach x,${common_libs},-l${x}) -fsanitize=address
ft8goblin_ldflags := ${LDFLAGS} $(foreach x,${ft8goblin_libs},-l${x})
ft8coder_ldflags := ${LDFLAGS} $(foreach x,${ft8coder_libs},-l${x})
sigcapd_ldflags := ${LDFLAGS} $(foreach x,${sigcapd_libs},-l${x})
subdirs += ft8_lib termbox2

##################
# Common Objects #
##################
common_objs += config.o
common_objs += daemon.o
common_objs += dict.o
common_objs += ipc.o
common_objs += logger.o
common_objs += memory.o
common_objs += ringbuffer.o
common_objs += util.o

################
# TUI: text UI #
################
tui_objs += tui.o tui-input.o tui-menu.o tui-textarea.o

###########
# FT8 TUI #
###########
# SQL utilities (sqlite3 / postgis wrapper)
ft8goblin_objs += sql.o

# tty user interface
ft8goblin_objs += ${tui_objs} 

# FCC ULS database (US hams)
ft8goblin_objs += fcc-db.o

ft8goblin_objs += ft8goblin.o
ft8goblin_objs += hamlib.o

# Geographic Names Information System (GNIS) local database lookup for place names
ft8goblin_objs += gnis-lookup.o

# Utility functions for dealing with maidenhead coordinates
ft8goblin_objs += maidenhead.o

# for dealing with supervising capture and decode processes
ft8goblin_objs += subproc.o

# watch lists / alerts
ft8goblin_objs += watch.o

# QRZ XML API callsign lookups (paid)
ft8goblin_objs += qrz-xml.o

###################
# FT8 De/En-coder #
###################
# interface to the FT8 library
ft8coder_objs += ft8lib.o

# ft8decoder and ft8encoder processes
ft8decoder_objs += ft8decoder.o ${ft8coder_objs}
ft8encoder_objs += ft8encoder.o ${ft8coder_objs}

###########
# sigcapd #
###########
sigcapd_objs += sigcapd.o
sigcapd_objs += uhd.o
sigcapd_objs += hamlib.o

# ALSA audio support (if you're using ncurses, you might well not have pulse/pipewire either)
sigcapd_objs += alsa.o

ifeq (${PULSEAUDIO}, y)
# XXX: This is a stub for now, feel free to write it :P
sigcapd_objs += pulse.o
sigcapd_cflags += -DPULSEAUDIO
.PHONY: obj/pulse.o pulse.c obj/sigcapd.o
obj/pulse.o: pulse.c
	${CC} ${CFLAGS} ${sigcapd_flags} -o $@ -c $<
else
extra_clean += obj/pulse.o
endif

# interprocess communication (with sigcapd and ft8goblin frontend)
sigcapd_objs += ipc.o 

# Source for UDP audio frames, such as from SDR software
sigcapd_objs += udp_src.o

##############################################################
##############################################################

# prepend obj/ to the obj names
ft8goblin_real_objs := $(foreach x,${ft8goblin_objs} ${common_objs},obj/${x})
ft8decoder_real_objs := $(foreach x,${ft8decoder_objs} ${common_objs},obj/${x})
ft8encoder_real_objs := $(foreach x,${ft8encoder_objs} ${common_objs},obj/${x})
sigcapd_real_objs := $(foreach x,${sigcapd_objs} ${common_objs},obj/${x})

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

.PHONY: clean subdirs-world distclean

distclean: 
	${RM} *.log
	${MAKE} clean subdirs-clean

clean:
	@echo "Cleaning..."

# Try to enforce cleaning before other rules
ifneq ($(filter clean,$(MAKECMDGOALS)),)
	$(shell ${RM} -f ${bins} ${ft8goblin_real_objs} ${ft8decoder_real_objs} ${ft8encoder_real_objs}  ${sigcapd_real_objs} ${extra_clean})
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

# build the user frontend
ft8goblin: ${ft8goblin_real_objs}
	${CC} -o $@ $^ ${ft8goblin_ldflags} 

# and the backends...
ft8decoder: ${ft8decoder_real_objs}
	${CC} -o $@ $^ ${ft8coder_ldflags}

ft8encoder: ${ft8encoder_real_objs}
	${CC} -o $@ $^ ${ft8coder_ldflags}

# sigcapd needs more libraries than the others
sigcapd: ${sigcapd_real_objs}
	${CC} -o $@ $^ ${sigcapd_ldflags}

obj/sigcapd.o: src/sigcapd.c
	${CC} ${CFLAGS} ${sigcapd_cflags} -o $@ -c $<

obj/%.o: src/%.c $(wildcard *.h)
	@echo "[CC] $< -> $@"
	@${CC} ${CFLAGS} -o $@ -c $<

qrztest: qrztest2.c
	gcc -o $@ $< -lxml2 -lcurl -I/usr/include/libxml2

####################################################################
# ugly hacks to quiet the compiler until we can clean things up... #
####################################################################
obj/tui-menu.o: src/tui-menu.c
#	${CC} $(filter-out -Werror,${CFLAGS}) -o $@ -c $<
	${CC} ${CFLAGS} -o $@ -c $< -Wno-int-conversion
