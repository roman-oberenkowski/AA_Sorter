# Automatic AA batery
Embedded systems project

Target platform: Arduino UNO R3


Pinout and schematic:
![schematic](https://github.com/roman-oberenkowski/SW-AA_Sorter/blob/main/resources/schematic.PNG)

Symbol | Description
------------ | -------------
A | Sorting part - responsible for putting the battery into apprioprate box
 A1 | Trapdoor motor - drops the battery from cart into the box
 A2 | Driver for A1
 A3 | Driver for A4
 A4 | Old printer carriage motor - moves the cart to end up over the apprioprate box
 
Symbol | Description
------------ | -------------
B | Fetching part - responsible for fetching one battery and measureing its voltage
 B1 | Rotates fetching module to grab one battery and drop it into the cart
 B2 | Driver for B1
 B3 | Battery terminals
 B4 | Servo for thghtening the terminals together and get reliable voltage read

Fetching part demo:
![fetch](https://github.com/roman-oberenkowski/SW-AA_Sorter/blob/main/resources/fetch.gif)
Sorting part demo:
![fetch](https://github.com/roman-oberenkowski/SW-AA_Sorter/blob/main/resources/drop.gif)

Parts list:
- Arduino Uno R3
- Stepper motor 28BYJ-48 with ULN2003 driver
- Stepper motor Nema 17 with EasyDriver board
- Old printer carriage module (5V motor)
- Relay-based DC motor driver
- Servo 
- Misc. parts (wood, screws, wires...)
