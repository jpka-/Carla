#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Carla Backend code (OSC stuff)
# Copyright (C) 2011-2015 Filipe Coelho <falktx@falktx.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# For a full copy of the GNU General Public License see the doc/GPL.txt file.

# ------------------------------------------------------------------------------------------------------------
# Imports (Custom)

from carla_host import *

# ------------------------------------------------------------------------------------------------------------
# Imports (liblo)

from liblo import (
  Address,
  AddressError,
  ServerError,
  Server,
  make_method,
  send as lo_send,
  TCP as LO_TCP,
  UDP as LO_UDP,
)

from random import random

# ------------------------------------------------------------------------------------------------------------
# Host OSC object

class CarlaHostOSC(CarlaHostQtPlugin):
    def __init__(self):
        CarlaHostQtPlugin.__init__(self)

        self.lo_target = None
        self.lo_target_name = ""

    # -------------------------------------------------------------------

    def printAndReturnError(self, error):
        print(error)
        self.fLastError = error
        return False

    def sendMsg(self, lines):
        if len(lines) < 1:
            return self.printAndReturnError("not enough arguments")

        method = lines.pop(0)

        if method == "set_engine_option":
            return True

        if self.lo_target is None:
            return self.printAndReturnError("lo_target is None")
        if self.lo_target_name is None:
            return self.printAndReturnError("lo_target_name is None")

        if method not in (
                          #"set_option",
                          "set_active",
                          "set_drywet",
                          "set_volume",
                          "set_balance_left",
                          "set_balance_right",
                          "set_panning",
                          #"set_ctrl_channel",
                          "set_parameter_value",
                          "set_parameter_midi_channel",
                          "set_parameter_midi_cc",
                          "set_program",
                          "set_midi_program",
                          #"set_custom_data",
                          #"set_chunk_data",
                          #"prepare_for_save",
                          #"reset_parameters",
                          #"randomize_parameters",
                          "send_midi_note"
                          ):
            return self.printAndReturnError("invalid method '%s'" % method)

        pluginId = lines.pop(0)

        args = []

        if method == "send_midi_note":
            channel, note, velocity = lines

            if velocity:
                method = "note_on"
                args   = [channel, note, velocity]
            else:
                method = "note_off"
                args   = [channel, note]

        else:
            for line in lines:
                if isinstance(line, bool):
                    args.append(int(line))
                else:
                    args.append(line)

        path = "/%s/%i/%s" % (self.lo_target_name, pluginId, method)

        print(path, args)

        lo_send(self.lo_target, path, *args)
        return True

    # -------------------------------------------------------------------

    def engine_init(self, driverName, clientName):
        return self.lo_target is not None

    def engine_close(self):
        return True

    def engine_idle(self):
        return

    def is_engine_running(self):
        return self.lo_target is not None

    def set_engine_about_to_close(self):
        return

# ------------------------------------------------------------------------------------------------------------
# OSC Control server

class CarlaControlServer(Server):
    def __init__(self, host, mode):
        Server.__init__(self, 8998 + int(random()*9000), mode)

        self.host = host

    def idle(self):
        self.fReceivedMsgs = False

        while self.recv(0) and self.fReceivedMsgs:
            pass

    def getFullURL(self):
        return "%scarla-control" % self.get_url()

    @make_method('/carla-control/add_plugin_start', 'is') # FIXME skip name
    def add_plugin_start_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, pluginName = args
        self.host._add(pluginId)
        self.host._set_pluginInfoUpdate(pluginId, {'name': pluginName})

    #@make_method('/carla-control/add_plugin_end', 'i') # FIXME skip name
    #def add_plugin_end_callback(self, path, args):
        #print(path, args)
        #self.fReceivedMsgs = True
        #pluginId, = args
        #self.host.PluginAddedCallback.emit(pluginId, "") #self.fPluginsInfo[pluginId].pluginInfo['name'])

    #@make_method('/carla-control/remove_plugin', 'i')
    #def remove_plugin_callback(self, path, args):
        #print(path, args)
        #self.fReceivedMsgs = True
        #pluginId, = args
        #self.host.PluginRemovedCallback.emit(pluginId)

    @make_method('/carla-control/set_plugin_info1', 'iiiih')
    def set_plugin_info1_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, type_, category, hints, uniqueId = args # , optsAvail, optsEnabled
        optsAvail = optsEnabled = 0x0 # FIXME

        hints &= ~PLUGIN_HAS_CUSTOM_UI

        pinfo = {
            'type': type_,
            'category': category,
            'hints': hints,
            'optionsAvailable': optsAvail,
            'optionsEnabled': optsEnabled,
            'uniqueId': uniqueId
        }

        self.host._set_pluginInfoUpdate(pluginId, pinfo)

    @make_method('/carla-control/set_plugin_info2', 'issss')
    def set_plugin_info2_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, realName, label, maker, copyright = args # , filename, name, iconName
        filename = name = iconName = "" # FIXME

        pinfo = {
            'filename': filename,
            #'name':  name, # FIXME
            'label': label,
            'maker': maker,
            'copyright': copyright,
            'iconName': iconName
        }

        self.host._set_pluginInfoUpdate(pluginId, pinfo)
        self.host._set_pluginRealName(pluginId, realName)

    @make_method('/carla-control/set_audio_count', 'iii')
    def set_audio_count_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, ins, outs = args
        self.host._set_audioCountInfo(pluginId, {'ins': ins, 'outs': outs})

    @make_method('/carla-control/set_midi_count', 'iii')
    def set_midi_count_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, ins, outs = args
        self.host._set_midiCountInfo(pluginId, {'ins': ins, 'outs': outs})

    @make_method('/carla-control/set_parameter_count', 'iii') # FIXME
    def set_parameter_count_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, ins, outs = args # , count
        count = ins + outs
        self.host._set_parameterCountInfo(pluginId, count, {'ins': ins, 'outs': outs})

    @make_method('/carla-control/set_program_count', 'ii')
    def set_program_count_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, count = args
        self.host._set_programCount(pluginId, count)

    @make_method('/carla-control/set_midi_program_count', 'ii')
    def set_midi_program_count_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, count = args
        self.host._set_midiProgramCount(pluginId, count)

    @make_method('/carla-control/set_parameter_data', 'iiiiss')
    def set_parameter_data_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, paramId, type_, hints, name, unit = args

        hints &= ~(PARAMETER_USES_SCALEPOINTS | PARAMETER_USES_CUSTOM_TEXT)

        paramInfo = {
            'name': name,
            'symbol': "",
            'unit': unit,
            'scalePointCount': 0,
        }
        self.host._set_parameterInfo(pluginId, paramId, paramInfo)

        paramData = {
            'type': type_,
            'hints': hints,
            'index': paramId,
            'rindex': -1,
            'midiCC': -1,
            'midiChannel': 0
        }
        self.host._set_parameterData(pluginId, paramId, paramData)

    @make_method('/carla-control/set_parameter_ranges1', 'iifff')
    def set_parameter_ranges1_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, paramId, def_, min_, max_ = args

        paramRanges = {
            'def': def_,
            'min': min_,
            'max': max_
        }

        self.host._set_parameterRangesUpdate(pluginId, paramId, paramRanges)

    @make_method('/carla-control/set_parameter_ranges2', 'iifff')
    def set_parameter_ranges2_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, paramId, step, stepSmall, stepLarge = args

        paramRanges = {
            'step': step,
            'stepSmall': stepSmall,
            'stepLarge': stepLarge
        }

        self.host._set_parameterRangesUpdate(pluginId, paramId, paramRanges)

    #@make_method('/carla-control/set_parameter_midi_cc', 'iii')
    #def set_parameter_midi_cc_callback(self, path, args):
        #print(path, args)
        #self.fReceivedMsgs = True
        #pluginId, paramId, cc = args
        #self.host._set_parameterMidiCC(pluginId, paramId, cc)
        #self.host.ParameterMidiCcChangedCallback.emit(pluginId, paramId, cc)

    #@make_method('/carla-control/set_parameter_midi_channel', 'iii')
    #def set_parameter_midi_channel_callback(self, path, args):
        #print(path, args)
        #self.fReceivedMsgs = True
        #pluginId, paramId, channel = args
        #self.host._set_parameterMidiChannel(pluginId, paramId, channel)
        #self.host.ParameterMidiChannelChangedCallback.emit(pluginId, paramId, channel)

    @make_method('/carla-control/set_output_parameter_value', 'iif')
    def set_parameter_value_callback(self, path, args):
        pluginId, paramId, paramValue = args

        if paramId < 0:
            self.host._set_internalValue(pluginId, paramId, paramValue)
        else:
            self.host._set_parameterValue(pluginId, paramId, paramValue)

        self.host.ParameterValueChangedCallback.emit(pluginId, paramId, paramValue)

    #@make_method('/carla-control/set_default_value', 'iif')
    #def set_default_value_callback(self, path, args):
        #print(path, args)
        #self.fReceivedMsgs = True
        #pluginId, paramId, paramValue = args
        #self.host._set_parameterDefault(pluginId, paramId, paramValue)
        #self.host.ParameterDefaultChangedCallback.emit(pluginId, paramId, paramValue)

    #@make_method('/carla-control/set_current_program', 'ii')
    #def set_current_program_callback(self, path, args):
        #print(path, args)
        #self.fReceivedMsgs = True
        #pluginId, current = args
        #self.host._set_currentProgram(pluginId, current)
        #self.host.ProgramChangedCallback.emit(pluginId, current)

    #@make_method('/carla-control/set_current_midi_program', 'ii')
    #def set_current_midi_program_callback(self, path, args):
        #print(path, args)
        #self.fReceivedMsgs = True
        #pluginId, current = args
        #self.host._set_currentMidiProgram(pluginId, current)
        #self.host.MidiProgramChangedCallback.emit(pluginId, current)

    @make_method('/carla-control/set_program_name', 'iis')
    def set_program_name_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, progId, progName = args
        self.host._set_programName(pluginId, progId, progName)

    @make_method('/carla-control/set_midi_program_data', 'iiiis')
    def set_midi_program_data_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        pluginId, midiProgId, bank, program, name = args
        self.host._set_midiProgramData(pluginId, midiProgId, {'bank': bank, 'program': program, 'name': name})

    #@make_method('/carla-control/note_on', 'iiii')
    #def set_note_on_callback(self, path, args):
        #print(path, args)
        #self.fReceivedMsgs = True
        #pluginId, channel, note, velocity = args
        #self.host.NoteOnCallback.emit(pluginId, channel, note, velocity)

    #@make_method('/carla-control/note_off', 'iii')
    #def set_note_off_callback(self, path, args):
        #print(path, args)
        #self.fReceivedMsgs = True
        #pluginId, channel, note = args
        #self.host.NoteOffCallback.emit(pluginId, channel, note)

    @make_method('/carla-control/set_peaks', 'iffff')
    def set_peaks_callback(self, path, args):
        self.fReceivedMsgs = True
        pluginId, in1, in2, out1, out2 = args
        self.host._set_peaks(pluginId, in1, in2, out1, out2)

    @make_method('/carla-control/exit', '')
    def set_exit_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        self.host.QuitCallback.emit()

    @make_method('/carla-control/exit-error', 's')
    def set_exit_error_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        error, = args
        self.host.lo_target = None
        self.host.QuitCallback.emit()
        self.host.ErrorCallback.emit(error)

    @make_method('/carla-control/callback', 'iiiiifs')
    def set_callback_callback(self, path, args):
        print(path, args)
        self.fReceivedMsgs = True
        action, pluginId, value1, value2, value3, valuef, valueStr = args
        engineCallback(self.host, action, pluginId, value1, value2, value3, valuef, valueStr)

    @make_method(None, None)
    def fallback(self, path, args):
        print("ControlServer::fallback(\"%s\") - unknown message, args =" % path, args)
        self.fReceivedMsgs = True

# ------------------------------------------------------------------------------------------------------------
# Main Window

class HostWindowOSC(HostWindow):
    def __init__(self, host, oscAddr):
        HostWindow.__init__(self, host, True)
        self.host = host

        if False:
            # kdevelop likes this :)
            host = CarlaHostPlugin()
            self.host = host

        # ----------------------------------------------------------------------------------------------------
        # Internal stuff

        self.fOscServer = None

        # ----------------------------------------------------------------------------------------------------
        # Connect actions to functions

        self.ui.act_file_connect.triggered.connect(self.slot_fileConnect)
        self.ui.act_file_refresh.triggered.connect(self.slot_fileRefresh)

        # ----------------------------------------------------------------------------------------------------
        # Final setup

        if oscAddr:
            QTimer.singleShot(0, self.connectOsc)

    def connectOsc(self, addr = None):
        if addr is not None:
            self.fOscAddress = addr

        lo_protocol    = LO_UDP if self.fOscAddress.startswith("osc.udp") else LO_TCP
        lo_target_name = self.fOscAddress.rsplit("/", 1)[-1]

        err = None
        print("Connecting to \"%s\" as '%s'..." % (self.fOscAddress, lo_target_name))

        try:
            lo_target = Address(self.fOscAddress)
            self.fOscServer = CarlaControlServer(self.host, lo_protocol)
            lo_send(lo_target, "/register", self.fOscServer.getFullURL())

        except AddressError as e:
            err = e
        except OSError as e:
            err = e
        except:
            err = { 'args': [] }

        if err is not None:
            del self.fOscServer
            self.fOscServer = None

            fullError = self.tr("Failed to connect to the Carla instance.")

            if len(err.args) > 0:
                fullError += " %s\n%s\n" % (self.tr("Error was:"), err.args[0])

            fullError += "\n"
            fullError += self.tr("Make sure the remote Carla is running and the URL and Port are correct.") + "\n"
            fullError += self.tr("If it still does not work, check your current device and the remote's firewall.")

            CustomMessageBox(self,
                             QMessageBox.Warning,
                             self.tr("Error"),
                             self.tr("Connection failed"),
                             fullError,
                             QMessageBox.Ok,
                             QMessageBox.Ok)
            return

        self.host.lo_target = lo_target
        self.host.lo_target_name = lo_target_name
        self.ui.act_file_refresh.setEnabled(True)

        self.startTimers()

    def disconnectOsc(self):
        self.killTimers()

        if self.host.lo_target is not None:
            try:
                lo_send(self.host.lo_target, "/unregister", self.fOscServer.getFullURL())
            except:
                pass

        if self.fOscServer is not None:
            del self.fOscServer
            self.fOscServer = None

        self.removeAllPlugins()

        self.host.lo_target = None
        self.host.lo_target_name = ""
        self.ui.act_file_refresh.setEnabled(False)

    # --------------------------------------------------------------------------------------------------------
    # Timers

    def startTimers(self):
        if self.fIdleTimerOSC == 0:
            self.fIdleTimerOSC = self.startTimer(20)

        HostWindow.startTimers(self)

    def restartTimersIfNeeded(self):
        if self.fIdleTimerOSC != 0:
            self.killTimer(self.fIdleTimerOSC)
            self.fIdleTimerOSC = self.startTimer(20)

        HostWindow.restartTimersIfNeeded(self)

    def killTimers(self):
        if self.fIdleTimerOSC != 0:
            self.killTimer(self.fIdleTimerOSC)
            self.fIdleTimerOSC = 0

        HostWindow.killTimers(self)

    # --------------------------------------------------------------------------------------------------------

    def removeAllPlugins(self):
        self.host.fPluginsInfo = []
        HostWindow.removeAllPlugins(self)

    # --------------------------------------------------------------------------------------------------------

    def loadSettings(self, firstTime):
        settings = HostWindow.loadSettings(self, firstTime)
        self.fOscAddress = settings.value("RemoteAddress", "osc.tcp://127.0.0.1:22752/Carla", type=str)

    def saveSettings(self):
        settings = HostWindow.saveSettings(self)
        if self.fOscAddress:
            settings.setValue("RemoteAddress", self.fOscAddress)

    # --------------------------------------------------------------------------------------------------------

    @pyqtSlot()
    def slot_fileConnect(self):
        dialog = QInputDialog(self)
        dialog.setInputMode(QInputDialog.TextInput)
        dialog.setLabelText(self.tr("Address:"))
        dialog.setTextValue(self.fOscAddress or "osc.tcp://127.0.0.1:22752/Carla")
        dialog.setWindowTitle(self.tr("Carla Control - Connect"))
        dialog.resize(400,1)

        ok = dialog.exec_()
        addr = dialog.textValue().strip()
        del dialog

        if not ok:
            return
        if not addr:
            return

        self.disconnectOsc()

        if addr:
            self.connectOsc(addr)

    @pyqtSlot()
    def slot_fileRefresh(self):
        if self.host.lo_target is None or self.fOscServer is None:
            return

        self.killTimers()
        self.removeAllPlugins()

        lo_send(self.host.lo_target, "/unregister", self.fOscServer.getFullURL())
        lo_send(self.host.lo_target, "/register", self.fOscServer.getFullURL())

        self.startTimers()

    @pyqtSlot()
    def slot_handleQuitCallback(self):
        HostWindow.slot_handleQuitCallback(self)
        self.disconnectOsc()

    # --------------------------------------------------------------------------------------------------------

    def timerEvent(self, event):
        if event.timerId() == self.fIdleTimerOSC:
            self.fOscServer.idle()

            if self.host.lo_target is None:
                self.disconnectOsc()

        HostWindow.timerEvent(self, event)

    def closeEvent(self, event):
        self.killTimers()

        if self.host.lo_target is not None and self.fOscServer is not None:
            try:
                lo_send(self.host.lo_target, "/unregister", self.fOscServer.getFullURL())
            except:
                pass

        HostWindow.closeEvent(self, event)

# ------------------------------------------------------------------------------------------------------------
