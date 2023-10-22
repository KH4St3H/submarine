import pygame
import socket
import time
import json

# تنظیم اتصال به Raspberry Pi
rpi_ip ='192.168.171.38'
#rpi_ip = '127.0.0.1'
rpi_port = 12345

# تنظیم اتصال UDP
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
rpi_address = (rpi_ip, rpi_port)

pygame.init()

# تشخیص دسته PS4
controller = pygame.joystick.Joystick(0)
controller.init()

def pretty(i):
    data = controller.get_axis(i)
    return float(f'{data:.2f}')

try:
    while True:
        while event := pygame.event.poll():
            if event.type == pygame.JOYAXISMOTION or event.type == pygame.JOYBUTTONDOWN:
                # خواندن مقادیر آنالوگ دسته PS4
                axis_x = controller.get_axis(0)
                axis_y = controller.get_axis(1)

                #print(f"Axis X: {axis_x}, Axis Y: {axis_y}")

                # خواندن مقادیر دکمه‌ها و آنالوگ‌ها
                data = {
                    "buttons": [controller.get_button(i) for i in range(controller.get_numbuttons())],
                    #"hats": [controller.get_hat(i) for i in range(controller.get_numhats())],
                    "left_joystick": (pretty(0), pretty(1)),
                    "right_joystick":(pretty(3), pretty(4)),
                    "thrust_left": pretty(2), # -1 to 1
                    "thrust_right": pretty(5), # -1 to 1
                }
                data_string = json.dumps(data) 
                print(data_string)
                sock.sendto(data_string.encode(), rpi_address)
                time.sleep(0.1)
except KeyboardInterrupt:
    controller.quit()
    sock.close()
