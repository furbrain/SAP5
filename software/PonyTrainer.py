#!/usr/bin/env python3
# -*- coding: UTF-8 -*-
import wx
import wx.stc
from wx.lib.docview import DocManager, DocTemplate, DOC_NEW
import gui
import importer
import svxtextctrl
from svxview import SVXView
from svxdocument import SVXDocument

class ActualMainFrame(gui.PonyFrame):
    def Import(self, event):
        dlg = importer.ActualImportDialog(self, None)
        if dlg.ShowModal()==wx.ID_OK:
            texts = dlg.get_texts(None)
            for title, text in texts:
                doc = self._docManager.CreateDocument("",flags=DOC_NEW)
                doc.SetTitle(title)
                doc.SetText(text)
                doc.GetFirstView().GetFrame().SetTitle(title)
        
    def Quit(self, event):
        self.Close()
    
    def DeviceSettings(self, event):
        dlg = gui.DeviceSettingsDialog(self)
        dlg.ShowModal()
    
    def About(self, event):
        gui.AboutDialog(self).ShowModal()
                        
PonyTrainer = wx.App(False)
docmanager = DocManager()
template = DocTemplate(docmanager, "Survex files", "*.svx", "", "svx", "Svx Doc", "Svx View", SVXDocument, SVXView)
docmanager.AssociateTemplate(template)
frame = ActualMainFrame(docmanager, None, wx.ID_ANY, "")
PonyTrainer.SetTopWindow(frame)
frame.Show()
PonyTrainer.MainLoop()        
