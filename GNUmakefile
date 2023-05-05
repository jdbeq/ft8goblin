# Adventurous, huh? So there's some weird things happening here to make sure parellel builds work..
# Try it! 'make -j40 clean world' or similar should successfully build
all: world

PREFIX ?= /usr
PULSEAUDIO=y
ALSA=y

bins := ft8goblin decoderd-ft8 encoderd-ft8 sigcapd

bin_install_path := ${PREFIX}/bin
etc_install_path ?= /etc/ft8goblin
lib_install_path := ${PREFIX}/lib

# required libraries: -l${x} will be expanded later...
common_libs += yajl ev
ft8goblin_libs += ncurses termbox2 hamlib
ft8coder_libs += ft8 m
sigcapd_libs += uhd rtlsdr uhd rtlsdr hamlib

ifeq (${PULSEAUDIO},y)
libs += pulse
endif
ifeq (${ALSA},y)
libs += asound
endif

#ERROR_FLAGS := -Werror 
WARN_FLAGS := -pedantic -Wno-unused-variable -Wno-unused-function #-Wno-missing-braces
CFLAGS += -fsanitize=address -O2 -ggdb3 -std=gnu11 -I./ext -I./include -Wall ${WARN_FLAGS} ${ERROR_FLAGS}
LDFLAGS += $(foreach x,${common_libs},-l${x}) -fsanitize=address
ft8goblin_ldflags := ${LDFLAGS} $(foreach x,${ft8goblin_libs},-l${x})
ft8coder_ldflags := ${LDFLAGS} $(foreach x,${ft8coder_libs},-l${x})
sigcapd_ldflags := ${LDFLAGS} $(foreach x,${sigcapd_libs},-l${x})
subdirs += ext/ft8_lib ext/termbox2

##################
# Common Objects #
##################
common_objs += config.o
common_objs += daemon.o
common_objs += debuglog.o
common_objs += dict.o
common_objs += ipc.o
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
#.PHONY: obj/pulse.o obj/sigcapd.o
obj/pulse.o: src/pulse.c
	@echo "[CC] $< -> $@"
	@${CC} ${CFLAGS} ${sigcapd_flags} -o $@ -c $<
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

# and bin/ to bin names
real_bins := $(foreach x,${bins},bin/${x})

# Build all subdirectories first, then our binary
world: subdirs-world ${real_bins}


install:
	@for i in ${real_bins}; do \
		install -m 0755 $$i ${bin_install_path}/$$i; \
	done

.PHONY: clean subdirs-world distclean

distclean: 
	${MAKE} clean subdirs-clean

clean:
	@echo "Cleaning..."
# Try to enforce cleaning before other rules
ifneq ($(filter clean,$(MAKECMDGOALS)),)
	$(shell ${RM} -f logs/*.log logs/*.debug run/*.pid run/*.pipe)
	$(shell ${RM} -f ${real_bins} ${ft8goblin_real_objs} ${ft8decoder_real_objs} ${ft8encoder_real_objs}  ${sigcapd_real_objs} ${extra_clean})
endif

subdirs-clean:
	@echo "Cleaning subdirectories..."
	@for i in ${subdirs}; do ${MAKE} -C $$i clean ; done

install-deps:
	@echo "Install subdirectories..."
	@for i in ${subdirs}; do ${MAKE} -C $$i install ; done

# Allow using sudo, if needed
install-deps-sudo:
	sudo ${MAKE} subdirs-install

subdirs-world:
	@echo "Building subdirectories..."
	@for i in ${subdirs}; do ${MAKE} -C $$i ; done

#################
# Build Targets #
#################
qrztest: qrztest2.c
	gcc -o $@ $< -lxml2 -lcurl -I/usr/include/libxml2

obj/%.o: src/%.c $(wildcard *.h)
	@echo "[CC] $< -> $@"
	@${CC} ${CFLAGS} -o $@ -c $<

bin/ft8goblin: ${ft8goblin_real_objs}
	@echo "[LD] $^ -> $@"
	@${CC} -o $@ $^ ${ft8goblin_ldflags} 

bin/decoderd-ft8: ${ft8decoder_real_objs}
	@echo "[LD] $^ -> $@"
	@${CC} -o $@ $^ ${ft8coder_ldflags}

bin/encoderd-ft8: ${ft8encoder_real_objs}
	@echo "[LD] $^ -> $@"
	@${CC} -o $@ $^ ${ft8coder_ldflags}

bin/sigcapd: ${sigcapd_real_objs}
	@echo "[LD] $^ -> $@"
	@${CC} -o $@ $^ ${sigcapd_ldflags}

obj/sigcapd.o: src/sigcapd.c
	@echo "[LD] $^ -> $@"
	@${CC} ${CFLAGS} ${sigcapd_cflags} -o $@ -c $<

####################################################################
# ugly hacks to quiet the compiler until we can clean things up... #
####################################################################
obj/tui-menu.o: src/tui-menu.c
	@echo "[CC] $< -> $@"
#	${CC} $(filter-out -Werror,${CFLAGS}) -o $@ -c $<
	@${CC} ${CFLAGS} -o $@ -c $< -Wno-int-conversion -Wno-error=int-conversion -Wno-missing-braces

############3
help:
	@echo "MAKE targets:"
	@echo ""
	@echo "all | world\t\t\tBuild everything (try -j$NUMCPU!)"
	@echo "clean\t\t\t\tClean up the tree before rebuilding"
	@echo "distclean\t\t\tClean up the tree before releasing/uploading"
	@echo "install-deps\t\t\tInstall needed libraries (ft8_lib and termbox2)"
	@echo "install-deps-sudo\t\tInstall needed libraries, using sudo"
