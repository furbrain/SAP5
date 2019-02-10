import wx
from wx.lib.docview import View, DocMDIChildFrame

from svxtextctrl import SVXTextCtrl

class SVXView(View):
    def OnCreate(self, doc, flags):
        topframe = wx.GetApp().GetTopWindow()
        self.frame = DocMDIChildFrame(doc, self, topframe, wx.ID_ANY, "Child Frame")
        self.SetFrame(self.frame)
        sizer = wx.BoxSizer()
        self.editor = SVXTextCtrl(self.frame)
        sizer.Add(self.editor, 1, wx.EXPAND, 0)
        self.frame.SetSizer(sizer)
        self.frame.Layout()
        self.frame.Show()
        self.editor.SetFocus()
        self.Activate(True)
        return True
        
    def OnClose(self, deleteWindow = True):
        if not self.GetDocument().Close():
            return False
        self.Activate(False)
        if deleteWindow:
            self.frame.Destroy()
            return True
        return True

