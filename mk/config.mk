PREFIX ?= /usr
PULSEAUDIO=y
ALSA=y
termbox_lib_ver := 2.0.0

bin_install_path := ${PREFIX}/bin
etc_install_path ?= /etc/ft8goblin
lib_install_path := ${PREFIX}/lib

# required libraries: -l${x} will be expanded later...
common_libs += yajl ev
ft8goblin_libs += ncurses termbox2 hamlib m curl
ft8coder_libs += m
sigcapd_libs += uhd rtlsdr uhd rtlsdr hamlib

ERROR_FLAGS := -Werror 
SAN_FLAGS := -fsanitize=address
WARN_FLAGS := -Wall -pedantic -Wno-unused-variable -Wno-unused-function #-Wno-missing-braces
OPT_FLAGS := -O2 -ggdb3
C_STD := -std=gnu11
CXX_STD := -std=gnu++17
CFLAGS += ${C_STD} -I./ext/ -I./include/ -I./ext/ft8_lib/
CFLAGS += ${SAN_FLAGS} ${WARN_FLAGS} ${ERROR_FLAGS} ${OPT_FLAGS}
CFLAGS += -DVERSION="\"${VERSION}\""
CXXFLAGS := ${CXX_STD} $(filter-out ${C_STD},${CFLAGS})
LDFLAGS += -L./lib/ $(foreach x,${common_libs},-l${x}) -fsanitize=address
ft8lib_cflags := -fPIC
ft8goblin_ldflags := ${LDFLAGS} $(foreach x,${ft8goblin_libs},-l${x})
ft8coder_ldflags := ${LDFLAGS} $(foreach x,${ft8coder_libs},-l${x})
ft8coder_ldflags += -lft8
sigcapd_ldflags := ${LDFLAGS} $(foreach x,${sigcapd_libs},-l${x})

ifeq (${PULSEAUDIO},y)
libs += pulse
endif
ifeq (${ALSA},y)
libs += asound
endif
