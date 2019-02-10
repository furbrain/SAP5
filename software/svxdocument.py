from wx.lib.docview import Document

class SVXDocument(Document):
    def GetEditor(self):
        return self.GetFirstView().editor
        
    def OnSaveDocument(self, filename):
        if not self.GetEditor().SaveFile(filename):
            return False
        self.Modify(False)
        self.SetDocumentSaved(True)
        
    def OnOpenDocument(self, filename):
        if not self.GetEditor().LoadFile(filename):
            return False
        self.SetFilename(filename, True)
        self.Modify(False)
        self.UpdateAllViews()
        return True
        
    def IsModified(self):
        return Document.IsModified(self) or self.GetEditor().IsModified()
        
    def Modify(self, mod):
        Document.Modify(self,mod)
        if mod:
            self.GetEditor().MarkDirty()
        else:
            self.GetEditor().SetSavePoint()
            
    def SetText(self, text):
        self.GetEditor().SetText(text)
