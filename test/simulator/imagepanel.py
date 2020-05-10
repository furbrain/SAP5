import wx

class ImagePanel(wx.Panel):

    def __init__(self, parent, id):
        wx.Panel.__init__(self, parent, id)
        self.image = wx.EmptyImage(128,64,True)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        
    def paintNow(self, image):
        self.image = wx.EmptyImage(image.size[0], image.size[1])
        new_image = image.convert('RGB')
        data = new_image.tobytes()
        self.image.SetData(data)
        dc = wx.ClientDC(self)
        self.render(dc)
            
    def OnPaint(self, event=None):
        dc = wx.PaintDC(self)
        self.render(dc)
    
    def render(self, dc):
        w, h = dc.GetSize()
        bmap = wx.BitmapFromImage(self.image.Scale(w, h))
        dc.DrawBitmap(bmap, 0, 0, False)
        
