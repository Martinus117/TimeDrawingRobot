# TimeDrawingRobot
Highlights:
- Time via Internet (NTP) (WiFi)
- Driven by 2 (small) Nema17 stepper (arm) and a cheap BY28 for cleaning
- Pen is driven by a micro servo (RC)
- LEDs to confirm Status etc.

The 'working plane' is a cheap children's toy which can be written on with a tiny magnet. I bought the table via Ali Express (see below).

The sketches are done with Fusion360 as CAD System
A lot of the mechanics and parts I have chosen because I had those parts. 
The TimeDrawingRobo could be realized in many other ways

I used a ESP-32 Mocrocontroller.

The stepper for cleaning could easily be driven with a UL2003 driver.

For the axes I used carbon pipes, as used in RC builds; and radial bearings.

For the steppers I use 12v. This is stepped down (with a cheap chinese part) to 3.3V for the ESP-32.

Letters
*******
With the simulation under C# / Visual Studio (Community Edition) I produced a stream of points wich defines the letters. Each point consists of x- and y-coordinate and a marker, if the pen should be up or down. The group delimiter is a semicolon, the delimiter inside a group a coma.
Bsp (the new-line is inserted for better readability): 
30.77,22.73,0;
30.45,21.27,;
30.03,20.58,;
29.45,19.91,;
28.72,19.27,;
27.81,18.65,;
...
0 means down (with the magnet to write), empty means nothing to do, 1 means up.
