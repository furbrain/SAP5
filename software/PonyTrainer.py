#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import wx
import gui

class ActualMainFrame(gui.MainFrame):
    def Import(self, event):
        dlg = gui.ImportDialog(self)
        dlg.ShowModal()
    
    def Save(self, event):
        pass
        
    def SaveAs(self, event):
        pass
        
    def Quit(self, event):
        self.Close()
    
    def DeviceSettings(self, event):
        dlg = gui.DeviceSettingsDialog(self)
        dlg.ShowModal()
    
    def About(self, event):
        gui.AboutDialog(self).ShowModal()
                        
PonyTrainer = wx.App(False)
frame = ActualMainFrame(None, wx.ID_ANY, "")
PonyTrainer.SetTopWindow(frame)
frame.Show()
PonyTrainer.MainLoop()        
