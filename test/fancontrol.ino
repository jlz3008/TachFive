/*
 Copyright © 2014 José Luis Zabalza  License LGPLv3+: GNU
 LGPL version 3 or later <http://www.gnu.org/copyleft/lgpl.html>.
 This is free software: you are free to change and redistribute it.
 There is NO WARRANTY, to the extent permitted by law.
*/


#define VERSION "1.1"

#include <inttypes.h>
#include <assert.h>
#include "TachFive.h"
#include "ansi.h"

//---------Volatile variables for interrupt routine -----------------------------

static uint8_t su8PWM;
static int siEdge;
static bool sbNoiseCanceller;
static bool sbAutocalibration;
static bool sNeedRefreshData;

//------Local prototipes---------------------------------------------------------

static TachFive & Tach = TachFive::instance();

//------Local prototipes---------------------------------------------------------

void Menu();
void ClearSubmenu();
void IdleProc();

//--------------------------------------------------------------------------------
void setup()
{
    Serial.begin(115200);
    Serial.print("Start");

    analogWrite(8,su8PWM);
    Tach.initialize();

    sNeedRefreshData = true;
    Menu();
}
//--------------------------------------------------------------------------------
void loop()
{
    char c;

    if(Serial.available())
    {
        c = Serial.read();
        switch(c)
        {
            case '+':
                if(su8PWM < 255)
                {
                    su8PWM++;
                    analogWrite(8,su8PWM);
                }
                sNeedRefreshData=true;
            break;
            case '-':
                if(su8PWM > 0)
                {
                    su8PWM--;
                    analogWrite(8,su8PWM);
                }
                sNeedRefreshData=true;
            break;
            case '0':
                 Tach.stop();
                sNeedRefreshData=true;
            break;
            case '1':
                 Tach.start();
                sNeedRefreshData=true;
            break;
            case 'N':
            case 'n':
                sbNoiseCanceller = (c == 'N');
                 Tach.setNoiseCanceller(sbNoiseCanceller);
                sNeedRefreshData=true;
            break;
            case 'A':
            case 'a':
                sbAutocalibration = (c == 'A');
                Tach.setAutocalibration(sbAutocalibration);
            break;
            case 'E':
            case 'e':
                siEdge = (c == 'E') ? RISING : FALLING;
                 Tach.setTriggerEdge(siEdge);
                sNeedRefreshData=true;
            break;
        }
    }// if serial available
    else
        IdleProc();
}
//-----------------------------------------------------------
void IdleProc()
{
    static bool stopped;
    static bool syncro;
    static bool calibration;
    static bool calerror;


    if(Tach.isStopped() != stopped)
    {
        stopped = !stopped;
        sNeedRefreshData = true;
    }

    if(Tach.isOnSyncronization() != syncro)
    {
        syncro = !syncro;
        sNeedRefreshData = true;
    }

    if(Tach.isOnCalibration() != calibration)
    {
        calibration = !calibration;
        sNeedRefreshData = true;
    }

    if(Tach.isCalibrationError() != calerror)
    {
        calerror = !calerror;
        sNeedRefreshData = true;
    }


    if(sNeedRefreshData)
    {
        sNeedRefreshData = false;
        GotoXY(1,3);
        ClearLine();Serial.print(" Tach power             = ");
            SetColor(Yellow,true,true);  Serial.print((stopped) ? "Off" : "On");SetColor(Reset);Serial.println("");
        ClearLine();Serial.print(" Tach Syncronization    = ");
            SetColor(Yellow,true,true);  Serial.print((syncro) ? "ON" : "Off");SetColor(Reset);Serial.println("");
        ClearLine();Serial.print(" Tach Autocalibration   = ");
            SetColor(Yellow,true,true);  Serial.print((sbAutocalibration) ? "ON" : "Off");SetColor(Reset);Serial.println("");
        ClearLine();Serial.print(" Tach Calibration       = ");
            SetColor(Yellow,true,true);  Serial.print((calibration) ? "ON" : "Off");SetColor(Reset);Serial.println("");
        ClearLine();Serial.print(" Tach Calibration Error = ");
            SetColor(Yellow,true,true);  Serial.print((calerror) ? "ON" : "Off");SetColor(Reset);Serial.println("");
        GotoXY(40,3); Serial.print("PWM Speed        = ");
            SetColor(Yellow,true,true); Serial.print(su8PWM);SetColor(Reset);Serial.println("");
        GotoXY(40,4); Serial.print("Tach speed       = ");
            SetColor(Yellow,true,true); Serial.print(Tach.getRPM());SetColor(Reset);Serial.println(" RPM");
        GotoXY(40,5); Serial.print("Edge tach signal = ");
            SetColor(Yellow,true,true); Serial.print((siEdge) ? "RISING" : "FALLING");SetColor(Reset);Serial.println("");
        GotoXY(40,6); Serial.print("Noise Canceller  = ");
            SetColor(Yellow,true,true); Serial.print((sbNoiseCanceller) ? "ON" : "OFF");SetColor(Reset);Serial.println("");
    }
    else if(Tach.isRunning())
    {
        static unsigned refreshCounter;
        if(++refreshCounter > 1000)
        {
            GotoXY(59,4);ClearLine(from_cursor);
            SetColor(Yellow,true,true); Serial.print(Tach.getRPM());SetColor(Reset);Serial.println(" RPM");
            refreshCounter = 0;
        }
    }
}

//-----------------------------------------------------------
void Menu()
{
    ClearScreen();
    Serial.println("Fan control test ");
    Serial.println("=================");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");

    Serial.println("----------------------------------------------------------------------------");
    Serial.println(" +.- Increase speed                       +.- Decrease speed");
    Serial.println(" 1.- Start Tach                           0.- Stop Tach");
    Serial.println(" N/n Noise Canceller (On/off)             E/e Edge tach signal (Rising/falling)");
    Serial.println(" A/a Autocalibration (On/off)");
    Serial.println("----------------------------------------------------------------------------");

}
