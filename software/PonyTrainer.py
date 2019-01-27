#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import wx
import wx.stc
import gui
import legs
import importer

class ActualMainFrame(gui.MainFrame):
    def __init__(self, *args, **kwargs):
        gui.MainFrame.__init__(self, *args, **kwargs)
        self.documents.DeleteAllPages()

    def Import(self, event):
        dlg = importer.ActualImportDialog(self, None)
        if dlg.ShowModal()==wx.ID_OK:
            texts = dlg.get_texts(None)
            for title, text in texts:
                doc = wx.stc.StyledTextCtrl(self.documents)
                doc.SetText(text)
                self.documents.AddPage(doc,title)

    def Cut(self, event):
        pg = self.documents.GetCurrentPage()
        if pg:
            pg.Cut()
    
    def Copy(self, event):
        pg = self.documents.GetCurrentPage()
        if pg:
            pg.Copy()
    
    def Paste(self, event):
        pg = self.documents.GetCurrentPage()
        if pg:
            pg.Paste()
    
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
