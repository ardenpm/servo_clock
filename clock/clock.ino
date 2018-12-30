//
// Clock controller for the Otvinta Servo Clock (http://www.otvinta.com/download14.html)
//
// Author: Paul Arden <paul@arden.io>
// Created: 30th December 2018
// Dependencies: Adafruit RTCLib - https://github.com/adafruit/RTClib
// License: GNU General Public License v3.0 (GNU GPLv3)
//
// Notes:
// - Assumes use of DS3231 RTC module on default I2C pins
// - Ensure servos are powered separately (there are 28 of them)
// - If reducing the STAGGER_DELAY measure current draw to ensure it is not too high
// - Set servo pins and angles for on and off states in the setup() function
// - Very limited error checking is provided
// - See 'configuration' variable below for inputting pins and angles
//
#include <Servo.h>
#include <Wire.h>
#include "RTClib.h"

// Configuration file which should be edited for your clock
#include "configuration.h"

// Uncomment to enable calibration mode to find servo pins and angles
//#define CALIBRATION_MODE

// Our real time clock
RTC_DS3231 rtc;

// Individual segment and its associated servo
class Segment
{
    Servo servo;
    int on_angle;
    int off_angle;
  public:
    Segment(int pin, int on, int off) {
      on_angle = on;
      off_angle = off;
      servo.attach(pin);
      delay(STAGGER_DELAY);
    }
    on() {
      servo.write(on_angle);
      delay(STAGGER_DELAY);
    }
    off() {
      servo.write(off_angle);
      delay(STAGGER_DELAY);
    }
    home() {
      servo.write((int)((on_angle + off_angle) / 2.0));
      delay(STAGGER_DELAY);
    }
    ~Segment() {
      servo.detach();
    }
};

// Digit with 7 segments
class Digit
{
  public:
    static const int segment_count = 7;
    Digit(const int pins[], const int on_angles[], const int off_angles[]) {
      // NOTE: expects 7 segments will fail if more or less
      
      // Dynamically allocate all segments for the digit
      for (int i = 0; i < segment_count; i++) {
        segments[i] = new Segment(pins[i], on_angles[i], off_angles[i]);
      }
    }
    on() {
      for (int i = 0; i < segment_count; i++) {
        segments[i]->on();
      }
    }
    off() {
      for (int i = 0; i < segment_count; i++) {
        segments[i]->off();
      }
    }
    home() {
      for (int i = 0; i < segment_count; i++) {
        segments[i]->home();
      }
    }
    number(int n) {
      switch(n) {
        case 0:
          // 0 1 2 4 5 6
          segments[0]->on();
          segments[1]->on();
          segments[2]->on();
          segments[3]->off();
          segments[4]->on();
          segments[5]->on();
          segments[6]->on();
          break;
        case 1:
          // 2 5
          segments[0]->off();
          segments[1]->off();
          segments[2]->on();
          segments[3]->off();
          segments[4]->off();
          segments[5]->on();
          segments[6]->off();
          break;
        case 2:
          // 0 2 3 4 6
          segments[0]->on();
          segments[1]->off();
          segments[2]->on();
          segments[3]->on();
          segments[4]->on();
          segments[5]->off();
          segments[6]->on();
          break;
        case 3:
          // 0 2 3 5 6
          segments[0]->on();
          segments[1]->off();
          segments[2]->on();
          segments[3]->on();
          segments[4]->off();
          segments[5]->on();
          segments[6]->on();
          break;
        case 4:
          // 1 2 3 5
          segments[0]->off();
          segments[1]->on();
          segments[2]->on();
          segments[3]->on();
          segments[4]->off();
          segments[5]->on();
          segments[6]->off();
          break;
        case 5:
          // 0 1 3 5 6
          segments[0]->on();
          segments[1]->on();
          segments[2]->off();
          segments[3]->on();
          segments[4]->off();
          segments[5]->on();
          segments[6]->on();
          break;
        case 6:
          // 0 1 3 4 5 6
          segments[0]->on();
          segments[1]->on();
          segments[2]->off();
          segments[3]->on();
          segments[4]->on();
          segments[5]->on();
          segments[6]->on();
          break;
        case 7:
          // 0 2 5
          segments[0]->on();
          segments[1]->off();
          segments[2]->on();
          segments[3]->off();
          segments[4]->off();
          segments[5]->on();
          segments[6]->off();
          break;
        case 8:
          // 0 1 2 3 4 5 6
          segments[0]->on();
          segments[1]->on();
          segments[2]->on();
          segments[3]->on();
          segments[4]->on();
          segments[5]->on();
          segments[6]->on();
          break;
        case 9:
          // 0 1 2 3 5 6
          segments[0]->on();
          segments[1]->on();
          segments[2]->on();
          segments[3]->on();
          segments[4]->off();
          segments[5]->on();
          segments[6]->on();
          break;
        default:
          // Show an E for error
          segments[0]->on();
          segments[1]->on();
          segments[2]->off();
          segments[3]->on();
          segments[4]->on();
          segments[5]->off();
          segments[6]->on();          
      }
    }
    ~Digit() {
      for (int i = 0; i < segment_count; i++) {
        delete segments[i];
      }      
    }
  private:
    Segment* segments[segment_count];
};

class Clock {
    Digit* digits[digit_count];
  public:
    Clock() {}
    init() {
      // Create the digits from the configuration
      for (int i = 0; i < digit_count; i++) {
        digits[i] = new Digit(
          digit_configuration[i][0], // pins
          digit_configuration[i][1], // on angles
          digit_configuration[i][2]  // off angles
        );
      }  
    }
    time(DateTime time) {
      int hour = time.hour();
      int minute = time.minute();
    
      // Find the individual digits to display for the hour
      if (hour > 9) {
        if (hour > 19) {
          digits[0]->number(2);
          hour -= 20;
        } else {
          digits[0]->number(1);
          hour -= 10;
        }
        digits[1]->number(hour);
      } else {
        digits[0]->number(0);
        digits[1]->number(hour);
      }
    
      // Find the individual digits to display for the minute
      if (minute > 9) {
        if (minute > 49) {
          digits[2]->number(5);
          minute -= 50;
        } else if (minute > 39) {
          digits[2]->number(4);
          minute -= 40;
        } else if (minute > 29) {
          digits[2]->number(3);
          minute -= 30;
        } else if (minute > 19) {
          digits[2]->number(2);
          minute -= 20;
        } else {
          digits[2]->number(1);
          minute -= 10;
        }
        digits[3]->number(minute);
      } else {
        digits[2]->number(0);
        digits[3]->number(minute);
      }
    }
    ~Clock() {
      for (int i = 0; i < digit_count; i++) {
        delete digits[i];
      }      
    }
};

// Our one and only clock
Clock clock;

// Get input from serial
int serial_get_number() {
  while (Serial.available() > 0) {
    int val = Serial.parseInt();
    if (Serial.read() == '\n') {
      return val;
    }
  }
}

// Only needed in calibration mode. For some reason if this object
// is constructed repeatedly inside the main loop the servos jitter
// so it is defined globally here.
#ifdef CALIBRATION_MODE
Servo servo;
#endif

void setup() {
  // Serial console for debugging
  Serial.begin(9600);

#ifdef CALIBRATION_MODE
  Serial.print("Servo Calibration Mode. Ensure newline is enabled in serial monitor!\n");
#else
  // Check if RTC module exists
  if (!rtc.begin()) {
    Serial.println("Error: RTC module not found");
    while (1);
  }

  // Set RTC time if not set already
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // Get our clock ready to use
  clock.init();
#endif
}

void loop() {
#ifdef CALIBRATION_MODE
  Serial.print("Enter servo pin number: ");
  while (!Serial.available()) delay(10); // wait
  int pin = serial_get_number();
  Serial.print(pin); Serial.print("\n");

  Serial.print("Enter servo angle: ");
  while (!Serial.available()) delay(10); // wait
  int angle = serial_get_number();
  Serial.print(angle); Serial.print("\n");

  Serial.print("Moving servo...\n");
  servo.attach(pin); servo.write(angle);
#else
  // Fetch the time
  DateTime now = rtc.now();

  // Display the time
  clock.time(now);

  // We don't need to loop too thightly
  delay(100);
#endif
}

