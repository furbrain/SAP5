#!/usr/bin/env python3
# -*- coding: UTF-8 -*-
import wx
import wx.stc
import os
import sys
import datetime
import webbrowser

import gui
import importer
import svxtextctrl
import bootloader
import config
import calibration
import json
import struct_parser
from functools import partial


class ActualMainFrame(gui.PonyFrame):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.notebook.DeleteAllPages()
        self.bootloader = None
        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.check_bootloader, self.timer)
        self.timer.Start(1000)
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        
    def create_pane(self, pane=None, ctrl = None, focus=True):
        if pane is None:
            new_page = True
            pane = wx.Panel(self.notebook, wx.ID_ANY)
        else:
            new_page = False
        if ctrl is None:
            ctrl = svxtextctrl.SVXTextCtrl(pane)
        else:
            ctrl.Reparent(pane)
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(ctrl, 1, wx.EXPAND, 0)
        pane.SetSizer(sizer)
        pane.ctrl = ctrl
        ctrl.Bind(wx.stc.EVT_STC_SAVEPOINTREACHED, lambda x: self.set_pane_title(pane, ctrl))
        ctrl.Bind(wx.stc.EVT_STC_SAVEPOINTLEFT, lambda x: self.set_pane_title(pane, ctrl))
        if new_page:
            self.notebook.AddPage(pane, ctrl.GetTitle())
        if focus:
            index = self.notebook.FindPage(pane)
            self.notebook.SetSelection(index)
        return pane
        
    def get_active_ctrl(self):
        index = self.notebook.GetSelection()
        pane = self.notebook.GetPage(index)
        return pane.ctrl
        
    def set_pane_title(self, pane, ctrl):
        index = self.notebook.FindPage(pane)
        self.notebook.SetPageText(index, ctrl.GetTitle())
        
    def check_bootloader(self, event):
        if self.bootloader is None:
            try:
                self.bootloader = bootloader.Programmer()
            except bootloader.ProgrammerError:
                pass
            else:
                self.frame_statusbar.SetStatusText("Connected to " + self.bootloader.get_name(), 0)
        else:
            try:
                self.bootloader.read_program(0x9d000000,1)
            except IOError:
                self.bootloader = None
                self.frame_statusbar.SetStatusText("Disconnected", 0)
               
    def no_pony_error(self):
        msg = wx.MessageDialog(self, "No Pony attached").ShowModal()
                
    def Import(self, event):
        if self.bootloader is None:
            self.no_pony_error()
            return
        dlg = importer.ActualImportDialog(self, self.bootloader)
        if dlg.ShowModal()==wx.ID_OK:
            texts = dlg.get_texts(None)
            for title, text in texts:
                ctrl = svxtextctrl.SVXTextCtrl(self, text = text, filename = title)
                self.create_pane(ctrl=ctrl)
                ctrl.named = False
        
    def OnClose(self, event):
        if event.CanVeto():
            for i in range(self.notebook.GetPageCount()):
                pane = self.notebook.GetPage(i)
                if not pane.ctrl.CanClose():
                    event.Veto()
                    return
        self.Destroy()
        
    def OnQuit(self, event):
        self.Close()
    
    def DeviceSettings(self, event):
        dlg = gui.DeviceSettingsDialog(self)
        dlg.ShowModal()
        
    def DeviceGetCalibration(self, event):
        if self.bootloader is None:
            self.no_pony_error()
            return
        data = {'name': self.bootloader.get_name(),
                'shots': calibration.read_cal(self.bootloader),
                'conf': config.get_config(self.bootloader)}
        with wx.FileDialog(self, "Save Calibration", wildcard="Calibration file (*.cal)|*.cal",
                               style=wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT) as fileDialog:
            if fileDialog.ShowModal() == wx.ID_CANCEL:
                return
            try:
                with open(fileDialog.GetPath(),"w") as f:
                     json.dump(data,f,cls=struct_parser.StructEncoder)
            except IOError:                 
                wx.MessageDialog(self, "Failed to save file:\n%s" % e).ShowModal()
                
    def About(self, event):
        gui.AboutDialog(self).ShowModal()

    def OnNew(self, event):  # wxGlade: PonyFrame.<event_handler>
        self.create_pane()

    def OnOpen(self, event):  # wxGlade: PonyFrame.<event_handler>
        with wx.FileDialog(self, "Open SVX file", wildcard="Survex files (*.svx)|*.svx",
                               style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST) as fileDialog:

                if fileDialog.ShowModal() == wx.ID_CANCEL:
                    return     # the user changed their mind

                # Proceed loading the file chosen by the user
                pathname = fileDialog.GetPath()
                ctrl = svxtextctrl.SVXTextCtrl(self, filename=pathname)
                try:
                    ctrl.LoadFile(pathname)
                except IOError:
                    wx.MessageDialog(self, "Failed to load file:\n%s" % e).ShowModal()
                else:
                    self.create_pane(ctrl=ctrl)

    def OnRevert(self, event):  # wxGlade: PonyFrame.<event_handler>
        ctrl = self.get_active_ctrl()
        if not ctrl.named: return
        if ctrl.IsModified():
            message = "Revert changes to %s" % os.path.basename(ctrl.filename)
            if wx.MessageBox(message, "Revert File", wx.YES_NO) == wx.YES:
                try:
                    ctrl.LoadFile(ctrl.filename)
                except IOError as e:
                    wx.MessageDialog(self, "Failed to load file:\n%s" % e).ShowModal()

    def OnSave(self, event):  # wxGlade: PonyFrame.<event_handler>
        ctrl = self.get_active_ctrl()
        ctrl.OnSave()
                    
    def OnSaveAs(self, event):  # wxGlade: PonyFrame.<event_handler>
        ctrl = self.get_active_ctrl()
        ctrl.OnSaveAs()

    def OnClosePane(self, event):  # wxGlade: PonyFrame.<event_handler>
        ctrl = self.get_active_ctrl()
        if ctrl.CanClose():
            self.notebook.DeletePage(self.notebook.GetSelection())

    def OnCut(self, event):  # wxGlade: PonyFrame.<event_handler>
        ctrl = self.get_active_ctrl()
        ctrl.Cut()

    def OnCopy(self, event):  # wxGlade: PonyFrame.<event_handler>
        ctrl = self.get_active_ctrl()
        ctrl.Copy()

    def OnPaste(self, event):  # wxGlade: PonyFrame.<event_handler>
        ctrl = self.get_active_ctrl()
        ctrl.Paste()

    def DeviceUploadFirmware(self, event):  # wxGlade: PonyFrame.<event_handler>
        if self.bootloader is None:
            self.no_pony_error()
            return
        with wx.FileDialog(self, "Open HEX file", wildcard="Hex files (*.hex)|*.hex",
                               style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST) as dlg:
            if dlg.ShowModal() == wx.ID_CANCEL:
                return
            path = dlg.GetPath()
        try:
            hexfile  = bootloader.HexFile(path)
        except IOError as e:
            wx.MessageBox("Could not open Hex File\n%s" % e)
        except bootloader.HexFileError as e:
            wx.MessageBox("Invalid Hex File\n%s" % e)
        else:
            offset = self.bootloader.user_range[0]
            maximum = self.bootloader.user_range[1]-offset
            try:
                with wx.ProgressDialog("Updating Firmware", "Writing...", maximum) as dlg:
                    self.bootloader.write_program(hexfile, set_progress=lambda x: dlg.Update(x-offset))
                    dlg.Update(0,"Verifying...")
                    self.bootloader.verify_program(hexfile,set_progress=lambda x: dlg.Update(x-offset))
                    self.bootloader.write_datetime(datetime.datetime.now())
                wx.MessageBox("Programming complete")
            except bootloader.ProgrammerError as e:
                wx.MessageBox("Firmware update failed\n%s" % e, "Error")
                
    def DeviceSetClock(self, event):
        if self.bootloader is None:
            self.no_pony_error()
            return
        dt = datetime.datetime.now()
        self.bootloader.write_datetime(dt)
        wx.MessageBox("%s Clock set to %s" % (self.bootloader.get_name(),dt.strftime("%Y-%m-%d %H:%M")))

    def resource_path(self, relative_path):
        """ Get absolute path to resource, works for dev and for PyInstaller """
        base_path = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
        return os.path.join(base_path, relative_path)

        
    def ShowManual(self, event):  # wxGlade: PonyFrame.<event_handler>
        manual = self.resource_path('manual.pdf')
        webbrowser.open(manual)
                        
PonyTrainer = wx.App(False)
frame = ActualMainFrame(None, wx.ID_ANY, "PonyTrainer")
PonyTrainer.SetTopWindow(frame)
frame.Show()
PonyTrainer.MainLoop()        
