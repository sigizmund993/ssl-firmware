## EAGLES
Firmware for SSL robots for team EAGLES (Щеглы).

Designed for Arduino Mega 2560

## Description
after power is supplied, imu and nrf are automaticly initialized.
if kicker is active indicator shows dash
if autokick is enabled indicator draws dot
blue LED is on if ball is in front of the kicker
indicator shows selected channel
if battery is low, motors keep running, but indicator blinks with letter "L"
if battery is critical, motors stop, and indicator shows letter "L"
if robot is rotated 90 degrees or more on pitch or roll axis (robot fell), motors stop to prevent hurting surrounding.
if enter button is pressed, kicker kicks
yaw axis has stabilisation (holds one absolute angle)


## Supported features:
- 3 DC motors with incremental encoders, LPF for encoder values, PID speed control for each motor
- Kicker
- Dribbler ball presence sensor
- Seven segment indicator for showing radio channel number and other info
- Buttons for changing radio channel number and other purposes
- Battery voltmeter for undervoltage protection
- 2 LEDs
- NRF
- MPU-9250

## TODO:

