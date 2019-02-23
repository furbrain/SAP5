import math
import wx

import gui
import legs
import config


FEET_PER_METRE = 3.281
DEGREES_PER_RADIAN = 57.296
GRADS_PER_DEGREE = 1.111111111


class ActualImportDialog(gui.ImportDialog):
    def __init__(self, parent, bootloader):
        gui.ImportDialog.__init__(self, parent)
        self.bootloader = bootloader
        ###FIXME### get config details from pony
        self.config = config.Config.create_empty()
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
        
    def get_config(self):
        angle = self.angles.GetSelection()
        units = self.units.GetSelection()
        if angle==0:
            ##FIXME## get angle type from pony
            pass
        else:
            self.config.display_style = angle - 1
        if units==0:
            ##FIXME## get unit type from pony
            pass
        else:
            self.config.length_units = units - 1
            
        
    def get_date_string(self, survey):
            return "*date {time:%Y.%m.%d}\n".format(**survey)
        
    def get_units(self):
        result = ""
        if self.config.length_units==0: 
            result += "*units length metres\n"
        else:
            result += "*units length feet\n"
        if self.config.display_style==1:
            result += "*units compass grads\n"
            result += "*units clino grads\n"
        else:                    
            result += "*units compass degrees\n"
            result += "*units clino degrees\n"
        return result    
    
    def get_data_format(self):
        if self.config.display_style==2: #cartesian!
            return "*data cartesian from to easting northing vertical\n"
        else:
            return "*data normal from to tape compass clino\n"

    def get_leg_entry(self, leg):
        frm = leg.frm
        to = leg.to
        if self.config.length_units==1: #imperial
            deltas = [x*FEET_PER_METRE for x in leg.delta]
        else:
            deltas = leg.delta[:]
        if self.config.display_style==2: #cartesian
            return  "{frm} {to} {delta[0]:.2f} {delta[1]:.2f} {delta[2]:.2f}\n".format(frm=frm, to=to, delta=deltas)
        compass = math.atan2(deltas[0], deltas[1]) * DEGREES_PER_RADIAN
        if compass<0:
            compass += 360
        extension = math.hypot(deltas[0], deltas[1])
        clino = math.atan2(deltas[2], extension) * DEGREES_PER_RADIAN
        tape = math.hypot(extension, deltas[2])
        if self.config.display_style==1: #grads
            compass *= GRADS_PER_DEGREE
            clino *= GRADS_PER_DEGREE
        return  "{frm} {to} {tape:.2f} {compass:.1f} {clino:.1f}\n".format(frm=frm, to=to, tape=tape, compass=compass, clino=clino)
            
        
    def get_text(self, survey, template):
        text = "*begin XXX\n"
        text += self.get_date_string(survey)
        text += self.get_units()
        text += self.get_data_format()
        for leg in survey['legs']:
            text += self.get_leg_entry(leg)
        text += "*end XXX\n"
        return (str(survey['time']),text)
        
    def get_texts(self, template):
        texts = []
        item = -1
        self.get_config()
        while True:
            item = self.survey_list.GetNextSelected(item)
            if item==-1: break
            texts.append(self.get_text(self.surveys[self.survey_list.GetItemData(item)], template))
        return texts
    
    def is_edit(self):
        return self.edit_btn.GetValue()

