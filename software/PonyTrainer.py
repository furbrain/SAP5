#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import wx
import gui
import legs


class ActualMainFrame(gui.MainFrame):

    def sort_import_list(self, item1, item2):
        return self.surveys[item1]['time'] < self.surveys[item2]['time']

    def Import(self, event):
        all_legs = legs.read_legs()
        self.surveys = legs.get_surveys(all_legs)
        dlg = gui.ImportDialog(self)
        for idx, s in enumerate(self.surveys.values()):
            dlg.survey_list.Append([s['time'].strftime("%Y.%m.%d %H:%M"), s['station_count'], s['leg_count']])
            dlg.survey_list.SetItemData(idx, s['survey'])
        dlg.survey_list.SortItems(self.sort_import_list)
        dlg.survey_list.SetColumnWidth(0,-1)
        dlg.survey_list.SetColumnWidth(1,-2)
        dlg.survey_list.SetColumnWidth(2,-2)
        if dlg.ShowModal()==wx.ID_OK:
            item = -1
            while True:
                item = dlg.survey_list.GetNextSelected(item)
                if item==-1: break
                print self.surveys[dlg.survey_list.GetItemData(item)]
    
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
