//#include <SoftwareSerial.h>
#include "LED.h"
#define IND 13

//uint8_t red = 0, green = 0, blue = 0;
//SoftwareSerial blt(8,7);
/*

1 byte sent for each colour

pos       7       6       5       4       3       2       1       0
route     0       0       0       0       0       0       0       0
                  F       P       L       U       B       M       T

F : Flash
L : Watch External Light Levels
P : Pulse (Breather Mode)
U : User
B : Bass
M : Mid
T : Treble

*/


LED Red(5);
LED Green(3);
LED Blue(6);

void setup()
{ 
    //blt.begin(9600);
    Serial.begin(115200);
    pinMode(IND, OUTPUT);
    digitalWrite(IND, HIGH);
    red.setTime(3,5);
    green.setTime(3,4);
    blue.setTime(3,3);
    red.pulse(true);
    green.pulse(true);
    blue.pulse(true);
}

String rxBuff;

void loop()
{
    /*
    if((1<<3) & redControl)
    red.setBrightness(X)
    if((1<<6) & redControl || (1<<5) & redControl)
    red.update();
    */
    red.update();
    green.update();
    blue.update();
    /*int i;
    String rx = "";
    while(blt.available())
    {
        char c = blt.read();
        rx += c;
    }
    if(rx.length() == 1)
    rxBuff = rx;
    if(rx != "")
    {
        rx = rxBuff + rx;
        //Serial.print("Raw:");
        //Serial.println(rx);
        for(i = rx.length() - 2; i >= 0; i--)
        {
            char c = rx.charAt(i);
            if(c == ')')
            {
                rx = rx.substring(i+1,(rx.length() - 1));
                break;
                //Serial.println(rx);
            }
        }
        Serial.println(rx);
        getVars(rx);
        redControl = rx[0];
        greenControl = rx[1];
        blueControl = rx[2];
        redControl = rx[3];
        greenControl = rx[4];
        blueControl = rx[5];
    }*/
}

void getVars(String rx)
{
  // Assume we got 3 bytes here, each for red, green and blue
  // B M T uint8_t
  /*
  Incoming Message: 6 bytes
  
      #X.X.X.R.G.B~
  
  X = Status Byte of respective colour
  */
  Serial.println("Updating...");
  String findRed = "" , findGreen = "" , findBlue = "";
  boolean fred = false, fgreen = false, fblue = false;
  char c;
  uint8_t length = rx.length();
  for(int i = 0; i < length; i++)
  {
    c = rx.charAt(i);
    if(isDigit(c) && !fred)
    {
        findRed += c;
        continue;
    }
    else if(!fred)
    {
        fred = true;
        continue;
    }
    if(isDigit(c) && !fgreen)
    {
        findGreen += c;
        continue;
    }
    else if(!fgreen)
    {
        fgreen = true;
        continue;
    }
    if(isDigit(c) && !fblue)
    {
        findBlue += c;
        continue;
    }
    else if (!fblue)
    {
        fblue = true;
        continue;
    }
    }
    //Serial.println(findRed);
    //Serial.println(findGreen);
    //Serial.println(findBlue);
    red = findRed.toInt();
    green = findGreen.toInt();
    blue = findBlue.toInt();
}