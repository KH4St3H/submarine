import serial
import time
import json
import socket


import serial

rpi_ip = '0.0.0.0'
rpi_port = 12345

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(0.1)
sock.bind((rpi_ip, rpi_port))


arduino = serial.Serial(port='/dev/ttyACM0',   baudrate=29000, timeout=0.1)


class ControllerData:
    def __init__(self, hats=(0, 0), left_joystick=(0, 0), right_joystick=(0, 0), thrust_left=0, thrust_right=0, buttons=0) -> None:
        self.hats = tuple(hats)
        self.left_joystick = tuple(left_joystick)
        self.right_joystick = tuple(right_joystick)

        self.thrust_left = thrust_left
        self.thrust_right = thrust_right

        self.buttons = buttons

    @classmethod
    def fromjsonstring(cls, string: str):
        js = json.loads(string)

        return cls(js['hats'], js['left_joystick'], js['right_joystick'], js['thrust_left'], js['thrust_right'], js['B'])

    def __str__(self) -> str:
         return (f'{self.hats=}{self.left_joystick=}{self.right_joystick=}{self.thrust_right=}{self.thrust_left=}{self.buttons=}')

    def tojson(self):
         return json.dumps({
             'hats': self.hats,
             'lj': self.left_joystick,
             'rj': self.right_joystick,
             #'thrust_left': self.thrust_left,
             #'thrust_right': self.thrust_right,
             'B': self.buttons,
             'check': 15})

    def __eq__(self, other):
        threshold = 0.08
        if abs(self.left_joystick[0] - other.left_joystick[0]) > threshold:
            return False

        if abs(self.left_joystick[1] - other.left_joystick[1]) > threshold:
            return False

        if abs(self.right_joystick[1] - other.right_joystick[1]) > threshold:
            return False

        if abs(self.right_joystick[0] - other.right_joystick[0]) > threshold:
            return False

        if abs(self.thrust_left - other.thrust_left) > threshold:
            return False

        if abs(self.thrust_right - other.thrust_right) > threshold:
            return False

        if self.hats != other.hats:
            return False

        if self.buttons != other.buttons:
            return False

        return True


def write_read(x):
    arduino.write(bytes(x,   'utf-8'))

def read():

    data = arduino.readline()
 
    if not data:
        return
    if (d := data.decode()).startswith('#debug'):
        print(d)
        return None
    try:
        dt = json.loads(data.decode())
        arduino.reset_input_buffer()
    except Exception:
        dt = None
    return dt

old_data = ControllerData()
addr = None
try:
    while True:

        if arduino.in_waiting > 0:
            try:
                data = read()
            except Exception:
                continue

            if data:
                if addr:
                    data = json.dumps(data)
                    sock.sendto(data.encode(), addr)
                # print(data)

        try:
            data, addr = sock.recvfrom(1024)  # ﺩﺭیﺎﻔﺗ ﺩﺍﺪﻫ
        except TimeoutError as toe:
            write_read('{"check": 1}\n')
            # write_read(old_data.tojson() + '\n')
            continue

        data = data.decode()
        controller_data = ControllerData.fromjsonstring(data)
        # print(data)
        # print(json_data)
        if controller_data != old_data:
            cj = controller_data.tojson() + '\n'
            print('new data:', cj)

            callback = write_read(cj)
            old_data = controller_data


except KeyboardInterrupt:
    sock.close()
