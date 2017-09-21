# TimeDrawingRobot
Highlights:
- Time via Internet (WiFi)
- Alternatively / as a backup via RTC (RealTimeClock)
- Driven by 2 (small) Nema17 stepper (arm) and a cheap BY28 for cleaning
- Pen is driven by a micro servo (RC)
- LEDs to confirm Status etc.

The sketches are done with Fusion360 as CAD System
A lot of the mechanics and parts I had chosen because I had those parts. 
The TimeDrawingRobo could be realized in many other ways

I used a ESP-32 Mocrocontroller and a cheap RTC.

The stepper for cleaning could easily be driven with a UL2003 driver.

For the axes I used carbon pipes, as used in RC builds; and radial bearings.

For the steppers I use 12v. This is stepped down (with a cheap chinese part) to 3.3V for the ESP-32.