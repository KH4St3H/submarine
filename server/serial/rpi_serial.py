import serial
import time
import json
import socket

rpi_ip = '0.0.0.0'
rpi_port = 12345

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((rpi_ip, rpi_port))


arduino = serial.Serial(port='/dev/ttyACM1',   baudrate=9600, timeout=0.01)


class ControllerData:
    def __init__(self, left_joystick=(0, 0), right_joystick=(0, 0), thrust_left=0, thrust_right=0, buttons=[i for i in range(13)]) -> None:
        self.left_joystick = left_joystick
        self.right_joystick = right_joystick

        self.thrust_left = thrust_left
        self.thrust_right = thrust_right

        self.buttons = buttons

    @classmethod
    def fromjsonstring(cls, string: str):
        js = json.loads(string)

        return cls(js['left_joystick'], js['right_joystick'], js['thrust_left'], js['thrust_right'], js['buttons'])

    def __str__(self) -> str:
         return (f'{self.left_joystick=}{self.right_joystick=}{self.thrust_right=}{self.thrust_left=}{self.buttons=}')

    def tojson(self):
         return json.dumps({
             'left_joystick': self.left_joystick,
             'right_joystick': self.right_joystick,
             'thrust_left': self.thrust_left,
             'thrust_right': self.thrust_right,
             'buttons': self.buttons})

    def __eq__(self, other):
        threshold = 0.2
        if abs(self.left_joystick[0] - other.left_joystick[0]) < threshold:
            return False

        if abs(self.left_joystick[1] - other.left_joystick[1]) < threshold:
            return False

        if abs(self.right_joystick[1] - other.right_joystick[1]) < threshold:
            return False

        if abs(self.right_joystick[0] - other.right_joystick[0]) < threshold:
            return False

        if abs(self.thrust_left - other.thrust_left) < threshold:
            return False

        if abs(self.thrust_right - other.thrust_right) < threshold:
            return False

        for i in range(len(self.buttons)):
            if other.buttons[i] != self.buttons[i]:
                return False

        return True


def write_read(x):
    arduino.write(bytes(x,   'utf-8'))
    time.sleep(0.05)
    data = arduino.readline()
    return data

old_data = ControllerData()
try:
    while True:
        data, addr = sock.recvfrom(1024)  # ﺩﺭیﺎﻔﺗ ﺩﺍﺪﻫ
        data = data.decode()
        controller_data = ControllerData.fromjsonstring(data)
        json_data = json.loads(data)
        # print(data)
        # print(json_data)
        if controller_data != old_data:
            print(controller_data)
            callback = write_read(controller_data.tojson())
            print(callback)
            old_data = controller_data

        # ﺩﺭ ﺍیﻦﺟﺍ ﻡی<200c>ﺕﻭﺎﻧیﺩ ﺩﺍﺪﻫ<200c>ﻫﺍی ﺩﺭیﺎﻔﺗی ﺭﺍ پﺭﺩﺍﺰﺷ کﻥیﺩ
        #print(f"Received data: {data}")
except KeyboardInterrupt:
    sock.close()

