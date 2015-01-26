# TachFive

Arduino library. Precision tachometer using timer/counter 5
============================================================

Preliminary
-----------
 1.- The Timer/Counter 5 will be used. (The Timer onwards)
 2.- The example was tested on Mega2560 board but it should work on other boards.

Strategies
---------------
 1.- The tach signal will be connected to ICP5 pin (PL1 -> GPIO48 -> pin 29 on XIO connector on MEGA2560 board)
 2.- The Timer will be programned on counter mode and free run.
 3.- The counter input is internal CLKi/o pulses from prescaler.
 4.- The class is a SINGLETON because it's no desirable to have more instances of this class.

Operations
----------

 The application must get the instance of the library object with instance() function
 ```
		  static TachFive & Tach = TachFive::instance();
 ```
 The application must execute `initialize()` function to configure the Timer. The sketch setup() function
 is the perfect place to do this.

 This library (the auto instantiated object or the class more exactly) have four running modes.
  - **Stopped**        : The clock is turn off and no operation is possible.
  - **Syncronization** : The library is waiting for a tachometer pulse to start the prescaler calibration.
  - **Calibration**    : The library is calibrating the prescaler.
  - **Running**        : The library is measuring the tachometer.



 On start, the class begins on Stopped Mode so, the application can setup tachometer pulse characteristics before
 execute `start()` function to start operations

 The library will walks through the those states automatically. The application can stop the
 operations with stop()function and restart with start() function.

 The application can set tachometer signal characteristics with the functions:

   - `void setTriggerEdge(int newEdge);`
   - `void setNoiseCanceller(bool newNC);`

 The application can set autocalibration mode with setAutocalibration(bool)

 If autocalibration is OFF (default) the library turn off the Timer if is not posible find a correct
 prescaler setup to tachometer frequency signal (theorialy 960,000,000 RPM to 0.004 RPM). In this case
 the application should start the Timer again with start() function.

 If autocalibration is On, the library tries again and again find the correct prescaler setup,
 bringing the library from syncronization to calibration mode until find prescaler value.

 The application get the library status with functions :

   - `bool isStopped();`
   - `bool isOnSyncronization();`
   - `bool isOnCalibration();`
   - `bool isCalibrationError();`

 If the status is running, the application get the tachometer measurement with getRPM() function:

Example
=======

This library was tested with a 4 wires computer fan. The connections are documented on doc directory.


