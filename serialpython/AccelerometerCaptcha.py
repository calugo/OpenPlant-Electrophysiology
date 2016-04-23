#!/usr/bin/python
#-*- coding: utf-8 -*-
#===============================================================================
#
#         FILE: AccelerometerCaptcha.py
#
#        USAGE: ./AccelerometerCaptcha.py  
#
#  DESCRIPTION: GUI for capturing and saving Engduino Accelerometer data
#               to a .csv file.  Please install pyserial before attempting
#               to use this.
#
#      OPTIONS: ---
# REQUIREMENTS: Tkinter, pyserial
#         BUGS: ---
#        NOTES: ---
#       AUTHOR: Andreas Frangopoulos (), 
# ORGANIZATION: Engduino
#      VERSION: 1.0
#      CREATED: August 23, 2013
#     REVISION: ---
#===============================================================================


import serial, shutil, tkFileDialog, os, sys, glob, tkMessageBox, threading
from Tkinter import *
from sys import platform as _platform



class popupWindow(object):
    def __init__(self,master):
        top=self.top=Toplevel(master)
        self.l=Label(top,text="Enter your Engduino's COM Port Number")
        self.l.pack()
        self.e=Entry(top)
        self.e.focus()
        self.e.pack()
        self.b=Button(top,text='Ok',command=self.cleanup)
        self.b.pack()
        

    def cleanup(self):
        var.set("COM"+self.e.get())
        self.top.destroy()
        
class LogThread(threading.Thread):
    def run(self):
        if  var.get() == "0":
                tkMessageBox.showinfo("Error", "Device not Selected")
        else:
            addr  = var.get()
            baud  = 56000
            fname = 'accel.csv'
            fmode = 'wb'
            reps  = 500
            print "Attempting to open serial port: " + var.get()
            with serial.Serial(addr,baud) as port, open(fname,fmode) as outf:
                for i in range(reps):
                    x = port.readline()
                    print x
                    outf.write(x)
                    outf.flush()

class StdoutToWidget:
    '''
    Retrieves sys.stdout and show write calls also in a tkinter
    widget. It accepts widgets which have a "text" config and defines
    their width and height in characters. It also accepts Text widgets.
    Use stop() to stop retrieving.

    You can manage output height by using the keyword argument. By default
    the class tries to get widget\'s height configuration and use that. If
    that fails it sets self.height to None which you can also do manually.
    In this case the output will not be trimmed. However if you do not
    manage your widget, it can grow vertically hard by getting more and
    more inputs.
    '''

    # Inspired by Jesse Harris and Stathis
    # http://stackoverflow.com/a/10846997/2334951
    # http://stackoverflow.com/q/14710529/2334951

    # TODO: horizontal wrapping
    #       make it a widget decorator (if possible)
    #       height management for Text widget mode

    def __init__(self, widget, height='default', width='default'):
        self._content = []
        self.defstdout = sys.stdout
        self.widget = widget

        if height == 'default':
            try:
                self.height = widget.cget('height')
            except:
                self.height = None
        else:
            self.height = height
        if width == 'default':
            try:
                self.width = widget.cget('width')
            except:
                self.width = None
        else:
            self.width = width   

    def flush(self):
        '''
        Frame sys.stdout's flush method.
        '''
        self.defstdout.flush()

    def write(self, string, end=None):
        '''
        Frame sys.stdout's write method. This method puts the input
        strings to the widget.
        '''

        if string is not None:
            self.defstdout.write(string)
            try:
                last_line_last_char = self._content[-1][-1]
            except IndexError:
                last_line_last_char = '\n'
            else:
                if last_line_last_char == '\n':
                    self._content[-1] = self._content[-1][:-1]

            if last_line_last_char != '\n' and string.startswith('\r'):
                self._content[-1] = string[1:]
            elif last_line_last_char != '\n':
                self._content[-1] += string
            elif last_line_last_char == '\n' and string.startswith('\r'):
                self._content.append(string[1:])
            else:
                self._content.append(string)

        if hasattr(self.widget, 'insert') and hasattr(self.widget, 'see'):
            self._write_to_textwidget()
        else:
            self._write_to_regularwidget(end)

    def _write_to_regularwidget(self, end):
        if self.height is None:
            self.widget.config(text='\n'.join(self.content))
        else:
            if not end:
                content = '\n'.join(self.content[-self.height:])
            else:
                content = '\n'.join(self.content[-self.height+end:end])
            self.widget.config(text=content)

    def _write_to_textwidget(self):
        self.widget.insert('end', '\n'.join(self.content))
        self.widget.see('end')        

    def start(self):
        '''
        Starts retrieving.
        '''
        sys.stdout = self

    def stop(self):
        '''
        Stops retrieving.
        '''
        sys.stdout = self.defstdout

    @property
    def content(self):
        c = []
        for li in self._content:
            c.extend(li.split('\n'))

        if not self.width:
            return c
        else:
            result = []
            for li in c:
                while len(li) > self.width:
                    result.append(li[:self.width])
                    li = li[self.width:]
                result.append(li)
            return result

    @content.setter
    def content(self, string):
        self._content = string.split('\n')

    @property
    def errors(self):
        return self.defstdout.errors

    @property
    def encoding(self):
        return self.defstdout.encoding

class App(Frame):
    
    

    def data_log(self):
        loggerThread = LogThread()
        loggerThread.start();        
    
    def save_to_disk(self):
        dataFilePath = "accel.csv"
        fileSavePath = tkFileDialog.asksaveasfilename()
        fileSavePath += ".csv"
        shutil.copyfile(dataFilePath,fileSavePath)

        print "Saved data file ",dataFilePath, " to ", fileSavePath
    
    def createWidgets(self):   
        
           
        self.QUIT = Button(self)
        self.QUIT["text"] = "QUIT"
        self.QUIT["command"] =  self.quit
        self.QUIT.grid(row = 2, column=2)


        #self.dataframe = Canvas(self, height="200", width="200")
        #self.dataframe.grid(row=0,column = 0, columnspan=2)

        self.save_data = Button(self)
        self.save_data["text"] = "Save"
        self.save_data["command"]= self.save_to_disk
        self.save_data.grid(row=2, column=0)

        self.start_log = Button(self, width=10)
        self.start_log["text"] = "Start",
        self.start_log["command"] = self.data_log
        self.start_log.grid(row=2,column=1)

        self.cons_box = Text(self)
        self.cons_box.grid(row=1, columnspan =3)
        self.consWidg = StdoutToWidget(self.cons_box)
        self.consWidg.start()
        

    def __init__(self, master=None):
        self.dev =0;
        Frame.__init__(self, master)
        self.grid()
        self.createWidgets()
    

# create the application
def hello():
    print hello

def winDevPopup():
    devPopup = popupWindow(master=None)
    root.wait_window(devPopup.top)
root=Tk()
myapp = App(master=root)
#
# here are method calls to the window manager class
#
myapp.master.title("Accelerometer Data Logger")
menubar = Menu(root)

# create a pulldown menu, and add it to the menu bar
filemenu = Menu(menubar, tearoff=0)
filemenu.add_command(label="Save", command=hello)
filemenu.add_separator()
filemenu.add_command(label="Exit", command=root.quit)
menubar.add_cascade(label="File", menu=filemenu)

# create more pulldown menus
editmenu = Menu(menubar, tearoff=0)
menubar.add_cascade(label="Device", menu=editmenu)
devList = ["Select your Device here"]
var = StringVar()
var.set("0")

if _platform == "linux" or _platform == "linux2":
            print "linux"

elif _platform == "darwin":
            
    path = "/dev/tty.*"
    devmenu=Menu(editmenu,tearoff=0)
    for filename in glob.glob(path):
        print "Found " + filename
        devmenu.add_checkbutton(label=filename, onvalue=filename, variable=var)
    editmenu.add_cascade(label="Select Device", menu=devmenu)

elif _platform == "win32":
    editmenu.add_command(label="Select Device", command=winDevPopup)

helpmenu = Menu(menubar, tearoff=0)
helpmenu.add_command(label="About", command=hello)
menubar.add_cascade(label="Help", menu=helpmenu)

# display the menu
root.config(menu=menubar)
#myapp.master.geometry("200x")

# start the program
myapp.mainloop()
root.destroy()
