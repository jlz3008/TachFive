/*
 Copyright © 2014 José Luis Zabalza  License LGPLv3+: GNU
 LGPL version 3 or later <http://www.gnu.org/copyleft/lgpl.html>.
 This is free software: you are free to change and redistribute it.
 There is NO WARRANTY, to the extent permitted by law.
*/

#include <Arduino.h>
#include "TachFive.h"

//-------[Variables used on ISR]-------------------------------------------------

static volatile unsigned suLastCapture;
static volatile unsigned suLastCount=0xFFFF;
static volatile bool sbOverrun;
static volatile bool sbCalibrationError;
static volatile uint8_t su8Prescaler; // 0 = Stop, 1..5 = Valid prescaler values
static volatile bool sbAutocalibration;

typedef enum {StoppedMode = 0,RunningMode = 1, CalibrationMode = 2,SyncronizationMode = 3} TachFiveMode;
static TachFiveMode sT5Mode;

//--------------------------------------------------------------------------
static void setPrescaler(uint8_t prescaler)
{
    if(prescaler <= 5)
        TCCR5B = (TCCR5B & ~(_BV(CS52) | _BV(CS51) | _BV(CS50))) | prescaler;
}
//--------------------------------------------------------------------------
#define stopCounter()  setPrescaler(0)

//--------------------------------------------------------------------------
static void incPrescaler()
{
    su8Prescaler++;
    if(su8Prescaler > 5)
    {
        sbCalibrationError = true;
        if(sbAutocalibration)
        {
            su8Prescaler = 1; // autocalibration = on, turn again
        }
        else
        {
// On Error, Stop Counter
            sT5Mode = StoppedMode;
            stopCounter();
        }
    }
}
//--------------------------------------------------------------------------
static void activateCalibration()
{
    sT5Mode = CalibrationMode; // after prescaler initialitation
    TCNT5 = 0;                        // Timer counter = 0
    ICR5  = 0;                        // Input capture = 0
    setPrescaler(su8Prescaler);
}
//--------------------------------------------------------------------------
static void initCalibration()
{
    // ensure synchronization init point to calibration
    sT5Mode = SyncronizationMode;
    sbCalibrationError = sbOverrun = false;
    TCNT5 = 0;                        // Timer counter = 0
    ICR5  = 0;                        // Input capture = 0
    su8Prescaler = 1;
    setPrescaler(su8Prescaler);
}
//--------------------------------------------------------------------------
ISR(TIMER5_OVF_vect)
{
    switch(sT5Mode)
    {
        case RunningMode:
            sbOverrun = true;
        break;
        case CalibrationMode:
            stopCounter();
            sT5Mode = SyncronizationMode;
            incPrescaler();
        break;
        case StoppedMode: // Preventive Programming. counter should be stopped
            stopCounter();
        break;
        case SyncronizationMode:
        default:
            //do nothing
        break;
    }
}
//--------------------------------------------------------------------------
ISR(TIMER5_CAPT_vect)
{
   unsigned actualCount = ICR5;

    switch(sT5Mode)
    {
        case RunningMode:
        {
            if(sbOverrun)
            {
                if(suLastCount < actualCount)
                {
                    // The count is > 64K -> it Need calibration
                    if(sbAutocalibration)
                    {
                        sT5Mode = CalibrationMode;
                        // Don't init su8Prescaler, so the prescaler sequence begins since last value
                        //initNextPrescaler();
                    }
                    else
                    {
                        sT5Mode = StoppedMode;
                        stopCounter();
                    }
                }
                else
                {
/// @todo if noise canceller is on, the signal is delayed 4 CLKi/o clocks
                    // Good capture with overrun
                    suLastCapture = 0xFFFFU - suLastCount + actualCount;
                }
                sbOverrun = 0;
            }
            else
            {
// Good capture without overrun
/// @todo if noise canceller is on, the signal is delayed 4 CLKi/o clocks
                suLastCapture = actualCount - suLastCount;
            }
            suLastCount = actualCount;
        }
        break;
        case SyncronizationMode:           // synchronization init point to calibration found
            activateCalibration();
        break;
        case CalibrationMode:
        {
          // We are calibrating and there is a capture, so the prescaler is OK
            suLastCount = suLastCapture = actualCount;
            sbOverrun = sbCalibrationError = false;
            sT5Mode = RunningMode;
        }
        break;
        case StoppedMode:
            stopCounter();
        break; // counter should be stopped
        default:
            // do nothing
        break;
    }
}
//--------------------------------------------------------------------------
TachFive::TachFive()
{
    // Some library or code initialize Timer/Counter 5 so we can't do something here
}
//--------------------------------------------------------------------------
void TachFive::initialize()
{
    TCCR5A = 0;                        // OC5A,OC5B and OC5C -> GPIO function and Timer Mode = Normal (0)
    TCCR5B = 0;                        // Noise Canceller = off, Edge Input capture = falling
                                       // Timer Mode = Normal (0) Prescaler = Clock Stopped
    TCCR5C = 0;                        // Force Output compare (A,B,C)= none
     TCNT5 = 0;                        // Timer counter = 0
      ICR5 = 0;                        // Input capture = 0
    TIMSK5 = _BV(ICIE5) | _BV(TOIE5) ; // Activate Input Capture and Overrun interrupts
    sT5Mode = StoppedMode;
}
//--------------------------------------------------------------------------
void TachFive::setTriggerEdge(int newEdge)
{
    if(newEdge == RISING)
        TCCR5B |= _BV(ICES5);
    else
        TCCR5B &= ~(_BV(ICES5));
}
//--------------------------------------------------------------------------
void TachFive::setNoiseCanceller(bool newNC)
{
    if(newNC)
        TCCR5B |= _BV(ICNC5);
    else
        TCCR5B &= ~(_BV(ICNC5));
}
//--------------------------------------------------------------------------
void TachFive::setAutocalibration(bool on)
{
    sbAutocalibration = on;
}
//--------------------------------------------------------------------------
void TachFive::start()
{
    uint8_t sreg;
// disconnect interrupt to atomic operation to no disturb
    sreg = SREG;
    cli();
// initialize prescaler sequence
    initCalibration();
// this set interrupt in the last status. I'm supposed to be activated at some point
    SREG = sreg;
}
//--------------------------------------------------------------------------
void TachFive::stop()
{
    uint8_t sreg;
// disconnect interrupt to atomic operation to no disturb
    sreg = SREG;
    cli();

    su8Prescaler = 0;
    sT5Mode = StoppedMode;
    stopCounter();
    // this set interrupt in the last status. I'm supposed to be activated at some point
    SREG = sreg;
}
//--------------------------------------------------------------------------
bool  TachFive::isStopped()
{
    return(sT5Mode == StoppedMode);
}
//--------------------------------------------------------------------------
bool  TachFive::isOnSyncronization()
{
    return(sT5Mode == SyncronizationMode);
}
//--------------------------------------------------------------------------
bool TachFive::isOnCalibration()
{
    return(sT5Mode == CalibrationMode);
}
//--------------------------------------------------------------------------
bool TachFive::isRunning()
{
    return(sT5Mode == RunningMode);
}
//--------------------------------------------------------------------------
bool TachFive::isCalibrationError()
{
    return(sbCalibrationError);
}
//--------------------------------------------------------------------------
unsigned TachFive::getRPM()
{
    switch(su8Prescaler)
    {
// CLKi/o = CPU clock. On Mega2560 = 16MHz
// Prescaler divisor values from Page 161 of 2549P–AVR–10/2012 datasheet
        case 1:return(16000000UL*60UL/(   1UL * (unsigned long)suLastCapture));break;
        case 2:return(16000000UL*60UL/(   8UL * (unsigned long)suLastCapture));break;
        case 3:return(16000000UL*60UL/(  64UL * (unsigned long)suLastCapture));break;
        case 4:return(16000000UL*60UL/( 256UL * (unsigned long)suLastCapture));break;
        case 5:return(16000000UL*60UL/(1024UL * (unsigned long)suLastCapture));break;
    }

    return(0);
}
