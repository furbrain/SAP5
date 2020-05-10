#!/usr/bin/python
import gui
import wx
import zmq

class zmqTimer(wx.Timer):
    def __init__(self, message_handler):
        wx.Timer.__init__(self)
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        self.socket.bind("tcp://*:5555")
        self.handler = message_handler
        
    def Notify(self):
    #  Wait for next request from client
        try:
            while True:
                message = self.socket.recv(zmq.NOBLOCK, True)
                message = [ord(x) for x in message]
                result = self.handler(message)
                if result[0] in [2,4]:
                    print("Out: ", result)
                result = bytearray(result)
                self.socket.send(result)
        except zmq.ZMQError:
            pass


app = gui.Gui()
poll = zmqTimer(app.frame.message_handler)
poll.Start(1)
app.MainLoop()
