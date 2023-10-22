"""
  Code to be ran on host machine, it captures 
controller events and sends them to Raspberry PI
via udp.
"""

import pygame
import socket
import time
import json

rpi_ip ='192.168.171.38'
rpi_port = 12345

# udp setting
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
rpi_address = (rpi_ip, rpi_port)

pygame.init()

# first available controller
controller = pygame.joystick.Joystick(0)
controller.init()

def pretty(i):
    data = controller.get_axis(i)
    return float(f'{data:.3f}')

try:
    while True:
        while event := pygame.event.poll():
            if event.type == pygame.JOYAXISMOTION or event.type == pygame.JOYBUTTONDOWN:
                data = {
                    "buttons": [controller.get_button(i) for i in range(controller.get_numbuttons())],
                    "hats": controller.get_hat(0),
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
