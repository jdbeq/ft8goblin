#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: ft8 mass decoder
# Author: Dangerous Devices
# Copyright: 2023 Dangerous Devices
# Description: This flowgraph captures and streams all the HF ft8 subbands to external decoder processes via UDP
# GNU Radio version: 3.10.5.1

from packaging.version import Version as StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio.filter import firdes
import sip
from gnuradio import filter
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import network
from gnuradio import uhd
import time



from gnuradio import qtgui

class ft8capture(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "ft8 mass decoder", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("ft8 mass decoder")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "ft8capture")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Variables
        ##################################################
        self.rf_bandwidth = rf_bandwidth = 29000000
        self.waterfall_update_main = waterfall_update_main = 0.1
        self.waterfall_update_band = waterfall_update_band = 0.1
        self.slot_width = slot_width = 3500
        self.samp_rate = samp_rate = 48000
        self.rf_samp_rate = rf_samp_rate = 33333333
        self.rf_gain = rf_gain = 16
        self.rf_center = rf_center = rf_bandwidth/2
        self.ft8_80m = ft8_80m = 3573000
        self.ft8_60m = ft8_60m = 5357000
        self.ft8_40m = ft8_40m = 7074000
        self.ft8_30m = ft8_30m = 10133000
        self.ft8_20m = ft8_20m = 14074000
        self.ft8_17m = ft8_17m = 18100000
        self.ft8_160m = ft8_160m = 1084000
        self.ft8_15m = ft8_15m = 21074000
        self.ft8_12m = ft8_12m = 24915000
        self.ft8_10m = ft8_10m = 28074000
        self.decimation = decimation = 1
        self.antenna = antenna = "RX2"

        ##################################################
        # Blocks
        ##################################################

        self.uhd_usrp_source_0 = uhd.usrp_source(
            ",".join(('type=x300', '')),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
        )
        self.uhd_usrp_source_0.set_clock_source('gpsdo', 0)
        self.uhd_usrp_source_0.set_time_source('gpsdo', 0)
        self.uhd_usrp_source_0.set_subdev_spec('B:0', 0)
        self.uhd_usrp_source_0.set_samp_rate(rf_samp_rate)
        # Set the time to GPS time on next PPS
        # get_mboard_sensor("gps_time") returns just after the PPS edge,
        # thus add one second and set the time on the next PPS
        self.uhd_usrp_source_0.set_time_next_pps(uhd.time_spec(self.uhd_usrp_source_0.get_mboard_sensor("gps_time").to_int() + 1.0))
        # Sleep 1 second to ensure next PPS has come
        time.sleep(1)

        self.uhd_usrp_source_0.set_center_freq(rf_center, 0)
        self.uhd_usrp_source_0.set_bandwidth(rf_bandwidth, 0)
        self.uhd_usrp_source_0.set_gain(rf_gain, 0)
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            (slot_width * 0.5), #fc
            slot_width, #bw
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.set_update_time(waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.enable_grid(True)
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.enable_axis_labels(False)

        self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.disable_legend()


        labels = ['10M', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_4_0_0_0_0_0_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_4_0_0_0_0_0_0_win, 5, 1, 1, 1)
        for r in range(5, 6):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            (slot_width * 0.5), #fc
            slot_width, #bw
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0.set_update_time(waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0.enable_grid(True)
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0.enable_axis_labels(False)

        self.qtgui_waterfall_sink_x_4_0_0_0_0_0.disable_legend()


        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_4_0_0_0_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_4_0_0_0_0_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_4_0_0_0_0_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_4_0_0_0_0_0.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_4_0_0_0_0_0.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_4_0_0_0_0_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_4_0_0_0_0_0.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_4_0_0_0_0_0_win, 5, 0, 1, 1)
        for r in range(5, 6):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_waterfall_sink_x_4_0_0_0_0 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            (slot_width * 0.5), #fc
            slot_width, #bw
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_4_0_0_0_0.set_update_time(waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0_0_0_0.enable_grid(True)
        self.qtgui_waterfall_sink_x_4_0_0_0_0.enable_axis_labels(False)

        self.qtgui_waterfall_sink_x_4_0_0_0_0.disable_legend()


        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_4_0_0_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_4_0_0_0_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_4_0_0_0_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_4_0_0_0_0.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_4_0_0_0_0.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_4_0_0_0_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_4_0_0_0_0.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_4_0_0_0_0_win, 4, 1, 1, 1)
        for r in range(4, 5):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_waterfall_sink_x_4_0_0_0 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            (slot_width * 0.5), #fc
            slot_width, #bw
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_4_0_0_0.set_update_time(waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0_0_0.enable_grid(True)
        self.qtgui_waterfall_sink_x_4_0_0_0.enable_axis_labels(False)

        self.qtgui_waterfall_sink_x_4_0_0_0.disable_legend()


        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_4_0_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_4_0_0_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_4_0_0_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_4_0_0_0.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_4_0_0_0.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_4_0_0_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_4_0_0_0.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_4_0_0_0_win, 4, 0, 1, 1)
        for r in range(4, 5):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_waterfall_sink_x_4_0_0 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            (slot_width * 0.5), #fc
            slot_width, #bw
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_4_0_0.set_update_time(waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0_0.enable_grid(True)
        self.qtgui_waterfall_sink_x_4_0_0.enable_axis_labels(False)

        self.qtgui_waterfall_sink_x_4_0_0.disable_legend()


        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_4_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_4_0_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_4_0_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_4_0_0.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_4_0_0.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_4_0_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_4_0_0.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_4_0_0_win, 3, 1, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_waterfall_sink_x_4_0 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            (slot_width * 0.5), #fc
            slot_width, #bw
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_4_0.set_update_time(waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0.enable_grid(True)
        self.qtgui_waterfall_sink_x_4_0.enable_axis_labels(False)

        self.qtgui_waterfall_sink_x_4_0.disable_legend()


        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_4_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_4_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_4_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_4_0.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_4_0.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_4_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_4_0.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_4_0_win, 3, 0, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_waterfall_sink_x_4 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            (slot_width * 0.5), #fc
            slot_width, #bw
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_4.set_update_time(waterfall_update_band)
        self.qtgui_waterfall_sink_x_4.enable_grid(True)
        self.qtgui_waterfall_sink_x_4.enable_axis_labels(False)

        self.qtgui_waterfall_sink_x_4.disable_legend()


        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_4.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_4.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_4.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_4.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_4.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_4_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_4.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_4_win, 2, 1, 1, 1)
        for r in range(2, 3):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_waterfall_sink_x_3 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            (slot_width * 0.5), #fc
            slot_width, #bw
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_3.set_update_time(waterfall_update_band)
        self.qtgui_waterfall_sink_x_3.enable_grid(True)
        self.qtgui_waterfall_sink_x_3.enable_axis_labels(False)

        self.qtgui_waterfall_sink_x_3.disable_legend()


        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_3.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_3.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_3.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_3.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_3.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_3_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_3.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_3_win, 2, 0, 1, 1)
        for r in range(2, 3):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_waterfall_sink_x_2 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            (slot_width * 0.5), #fc
            slot_width, #bw
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_2.set_update_time(waterfall_update_band)
        self.qtgui_waterfall_sink_x_2.enable_grid(False)
        self.qtgui_waterfall_sink_x_2.enable_axis_labels(False)

        self.qtgui_waterfall_sink_x_2.disable_legend()


        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_2.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_2.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_2.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_2.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_2.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_2_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_2.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_2_win, 1, 1, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_waterfall_sink_x_1 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            (slot_width * 0.5), #fc
            slot_width, #bw
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_1.set_update_time(waterfall_update_band)
        self.qtgui_waterfall_sink_x_1.enable_grid(True)
        self.qtgui_waterfall_sink_x_1.enable_axis_labels(False)

        self.qtgui_waterfall_sink_x_1.disable_legend()


        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_1.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_1.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_1.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_1.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_1.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_1_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_1.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_1_win, 1, 0, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_waterfall_sink_x_0 = qtgui.waterfall_sink_c(
            2048, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            rf_center, #fc
            rf_bandwidth, #bw
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_0.set_update_time(waterfall_update_main)
        self.qtgui_waterfall_sink_x_0.enable_grid(True)
        self.qtgui_waterfall_sink_x_0.enable_axis_labels(True)

        self.qtgui_waterfall_sink_x_0.disable_legend()


        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [6, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_0.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_0.set_intensity_range(-140, 10)

        self._qtgui_waterfall_sink_x_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_0.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_0_win, 0, 0, 1, 2)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.network_udp_sink_3_0_0_0_0_0_0 = network.udp_sink(gr.sizeof_gr_complex, 1, '127.0.0.1', 13004, 0, 1472, False)
        self.network_udp_sink_3_0_0_0_0_0 = network.udp_sink(gr.sizeof_gr_complex, 1, '127.0.0.1', 13004, 0, 1472, False)
        self.network_udp_sink_3_0_0_0_0 = network.udp_sink(gr.sizeof_gr_complex, 1, '127.0.0.1', 13004, 0, 1472, False)
        self.network_udp_sink_3_0_0_0 = network.udp_sink(gr.sizeof_gr_complex, 1, '127.0.0.1', 13004, 0, 1472, False)
        self.network_udp_sink_3_0_0 = network.udp_sink(gr.sizeof_gr_complex, 1, '127.0.0.1', 13004, 0, 1472, False)
        self.network_udp_sink_3_0 = network.udp_sink(gr.sizeof_gr_complex, 1, '127.0.0.1', 13004, 0, 1472, False)
        self.network_udp_sink_3 = network.udp_sink(gr.sizeof_gr_complex, 1, '127.0.0.1', 13004, 0, 1472, False)
        self.network_udp_sink_2 = network.udp_sink(gr.sizeof_gr_complex, 1, '127.0.0.1', 12003, 0, 1472, False)
        self.network_udp_sink_1 = network.udp_sink(gr.sizeof_gr_complex, 1, '127.0.0.1', 12002, 0, 1472, False)
        self.network_udp_sink_0 = network.udp_sink(gr.sizeof_gr_complex, 1, '127.0.0.1', 12001, 0, 1472, False)
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0_0 = filter.freq_xlating_fir_filter_ccc(1, firdes.complex_band_pass(1, samp_rate, -samp_rate/(2*decimation), samp_rate/(2*decimation), slot_width), (ft8_10m - (samp_rate * 0.5)), samp_rate)
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0 = filter.freq_xlating_fir_filter_ccc(1, firdes.complex_band_pass(1, samp_rate, -samp_rate/(2*decimation), samp_rate/(2*decimation), slot_width), (ft8_12m - (samp_rate * 0.5)), samp_rate)
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0 = filter.freq_xlating_fir_filter_ccc(1, firdes.complex_band_pass(1, samp_rate, -samp_rate/(2*decimation), samp_rate/(2*decimation), slot_width), (ft8_15m - (samp_rate * 0.5)), samp_rate)
        self.freq_xlating_fir_filter_xxx_3_0_0_0 = filter.freq_xlating_fir_filter_ccc(1, firdes.complex_band_pass(1, samp_rate, -samp_rate/(2*decimation), samp_rate/(2*decimation), slot_width), (ft8_17m - (samp_rate * 0.5)), samp_rate)
        self.freq_xlating_fir_filter_xxx_3_0_0 = filter.freq_xlating_fir_filter_ccc(1, firdes.complex_band_pass(1, samp_rate, -samp_rate/(2*decimation), samp_rate/(2*decimation), slot_width), (ft8_17m - (samp_rate * 0.5)), samp_rate)
        self.freq_xlating_fir_filter_xxx_3_0 = filter.freq_xlating_fir_filter_ccc(1, firdes.complex_band_pass(1, samp_rate, -samp_rate/(2*decimation), samp_rate/(2*decimation), slot_width), (ft8_20m - (samp_rate * 0.5)), samp_rate)
        self.freq_xlating_fir_filter_xxx_3 = filter.freq_xlating_fir_filter_ccc(1, firdes.complex_band_pass(1, samp_rate, -samp_rate/(2*decimation), samp_rate/(2*decimation), slot_width), (ft8_40m - (samp_rate * 0.5)), samp_rate)
        self.freq_xlating_fir_filter_xxx_2 = filter.freq_xlating_fir_filter_ccc(1, firdes.complex_band_pass(1, samp_rate, -samp_rate/(2*decimation), samp_rate/(2*decimation), slot_width), (ft8_60m - (samp_rate * 0.5)), samp_rate)
        self.freq_xlating_fir_filter_xxx_1 = filter.freq_xlating_fir_filter_ccc(1, firdes.complex_band_pass(1, samp_rate, -samp_rate/(2*decimation), samp_rate/(2*decimation), slot_width), (ft8_80m - (samp_rate * 0.5)), samp_rate)
        self.freq_xlating_fir_filter_xxx_0 = filter.freq_xlating_fir_filter_ccc(1, firdes.complex_band_pass(1, samp_rate, -samp_rate/(2*decimation), samp_rate/(2*decimation), slot_width), (ft8_160m - (samp_rate * 0.5)), samp_rate)


        ##################################################
        # Connections
        ##################################################
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.network_udp_sink_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.qtgui_waterfall_sink_x_1, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_1, 0), (self.network_udp_sink_1, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_1, 0), (self.qtgui_waterfall_sink_x_2, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_2, 0), (self.network_udp_sink_2, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_2, 0), (self.qtgui_waterfall_sink_x_3, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3, 0), (self.network_udp_sink_3, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3, 0), (self.qtgui_waterfall_sink_x_4, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0, 0), (self.network_udp_sink_3_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0, 0), (self.qtgui_waterfall_sink_x_4_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0_0, 0), (self.network_udp_sink_3_0_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0_0, 0), (self.qtgui_waterfall_sink_x_4_0_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0_0_0, 0), (self.network_udp_sink_3_0_0_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0_0_0, 0), (self.qtgui_waterfall_sink_x_4_0_0_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0_0_0_0, 0), (self.network_udp_sink_3_0_0_0_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0_0_0_0, 0), (self.qtgui_waterfall_sink_x_4_0_0_0_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0, 0), (self.network_udp_sink_3_0_0_0_0_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0, 0), (self.qtgui_waterfall_sink_x_4_0_0_0_0_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0_0, 0), (self.network_udp_sink_3_0_0_0_0_0_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0_0, 0), (self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.freq_xlating_fir_filter_xxx_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.freq_xlating_fir_filter_xxx_1, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.freq_xlating_fir_filter_xxx_2, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.freq_xlating_fir_filter_xxx_3, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.freq_xlating_fir_filter_xxx_3_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.freq_xlating_fir_filter_xxx_3_0_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.freq_xlating_fir_filter_xxx_3_0_0_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.freq_xlating_fir_filter_xxx_3_0_0_0_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.qtgui_waterfall_sink_x_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "ft8capture")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_rf_bandwidth(self):
        return self.rf_bandwidth

    def set_rf_bandwidth(self, rf_bandwidth):
        self.rf_bandwidth = rf_bandwidth
        self.set_rf_center(self.rf_bandwidth/2)
        self.qtgui_waterfall_sink_x_0.set_frequency_range(self.rf_center, self.rf_bandwidth)
        self.uhd_usrp_source_0.set_bandwidth(self.rf_bandwidth, 0)

    def get_waterfall_update_main(self):
        return self.waterfall_update_main

    def set_waterfall_update_main(self, waterfall_update_main):
        self.waterfall_update_main = waterfall_update_main
        self.qtgui_waterfall_sink_x_0.set_update_time(self.waterfall_update_main)

    def get_waterfall_update_band(self):
        return self.waterfall_update_band

    def set_waterfall_update_band(self, waterfall_update_band):
        self.waterfall_update_band = waterfall_update_band
        self.qtgui_waterfall_sink_x_1.set_update_time(self.waterfall_update_band)
        self.qtgui_waterfall_sink_x_2.set_update_time(self.waterfall_update_band)
        self.qtgui_waterfall_sink_x_3.set_update_time(self.waterfall_update_band)
        self.qtgui_waterfall_sink_x_4.set_update_time(self.waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0.set_update_time(self.waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0_0.set_update_time(self.waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0_0_0.set_update_time(self.waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0_0_0_0.set_update_time(self.waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0.set_update_time(self.waterfall_update_band)
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.set_update_time(self.waterfall_update_band)

    def get_slot_width(self):
        return self.slot_width

    def set_slot_width(self, slot_width):
        self.slot_width = slot_width
        self.freq_xlating_fir_filter_xxx_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_1.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_2.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.qtgui_waterfall_sink_x_1.set_frequency_range((self.slot_width * 0.5), self.slot_width)
        self.qtgui_waterfall_sink_x_2.set_frequency_range((self.slot_width * 0.5), self.slot_width)
        self.qtgui_waterfall_sink_x_3.set_frequency_range((self.slot_width * 0.5), self.slot_width)
        self.qtgui_waterfall_sink_x_4.set_frequency_range((self.slot_width * 0.5), self.slot_width)
        self.qtgui_waterfall_sink_x_4_0.set_frequency_range((self.slot_width * 0.5), self.slot_width)
        self.qtgui_waterfall_sink_x_4_0_0.set_frequency_range((self.slot_width * 0.5), self.slot_width)
        self.qtgui_waterfall_sink_x_4_0_0_0.set_frequency_range((self.slot_width * 0.5), self.slot_width)
        self.qtgui_waterfall_sink_x_4_0_0_0_0.set_frequency_range((self.slot_width * 0.5), self.slot_width)
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0.set_frequency_range((self.slot_width * 0.5), self.slot_width)
        self.qtgui_waterfall_sink_x_4_0_0_0_0_0_0.set_frequency_range((self.slot_width * 0.5), self.slot_width)

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.freq_xlating_fir_filter_xxx_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_0.set_center_freq((self.ft8_160m - (self.samp_rate * 0.5)))
        self.freq_xlating_fir_filter_xxx_1.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_1.set_center_freq((self.ft8_80m - (self.samp_rate * 0.5)))
        self.freq_xlating_fir_filter_xxx_2.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_2.set_center_freq((self.ft8_60m - (self.samp_rate * 0.5)))
        self.freq_xlating_fir_filter_xxx_3.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3.set_center_freq((self.ft8_40m - (self.samp_rate * 0.5)))
        self.freq_xlating_fir_filter_xxx_3_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0.set_center_freq((self.ft8_20m - (self.samp_rate * 0.5)))
        self.freq_xlating_fir_filter_xxx_3_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0.set_center_freq((self.ft8_17m - (self.samp_rate * 0.5)))
        self.freq_xlating_fir_filter_xxx_3_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0.set_center_freq((self.ft8_17m - (self.samp_rate * 0.5)))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0.set_center_freq((self.ft8_15m - (self.samp_rate * 0.5)))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0.set_center_freq((self.ft8_12m - (self.samp_rate * 0.5)))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0_0.set_center_freq((self.ft8_10m - (self.samp_rate * 0.5)))

    def get_rf_samp_rate(self):
        return self.rf_samp_rate

    def set_rf_samp_rate(self, rf_samp_rate):
        self.rf_samp_rate = rf_samp_rate
        self.uhd_usrp_source_0.set_samp_rate(self.rf_samp_rate)

    def get_rf_gain(self):
        return self.rf_gain

    def set_rf_gain(self, rf_gain):
        self.rf_gain = rf_gain
        self.uhd_usrp_source_0.set_gain(self.rf_gain, 0)

    def get_rf_center(self):
        return self.rf_center

    def set_rf_center(self, rf_center):
        self.rf_center = rf_center
        self.qtgui_waterfall_sink_x_0.set_frequency_range(self.rf_center, self.rf_bandwidth)
        self.uhd_usrp_source_0.set_center_freq(self.rf_center, 0)

    def get_ft8_80m(self):
        return self.ft8_80m

    def set_ft8_80m(self, ft8_80m):
        self.ft8_80m = ft8_80m
        self.freq_xlating_fir_filter_xxx_1.set_center_freq((self.ft8_80m - (self.samp_rate * 0.5)))

    def get_ft8_60m(self):
        return self.ft8_60m

    def set_ft8_60m(self, ft8_60m):
        self.ft8_60m = ft8_60m
        self.freq_xlating_fir_filter_xxx_2.set_center_freq((self.ft8_60m - (self.samp_rate * 0.5)))

    def get_ft8_40m(self):
        return self.ft8_40m

    def set_ft8_40m(self, ft8_40m):
        self.ft8_40m = ft8_40m
        self.freq_xlating_fir_filter_xxx_3.set_center_freq((self.ft8_40m - (self.samp_rate * 0.5)))

    def get_ft8_30m(self):
        return self.ft8_30m

    def set_ft8_30m(self, ft8_30m):
        self.ft8_30m = ft8_30m

    def get_ft8_20m(self):
        return self.ft8_20m

    def set_ft8_20m(self, ft8_20m):
        self.ft8_20m = ft8_20m
        self.freq_xlating_fir_filter_xxx_3_0.set_center_freq((self.ft8_20m - (self.samp_rate * 0.5)))

    def get_ft8_17m(self):
        return self.ft8_17m

    def set_ft8_17m(self, ft8_17m):
        self.ft8_17m = ft8_17m
        self.freq_xlating_fir_filter_xxx_3_0_0.set_center_freq((self.ft8_17m - (self.samp_rate * 0.5)))
        self.freq_xlating_fir_filter_xxx_3_0_0_0.set_center_freq((self.ft8_17m - (self.samp_rate * 0.5)))

    def get_ft8_160m(self):
        return self.ft8_160m

    def set_ft8_160m(self, ft8_160m):
        self.ft8_160m = ft8_160m
        self.freq_xlating_fir_filter_xxx_0.set_center_freq((self.ft8_160m - (self.samp_rate * 0.5)))

    def get_ft8_15m(self):
        return self.ft8_15m

    def set_ft8_15m(self, ft8_15m):
        self.ft8_15m = ft8_15m
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0.set_center_freq((self.ft8_15m - (self.samp_rate * 0.5)))

    def get_ft8_12m(self):
        return self.ft8_12m

    def set_ft8_12m(self, ft8_12m):
        self.ft8_12m = ft8_12m
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0.set_center_freq((self.ft8_12m - (self.samp_rate * 0.5)))

    def get_ft8_10m(self):
        return self.ft8_10m

    def set_ft8_10m(self, ft8_10m):
        self.ft8_10m = ft8_10m
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0_0.set_center_freq((self.ft8_10m - (self.samp_rate * 0.5)))

    def get_decimation(self):
        return self.decimation

    def set_decimation(self, decimation):
        self.decimation = decimation
        self.freq_xlating_fir_filter_xxx_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_1.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_2.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))
        self.freq_xlating_fir_filter_xxx_3_0_0_0_0_0_0.set_taps(firdes.complex_band_pass(1, self.samp_rate, -self.samp_rate/(2*self.decimation), self.samp_rate/(2*self.decimation), self.slot_width))

    def get_antenna(self):
        return self.antenna

    def set_antenna(self, antenna):
        self.antenna = antenna




def main(top_block_cls=ft8capture, options=None):

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()

    tb.show()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()

if __name__ == '__main__':
    main()
