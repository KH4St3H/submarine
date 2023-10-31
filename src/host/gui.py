import tkinter as tk

from math import sin, cos, radians

class CompassFrame:
    def __init__(self, root, headvar):

        self._angle = 0

        canvas = tk.Canvas(root, width=200, height=200)
        canvas.pack()

        # Draw the compass rose
        canvas.create_oval(25, 25, 175, 175, outline="blue", width=3)
        canvas.create_text(100, 10, text="N", fill="blue")
        canvas.create_text(100, 190, text="S", fill="blue")
        canvas.create_text(10, 100, text="W", fill="blue")
        canvas.create_text(190, 100, text="E", fill="blue")

        heading_label = tk.Label(root, textvariable=headvar, font=("helvetica", 12))
        heading_label.pack(pady=10)
        self.canvas = canvas

    def draw(self):
        self.canvas.delete("needle")
        self.canvas.create_line(100, 100, 100 + 50 * sin(radians(self._angle)), 100 - 50 * cos(radians(self._angle)), fill="red", width=3, tags="needle")

    @property
    def angle(self):
        return self._angle

    @angle.setter
    def angle(self, val):
        self._angle = val
        self.draw()
        pass


class MonitorFrame(tk.Frame):
    def __init__(self, container):
        super().__init__(container)
        self.root = container
        self._create_compass()
        self._create_speed()
        self._create_angle()
    
    def update_angle(self, x=None, y=None):
        print(x, y)
        if x:
            self.angle_x.set(int(x))
        if y:
            self.angle_y.set(int(y))

    def update_speed(self, x=None, y=None, z=None):
        if x:
            self._speed_x.set(x)
        if y:
            self._speed_y.set(y)
        if z:
            self._speed_z.set(z)

    def _create_angle(self):
        self.angle_x = tk.IntVar()
        self.angle_y = tk.IntVar()

        self.scale_x = tk.Scale(self, orient='horizontal', length=200, from_=-90, to=90, state='disabled', variable=self.angle_x)
        self.scale_x.grid(row=5, columnspan=3, column=0)

        self.scale_y = tk.Scale(self, orient='vertical', from_=-90, to=90, length=200, state='disabled', variable=self.angle_y)
        self.scale_y.grid(row=0, rowspan=5, column=3)

    def _create_speed(self):
        self._speed_x = tk.DoubleVar(self)
        self._speed_y = tk.DoubleVar(self)
        self._speed_z = tk.DoubleVar(self)

        # Create a label to display the compass heading
        compass_label = tk.Label(self, text=f"X speed:", font=("helvetica", 12))
        heading_label = tk.Label(self, textvariable=self._speed_x, font=("helvetica", 12))
        compass_label.grid(row=2, column=0, pady=10, padx=10)
        heading_label.grid(row=2, column=1, pady=10, padx=10)

        compass_label = tk.Label(self, text=f"Y speed:", font=("helvetica", 12))
        heading_label = tk.Label(self, textvariable=self._speed_y, font=("helvetica", 12))
        compass_label.grid(row=3, column=0, pady=10, padx=10)
        heading_label.grid(row=3, column=1, pady=10, padx=10)

        compass_label = tk.Label(self, text=f"Z speed:", font=("helvetica", 12))
        heading_label = tk.Label(self, textvariable=self._speed_z, font=("helvetica", 12))
        compass_label.grid(row=4, column=0, pady=10, padx=10)
        heading_label.grid(row=4, column=1, pady=10, padx=10)

        separator = tk.Frame(self, bg="black", height=1, bd=0)
        separator.grid(row=6, column=0, columnspan=3,sticky='ew')


    def _create_compass(self):
        self._heading = 0
        self.headvar = tk.DoubleVar(self)

        separator = tk.Frame(self, bg="black", height=1, bd=0)
        separator.grid(row=1, column=0, columnspan=3,sticky='ew')

        self.compass = CompassFrame(self.root, self.headvar)
        #update_button = tk.Button(self, text="Update Heading", command=self.uc)
        #update_button.grid(row=7, column=0)

    def update_compass(self, angle):
        self._heading = angle % 360
        self.headvar.set(self._heading)
        self.compass.angle = self._heading

if __name__=='__main__':
    root = tk.Tk()
    root.geometry('600x600')
    frame = MonitorFrame(root)
    frame.pack()
    root.mainloop()
