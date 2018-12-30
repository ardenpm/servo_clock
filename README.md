# Servo Clock Controller Software for Arduino

![Servo Clock](images/servo_clock.jpg?raw=true "Servo-Controlled Digital Clock")

## TL;DR

To get running enable calibration mode to get your pins and angles, enter them in _configuration.h_, disable calibration mode and upload the Sketch. Take particular care with respect to powering the servos and have fun.

## Background

(Otvinta)[http://www.otvinta.com/] have created an amazing 3D printed, servo controlled digital clock. You can check out the full details of their build (here)[http://www.otvinta.com/download14.html] along with detailed instructions and STL files for printing.

The original project uses a [Raspberry Pi 3](https://raspberrypi.org) running [Windows IoT Core](https://developer.microsoft.com/en-us/windows/iot) which talks to multiple [Pololu Maestro](https://www.pololu.com/product/1356) servo controllers over USB to move the servos. A custom application (binary only) is provided which reads the time and sets the clock.

Since the Raspberry Pi does not have a Real-time Clock, it does not retain the time when powered off and the correct time must be set from an [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) server to give the correct time. This means an Internet connection is required to use the clock. It was also observed that Windows IoT inexplicably ran the clock faster than real-time in some situations.

As an alternative, this software allows the Raspberry Pi and Maestro controllers to be replaced by a [MEGA 2560](http://www.keyestudio.com/ks0342.html) control board, [MEGA Sensor Shield](http://www.keyestudio.com/ks0006.html) and a [DS3231](http://www.keyestudio.com/keyestudio-clock-module-for-arduino.html) real-time clock. The ATmega2560 microcontroller is ideal for this application as it has the large number of pins needed to control the servos.

## Hardware and Hookup

Hookup diagram is under construction.

## Power Requirements

The servos require independent power from the microcontroller in order to function. While one or two servos can be powered directly from the controller, 28 is out of the question. The MEGA Sensor Shield was chosen because it has an independent power input header over which a dedicated 5V supply can be provided.

Even with separate power, if all 28 SG90 servos are operating at full power at once, there is potential to draw nearly 25A which is unlikely to be able to be handed by anything less than a 125W 5V power supply (to say nothing of the PCB traces and connecting wires). This is worst case scenario with all digits changing at once, more or less continuously and is very unlikely to be reached in practice.

To dramatically reduce the power requirements, this clock application staggers the switching of the individual segments so that not all of them are being changed at once. In addition to creating an interesting effect, it reduces the total power used during a full change of all digits to less than 2A maximum. This allows the clock and microcontroller to be powered by a much smaller 10W 5V power supply or possibly 20W 5V supply if being conservative.

The MEGA Sensor Shield must be configured with a jumper to ensure it uses external power for the servos. **Make sure you check this when setting up**. The microcontroller can be powered by USB, however if it is desired to use the same supply for everything in the project you will need a boost converter since the MEGA 2560 expects 7V-12V when being driven from an external source and not the USB port.

## Calibration

The application comes with a mode to allow you to individually select a servo pin and change its current angle. This can be used to determine the correct pins, on angle and off angles for use in the configuration (see below). To enable calibration mode uncomment the following line:

```
// Uncomment to enable calibration mode to find servo pins and angles
#define CALIBRATION_MODE
```

Upload the code to your microcontroller and then open the serial monitor. The default baud rate is _9600_ but can be changed in _configuration.h_. You should see something like this after uploading.

```
Servo Calibration Mode. Ensure newline is enabled in serial monitor!
Enter servo pin number: 
```

As advised ensure you have set the line endings option to _newline_ in the serial monitor. You can now type the pin number of the servo you want to test and calibrate into the serial monitor, for example _51_ and press enter. It will then prompt you as follows.

```
Enter servo angle:
```

Just line the pin number you can now type the angle you want and press enter. It is recommended to chose _90_ degrees if you have not yet calibrated the servo since this has been found to place it in a safe location.

The program will now repeatedly ask for the pin number and angle. You can use this to confirm which pin each segment is connected to as well as the angle required to turn the segment on and off. This information should be recorded for later use in the configuration file.

## Configuration and Customisation

The file _configuration.h_ contains all of the configuration that potentially varies between clocks. After performing the calibration above you need to populate the configuration file with your specific values. It will look something like this.

```
const int digit_configuration[digit_count][3][7] = {
  // digit 0
  {
    {39,  37,  35,  33,  31,  29,  27}, // pins
    {53,  52,  127, 45,  123, 55,  45}, // on angles
    {139, 135, 35,  140, 35,  140, 130} // off angles
  },
  // digit 1
  {
    {38,  36,  34,  32,  30,  28,  26}, // pins
    {60,  57,  135, 45,  135, 45,  45}, // on angles
    {139, 135, 50,  140, 45,  140, 130} // off angles
  },
  // digit 2
  {
    {53,  51,  49,  47,  45,  43,  41}, // pins
    {53,  45,  133, 40,  130, 45,  45}, // on angles
    {139, 135, 50,  135, 45,  135, 130} // off angles
  },
  // digit 3
  {
    {52,  50,  48,  46,  44,  42,  40}, // pins
    {60,  47,  132, 50,  127, 50,  55}, // on angles
    {139, 135, 50,  140, 40,  135, 135} // off angles
  }
};
```

The digits on the clock are laid out left to right, so _01:23_. For each digit you must provide an array of the pin numbers, corresponding to where the servo is connected, the angles required to turn each segment on and the angles required to turn each segment off. There are 7 entries for each of these items for each digit corresponding to all segments for the digit. There are 28 segments in total.

The segment layout within a digit is as follows. The numbers represent the indices into the segment arrays. Each of the four digits uses the same segment layout.

```
     0
   -----
  |     |
1 |     | 2
  |  3  |
   -----
  |     |
4 |     | 5
  |     |
   -----
     6
```
