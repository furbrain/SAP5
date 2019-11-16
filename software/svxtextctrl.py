import wx
import wx.stc
import re
import os

COMMAND_STYLE=10
VARIABLE_STYLE=11
STRING_STYLE=12
COMMENT_STYLE=13
ERROR_STYLE=14

class SVXTextCtrl(wx.stc.StyledTextCtrl):
    COMMANDS = ("alias",
                "begin",
                "calibrate",
                "case",
                "copyright",
                "cs",
                "data",
                "date",
                "declination",
                "default",
                "end",
                "entrance",
                "equate",
                "export",
                "fix",
                "flags",
                "include",
                "infer",
                "instrument",
                "prefix",
                "ref",
                "require",
                "sd",
                "set",
                "solve",
                "team",
                "title",
                "truncate",
                "units")
                
    def __init__(self, parent, id=wx.ID_ANY, text="", filename=None):
        wx.stc.StyledTextCtrl.__init__(self, parent, id)
        cdb = wx.TheColourDatabase
        self.SetTabWidth(8)
        self.SetText(text)
        self.filename = filename
        self.SetLexer(wx.stc.STC_LEX_CONTAINER)
        self.Bind(wx.stc.EVT_STC_STYLENEEDED, self.OnStyleNeeded)
        
        self.StyleSetFont(wx.stc.STC_STYLE_DEFAULT, wx.Font(14, wx.MODERN,
wx.NORMAL, wx.NORMAL))
        self.StyleClearAll() 
        self.StyleSetBold(COMMAND_STYLE, True)
        self.StyleSetForeground(COMMAND_STYLE, cdb.Find("Maroon"))
        self.StyleSetForeground(VARIABLE_STYLE, cdb.Find("Dark Turquoise"))
        self.StyleSetForeground(STRING_STYLE, cdb.Find("Magenta"))
        self.StyleSetItalic(COMMENT_STYLE, True)
        self.StyleSetForeground(COMMENT_STYLE, cdb.Find("Grey"))
        self.StyleSetBackground(ERROR_STYLE, cdb.Find("Red"))
        self.named = bool(filename)
        
    def GetTitle(self):
        if self.named:
            fname = os.path.basename(self.filename)
        else:
            if self.filename:
                fname = self.filename
            else:
                fname = "Untitled"
        if self.IsModified():
            return "*" + fname
        else:
            return fname
        
    def OnStyleNeeded(self, event):
        line_start = self.LineFromPosition(self.GetEndStyled())
        line_end = self.LineFromPosition(event.GetPosition())
        for line in range(line_start, line_end+1):
            self.style_line(line)

    def style_line(self, line):
        start_pos = self.GetLineEndPosition(line-1)+1
        text = self.GetLine(line)
        self.StartStyling(start_pos, 255)
        self.SetStyling(len(text), 0)
        
        #highlight commands
        command = re.match(r"^\s*\*(\w+)",text)
        if command:
            if command.group(1) in self.COMMANDS:
                self.StartStyling(start_pos, 255)
                self.SetStyling(len(command.group()), COMMAND_STYLE)
                self.SetStyling(len(text) - len(command.group()), VARIABLE_STYLE)
            else:
                self.StartStyling(start_pos, 255)
                self.SetStyling(len(command.group()), ERROR_STYLE)
                
        #highlight strings
        for match in re.finditer(r'".*?"', text):
            self.StartStyling(match.start() + start_pos, 255)
            self.SetStyling(len(match.group()), STRING_STYLE)
            
        #highlight comments
        in_string=False
        for i, char in enumerate(text):
            if char == '"':
                in_string = not in_string
            if char == ';':
                if not in_string:
                    self.StartStyling(i + start_pos, 255)
                    self.SetStyling(len(text) - i, COMMENT_STYLE)
                    break
