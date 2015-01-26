/*
 Copyright © 2014 José Luis Zabalza  License LGPLv3+: GNU
 LGPL version 3 or later <http://www.gnu.org/copyleft/lgpl.html>.
 This is free software: you are free to change and redistribute it.
 There is NO WARRANTY, to the extent permitted by law.
*/

#include <avr/io.h>
#include <avr/interrupt.h>


class TachFive
{
  public:
  
// We can not allow another object of this class, so this class
// is a singleton.
    static TachFive & instance()
    {
        static TachFive o;
        return o;
    }

    void initialize();

    void start();
    void stop();

    bool isStopped();
    bool isOnSyncronization();
    bool isOnCalibration();
    bool isRunning();
    bool isCalibrationError();
    void setTriggerEdge(int  newEdge);
    void setNoiseCanceller(bool newNC);

    void setAutocalibration(bool on);

    unsigned getRPM();

protected:
    TachFive();

};

