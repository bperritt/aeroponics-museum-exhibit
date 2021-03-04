
import serial
import struct
import time
gi.require_version('Gtk', '3.0') #make sure the correct gtk version is running
from gi.repository import Gtk, GdkPixbuf, Gdk #import libraries from gi
time_string = "" #makes strings used in GUI global and empty
water_string = ""
food_string = ""
trad_water_string = ""
trad_food_string = ""
png = ''


class ResultsWindow(Gtk.Window): #class for result screen
   def __init__(self): #constructor for results screen
       Gtk.Window.__init__(self, title="Results Screen!") #titles gui
       self.set_default_size(1920, 1080) #sets size of window to monitor resolution
       vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing = 20) #creates a vertical box array for gui elements
       vbox.set_homogeneous(False) #elements do not take up even space
       hbox_top = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing = 10) #creates horizontal boxes in top vertical box
       hbox_top.set_homogeneous(True) #elements are evenly spaced
       hbox_bottom = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing = 100) #creates horizontal boxes in bottom vertical box
       hbox_bottom.set_homogeneous(False) #elements are not evenly spaced
       vbox_left = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing = 10) #creates left vertical boxes
       vbox_left.set_homogeneous(True) #elements take up even space
       vbox_leftmid = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing = 10) #creates left middle vertical boxes
       vbox_leftmid.set_homogeneous(True) #elements take up even space
       vbox_rightmid = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing = 10) #creates right middle vertical boxes
       vbox_rightmid.set_homogeneous(True) #elements take up even space
       vbox_right = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing = 10) #creates right vertical boxes
       vbox_right.set_homogeneous(True) #elements take up even space
      
       vbox.pack_start(hbox_top, True, True, 0) #packs top horizontal boxes into vertical boxes
       vbox.pack_start(hbox_bottom, True, True, 0) #packs bottom horizontal boxes into vertical boxes
       hbox_bottom.pack_start(vbox_left, True, True, 0) #packs left vertical boxes into bottom horizontal box
       hbox_bottom.pack_start(vbox_leftmid, True, True, 0) #packs left middle vertical boxes into bottom horizontal box
       hbox_bottom.pack_start(vbox_rightmid, True, True, 0) #packs right middle vertical boxes into bottom horizontal box
       hbox_bottom.pack_start(vbox_right, True, True, 0) #packs right vertical boxes into bottom hoirzontal box
      
       label = Gtk.Label() #creates new text field
       label.set_markup("<span foreground = 'white' font_size='100000'>Game Over!</span>") #sets text and properties
       hbox_top.pack_start(label, True, True, 0) #packs it into top horizontal box
      
       label = Gtk.Label() #creates new text field
       str = "<span foreground = 'white' font_size='30000'>Your plants lived for\n" #starts constructing string with text elements
       str += time_string #adds time variable
       str += " seconds!</span>" #finishes string
       label.set_markup(str) #sets properties
       vbox_leftmid.pack_start(label, True, True, 0) #packs text field into top of left middle box
      
       label = Gtk.Label() #same as above below other previous text field
       str = "<span foreground = 'white' font_size='30000'>You used "
       str += water_string
       str += "\ngallons of water</span>"
       label.set_markup(str)
       vbox_leftmid.pack_start(label, True, True, 0)
      
       label = Gtk.Label() #same as above below previous text field
       str = "<span foreground = 'white' font_size='30000'>You grew "
       str += food_string
       str += " meals worth \nof food with Aeroponics!</span>"
       label.set_markup(str)
       vbox_leftmid.pack_start(label, True, True, 0)        



       pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale('D:/images/dead_plant.png', 200, 200, True) #sets image properties
       image = Gtk.Image() #creates new image
       image.set_from_pixbuf(pixbuf) #sets image based on properties  
       vbox_rightmid.pack_start(image, True, True, 0) #packs image into top of right middle box



       label = Gtk.Label() #creates another label and packs it below image
       str = "<span foreground = 'white' font_size='30000'>Traditional farming methods use\n"
       str += trad_water_string
       str += " gallons of water</span>"
       label.set_markup(str)
       vbox_rightmid.pack_start(label, True, True, 0)   



       label = Gtk.Label() #creates another label and packs it below previous label
       str = "<span foreground = 'white' font_size='30000'>Traditional methods only produce\n"
       str += trad_food_string
       str += " meals worth of food</span>"
       label.set_markup(str)
       vbox_rightmid.pack_start(label, True, True, 0)       
      
       self.add(vbox) #adds conglomeration of boxes to the GUI
      
arduino = serial.Serial('COM8', 9600, timeout=1) #establishes connection to arduino                 
while True: #loops until conditions are met
   try:
       byte_structure = arduino.read(size = 6) #try to read 6 bytes from arduino serial port     
   except:
       byte_structure = ""; #if there's nothing to read byte_structure is an empty string
   if len(byte_structure) == 6: #if byte_structure is exactly 6 bytes long
           time, plants, water_used = struct.unpack('<HHH', byte_structure) #unpack the struct passed through the serial port into 3 unsigned shorts
           break #break out of loop
                
food_produced = int(time * plants / 10) #calculate food produced       
normal_water = int(water_used * 20) #calculate water used w/ traditional farming
normal_food = int(food_produced / 3) #calculate food that would normally be produced
time_string = str(time) #turns time into a string
food_string = str(food_produced) #turns food produced into a string    
water_string = str(water_used) #turns water used into a string
trad_water_string = str(normal_water) #turns water traditionally used into a string
trad_food_string = str(normal_food) #turns food traditionally produced into a string
win = ResultsWindow() #constructs new resultsWindow
col = Gdk.RGBA(0,0,0) #sets color to black
state = Gtk.StateFlags(0) #sets state to normal
win.override_background_color(state,col) #sets the backgroud color to black
win.fullscreen() #fullscreens window
win.connect("destroy", Gtk.main_quit) #if the X button is clicked close the window
win.show_all() #show the contents of the window
Gtk.main() #main called to continuously show window until quit event

