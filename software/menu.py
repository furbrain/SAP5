#!/usr/bin/python
import time
import motion
import SH1106
import bootloader
import inspect

                

def do_measurement():
    print inspect.stack()[0][3]

def do_quick_cal():
    print inspect.stack()[0][3]

def do_slow_cal():
    print inspect.stack()[0][3]

def set_metric():
    print inspect.stack()[0][3]

def set_imperial():
    print inspect.stack()[0][3]

def set_cartesian():
    print inspect.stack()[0][3]

def set_polar():
    print inspect.stack()[0][3]

def set_day():
    print inspect.stack()[0][3]

def set_night():
    print inspect.stack()[0][3]

def sleep():
    print inspect.stack()[0][3]
    for f in range(256):
        disp.set_brightness(255-f)
        time.sleep(0.01)
    disp.power_down()
    while get_input()!='double_click':
        pass
    disp.power_up()
    disp.clear_screen()
    disp.set_brightness(0)
    pages = SH1106.get_paged_text('  Hello!',spacing=4,offset=2)
    disp.send_pages(pages,page=2,column=2)
    for f in range(256):
        disp.set_brightness(f)
        time.sleep(0.01)

menu_items = (('Measure',do_measurement),
              ('Calibrate',(
                ('Quick',do_quick_cal),
                ('Slow',do_slow_cal))),
              ('Settings',(
                ('Units',(
                  ('Metric',set_metric),
                  ('Imperial',set_imperial))),
                ('Function',(
                  ('Cartesian',set_cartesian),
                  ('Polar',set_polar))),
                ('Display',(
                  ('Day',set_day),
                  ('Night',set_night))))),
              ('Off',sleep))

    
def get_input():    
    while True:
        time.sleep(0.05)
        sensors,raw_val = mpu.read_sensors()
#        if gpio.event_detected(23)==True:
#            time.sleep(1.0)
#            if gpio.event_detected(23):
#                return 'double_click'
#            else:
#                return 'single_click'

        if sensors['gyro'][1]>3000:
            if disp.inverted:
                return 'flip_down'
            else:
                return 'flip_up'
        if sensors['gyro'][1]<-3000:
            if disp.inverted:
                return 'flip_up'
            else:
                return 'flip_down'

        if sensors['gyro'][0]<-3000:
            if disp.inverted:
                return 'flip_right'
            else:
                return 'flip_left'
        if sensors['gyro'][0]>3000:
            if disp.inverted:
                return 'flip_left'
            else:
                return 'flip_right'

        if sensors['accel'][0]>8000 and disp.inverted:
            disp.flip(False)
        if sensors['accel'][0]<-8000 and not disp.inverted:
            disp.flip(True)

def scroll_text(text,direction='up'):
    paged_text = SH1106.get_paged_text(text,spacing=4,offset=2)
    if direction=='up' or direction=='down':
        up = direction=='up'
        disp.scroll_page(up=up)
        disp.scroll_pages(paged_text,up=up)
        disp.scroll_page(up=up)
        disp.scroll_page(up=up)
    elif direction=='left' or direction=='right':
        left = direction=='left'
        disp.swipe_pages(2,paged_text,left=left,rate=0)
        
def show_menu(menu,first_time=False):
    if first_time:
        scroll_text(menu[0][0])
    else:
        scroll_text(menu[0][0],direction='left')
    index = 0
    while True:
        option = get_input()
        if option=='flip_down':
            index -= 1
            index %= len(menu)
            scroll_text(menu[index][0],direction='down')
        elif option=='flip_up':
            index += 1
            index %= len(menu)
            scroll_text(menu[index][0],direction='up')
        elif option=='flip_right' or option=='single_click':
            if isinstance(menu[index][1],(list,tuple)):
                result = show_menu(menu[index][1])
            else:
                #only execute a function on button click
                if option=='single_click':
                    menu[index][1]()
                    result = True
                else:
                    continue               
            if result:
                if first_time:
                    #we've propagated back to the root menu...
                    index = 0
                    disp.clear_screen()
                    scroll_text(menu[0][0])
                else:
                    #successful function call, go back to root...
                    return True
            else:
                #we've bailed out - show ourselves again
                scroll_text(menu[index][0],direction='right')
        elif option=='flip_left':
            if not first_time:
                return False
                
#initialise various components
prog = bootloader.Programmer()
disp = SH1106.Display(prog)
disp.power_up()
disp.invert()
time.sleep(5)
disp.invert()
disp.scroll_rate = 0
disp.clear_screen()
mpu = motion.mpu9250(prog)
mpu.configure()

try:
    #sleep()
    show_menu(menu_items,True)
    print "finished"
except KeyboardInterrupt:
    disp.power_down()
    #gpio.cleanup()
