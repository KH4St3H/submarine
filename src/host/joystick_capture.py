"""
  Code to be ran on host machine, it captures 
controller events and sends them to Raspberry PI
via udp.
"""

import pygame
import socket
import time
import json
from threading import Thread

from monitor import Monitor


rpi_ip ='192.168.1.100'
# rpi_ip = '192.168.72.38'
rpi_port = 12345

# udp client setting
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
rpi_address = (rpi_ip, rpi_port)

monitor = Monitor()


# udp server settings
#sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 9000))

pygame.init()

# first available controller
controller = pygame.joystick.Joystick(0)
controller.init()


def pretty(i):
    data = controller.get_axis(i)
    return float(f'{data:.3f}')

def pack_buttons():
    return sum([controller.get_button(i)*1<<i for i in range(controller.get_numbuttons())])


def send_loop():
    while event := pygame.event.poll():
        if event.type == pygame.JOYAXISMOTION or event.type == pygame.JOYBUTTONDOWN:
            data = {
                "B": pack_buttons(),
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


sock.settimeout(0.05)
t = Thread(target=send_loop)
try:
    while True:
        if not t.is_alive():
            t = Thread(target=send_loop)
            t.start()
        try:
            data, addr = sock.recvfrom(1024)
            if data:
                monitor.update_json(data.decode())
        except TimeoutError as toe:
            continue

except KeyboardInterrupt:
    controller.quit()
    sock.close()
