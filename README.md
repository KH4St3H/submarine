
# Submarine
Collection of codes for operating an underwater vehicle
## Features  
- Control thrust with left joystick
- Burst thrust for rotation with R1 and L2 buttons
- Tilting device with right joystick for manoeuvrability
- Tilting in different directions with buttons
- Ascending and descending with controller hat

## pinout
```
Arduino Due:

| Pin number    | Component          | name  |
| ------------- |:------------------:| ----- |
| 26            | right thrust motor | ESC   |
| 27            | left thrust motor  | ESC   |
| 25            | front-right motor  | ESC   |
| 24            | front-left motor   | ESC   |
| 23            | back-right motor   | ESC   |
| 22            | back-left motor    | ESC   |
| GND           |      *****         | GND   |
| 19 (RX1)      | Raspberry PI       | TX    |
| 18 (TX1)      | Raspberry PI       | RX    |
```
## License  
[MIT](https://choosealicense.com/licenses/mit/)  
