import wx
import gui
import legs

class ActualImportDialog(gui.ImportDialog):
    def __init__(self, parent, bootloader):
        gui.ImportDialog.__init__(self, parent)
        self.bootloader = bootloader
        self.surveys = legs.get_surveys(legs.read_legs())
        for idx, s in enumerate(self.surveys.values()):
            self.survey_list.Append([s['time'].strftime("%Y.%m.%d %H:%M"), s['station_count'], s['leg_count']])
            self.survey_list.SetItemData(idx, s['survey'])
        self.survey_list.SortItems(self.sort_list)
        self.survey_list.SetColumnWidth(0,-1)
        self.survey_list.SetColumnWidth(1,-2)
        self.survey_list.SetColumnWidth(2,-2)
        
    def sort_list(self, item1, item2):
        return self.surveys[item1]['time'] < self.surveys[item2]['time']
        
    def get_text(self, survey, template):
        text = ""
        text += "*date {time:%Y.%m.%d}\n*begin XXX\n*data cartesian easting northing vertical\n".format(**survey)
        for leg in survey['legs']:
            text += "{frm} {to} {delta[0]:.2f} {delta[1]:.2f} {delta[2]:.2f}\n".format(**leg.__dict__)
        text += "*end XXX\n"
        return (str(survey['time']),text)
        
    def get_texts(self, template):
        texts = []
        item = -1
        while True:
            item = self.survey_list.GetNextSelected(item)
            if item==-1: break
            texts.append(self.get_text(self.surveys[self.survey_list.GetItemData(item)], template))
        return texts
    
    def is_edit(self):
        return self.edit_btn.GetValue()

