#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import wx
import gui
import legs
import importer

class ActualMainFrame(gui.MainFrame):

    def Import(self, event):
        dlg = importer.ActualImportDialog(self, None)
        if dlg.ShowModal()==wx.ID_OK:
            texts = dlg.get_texts(None)
            for t in texts:
                print "-----"
                print t
                
    
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
