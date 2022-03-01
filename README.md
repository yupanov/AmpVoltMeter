# SolarFridgeStats
Home solar station controller with DC fridge as a load (arduino).

The solar station contains:
- 100W solar panel
- 65 aH 12 v battery
- a cheap PWM battery charger controller
- 100 liters fridge (it was a small normal fridge from 1950's, now all the elements are put to the styrofoam box, I only extended the capillary tube and replaced the compressor, now it's a SIKELAN 75 W DC compressor)

- Mini360 DC-DC step-down converter
- Ardiuno nano
- 2 ACS712 current measure modules
- SD card module
- 2 10 kOhm thermistors
- relay module
- LCD 20x4 i2c display

The controller collects data from the system:
- Voltage
- Load and charge current
- Evaporator and fridge room temerature

Controls fridge cycles, stops at -16 and starts at 6 deg Celcius the compressor.
Saves the data in the SD card as comma-separated values .CSV file
 
 ![SolarStatsScreenshot](https://user-images.githubusercontent.com/62822363/156231102-6a906a42-65c7-4dce-a7ad-8a9d5a406dfe.png)
![IMG_20220301_150021ss](https://user-images.githubusercontent.com/62822363/156231280-30c072a5-58ab-4d48-830a-6262a222c0d8.png)
![Electronics](https://user-images.githubusercontent.com/62822363/156231298-cf0c0bfc-be71-44b6-bcc7-f5bef8c6439f.png)
![IMG_20220301_150102ss](https://user-images.githubusercontent.com/62822363/156231315-5d014a1c-e97a-4027-8f38-f17e1c71a9f4.png)
