
VERSION = 20230505.01
all: world
bins := ft8goblin decoderd-ft8 encoderd-ft8 sigcapd

include mk/config.mk

common_objs += config.o
common_objs += daemon.o
common_objs += debuglog.o
common_objs += dict.o
common_objs += ipc.o
common_objs += memory.o
common_objs += ringbuffer.o
common_objs += util.o
tui_objs += tui.o tui-input.o tui-menu.o tui-textarea.o

ft8goblin_objs += sql.o		# sqlite3 / postgis wrapper
ft8goblin_objs += ${tui_objs} 	# text user interface
ft8goblin_objs += fcc-db.o	# FCC ULS database
ft8goblin_objs += ft8goblin.o	# main TUI program
ft8goblin_objs += hamlib.o	# hamlib (rigctld) interface
ft8goblin_objs += gnis-lookup.o	# place names database
ft8goblin_objs += maidenhead.o	# maidenhead coordinates tools
ft8goblin_objs += subproc.o	# subprocess management
ft8goblin_objs += watch.o	# watch lists
ft8goblin_objs += qrz-xml.o	# QRZ XML API callsign lookups (paid)
ft8coder_objs += ft8lib.o	# interface to the FT8 library
ft8decoder_objs += decoderd-ft8.o ${ft8coder_objs}
ft8encoder_objs += encoderd-ft8.o ${ft8coder_objs}
sigcapd_objs += sigcapd.o
sigcapd_objs += uhd.o
sigcapd_objs += hamlib.o
sigcapd_objs += alsa.o		# ALSA Linux Audio

ifeq (${PULSEAUDIO}, y)
sigcapd_objs += pulse.o		# pulseaudio
sigcapd_cflags += -DPULSEAUDIO
# make these force rebuild of obj/pulse.o
obj/sigcapd.o: include/config.h mk/config.mk
else
extra_clean += obj/pulse.o
endif
sigcapd_objs += udp_src.o	# UDP input (gnuradio, gqrx, etc)

# Explode file names as needed for building
ft8goblin_real_objs := $(foreach x,${ft8goblin_objs} ${common_objs},obj/${x})
ft8decoder_real_objs := $(foreach x,${ft8decoder_objs} ${common_objs},obj/${x})
ft8encoder_real_objs := $(foreach x,${ft8encoder_objs} ${common_objs},obj/${x})
sigcapd_real_objs := $(foreach x,${sigcapd_objs} ${common_objs},obj/${x})
real_bins := $(foreach x,${bins},bin/${x})
extra_clean += ${ft8goblin_real_objs} ${ft8decoder_real_objs} ${ft8encoder_real_objs} ${sigcapd_real_objs}
extra_clean += ${real_bins} ${ft8lib} ${ft8lib_objs}

#################
# Build Targets #
#################
qrztest: qrztest2.c
	gcc -o $@ $< -lxml2 -lcurl -I/usr/include/libxml2

bin/ft8goblin: ${ft8goblin_real_objs}
	@echo "[Linking] $^ to $@"
	@${CC} -o $@ $^ ${ft8goblin_ldflags} ${LDFLAGS}

bin/decoderd-ft8: ${ft8decoder_real_objs}
	@echo "[Linking] $^ to $@"
	@${CC} -o $@ $^ ${ft8coder_ldflags} ${LDFLAGS}

bin/encoderd-ft8: ${ft8encoder_real_objs}
	@echo "[Linking] $^ to $@"
	@${CC} -o $@ $^ ${ft8coder_ldflags} ${LDFLAGS}

bin/sigcapd: ${sigcapd_real_objs}
	@echo "[Linking] $^ to $@"
	@${CXX} -o $@ $^ ${sigcapd_ldflags} ${LDFLAGS}

obj/sigcapd.o: src/sigcapd.cc
	@echo "[CXX] $^ -> $@"
	@${CXX} ${CXXFLAGS} ${sigcapd_cflags} -o $@ -c $<

####################################################################
# ugly hacks to quiet the compiler until we can clean things up... #
####################################################################
obj/tui-menu.o: src/tui-menu.c
	@echo "[CC] $< -> $@"
#	${CC} $(filter-out -Werror,${CFLAGS}) -o $@ -c $<
	@${CC} ${CFLAGS} -o $@ -c $< -Wno-error=int-conversion -Wno-missing-braces

include mk/compile.mk
include mk/ft8lib.mk
include mk/termbox2.mk
include mk/yajl.mk
include mk/help.mk
include mk/clean.mk
include mk/install.mk

# Build all subdirectories first, then our binary
world: ${extra_build_targets} ${real_bins}
