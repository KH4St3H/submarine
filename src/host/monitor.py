import tkinter as tk
import json

from gui import MonitorFrame


class Monitor:
    def __init__(self) -> None:
        root = tk.Tk()
        root.geometry('800x800')
        root.title('Submarine Mintor')

        self.mf = MonitorFrame(root)
        self.mf.pack()

    def update_json(self, js: str) -> bool:
        try:
            data = json.loads(js)

        except Exception as e:
            return False

        if 'gyro' in data:
            self.mf.update_speed(*data['gyro'])

        if 'accel' in data:
            self.mf.update_angle(*data['accel'])

        if 'compassHeading' in data:
            self.mf.update_compass(data['compassHeading'])

        self.mf.root.update()
        return True

