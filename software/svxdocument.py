from wx.lib.docview import Document
import os
class SVXDocument(Document):
    def GetEditor(self):
        return self.GetFirstView().editor
    
    def LoadObject(self, f):
        self.GetEditor().SetText(f.read())
    
    def SaveObject(self, f):
        f.write(self.GetEditor().GetText())
            
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
