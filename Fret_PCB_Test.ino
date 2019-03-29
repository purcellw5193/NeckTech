/*
 * Fret_PCB_Test.ino
 * 
 * Used to test the NeckTech fret detector PCB.
 * This code is designed to be used with the Teensy 3.2 microcontroller
 * Each possible address is cyclically inputted to the circuit,
 * with data being read from the output pins for each address.
 * 
 * J9 pin 1 (rightmost): PE Enable in - active low
 * J9 pin 2 : PE Enable out - active low
 * J9 pin 3 : 5V VCC
 * J9 Pin 4 (leftmost): GND
 * 
 * J8 pin 1 (top): MUX address C
 * J8 pin 2 : MUX address B
 * J8 pin 3 : MUX address A
 * J8 pin 4 : PE output A0
 * J8 pin 5 : PE output A1
 * J8 pin 6 (bottom): PE output A2
 * 
 * v1.0 Author: Bill Purcell
 * March 22, 2019
 * 
 */

#include <MIDI.h>
#include <string.h>
#include <stdlib.h>
#include "noteList.h"
#include "pitches.h"

// MUX Address pins [7, 8, 9]
// SN74HC151N
static int MUX0 = 7;
static int MUX1 = 8;
static int MUX2 = 9;

// Priority Encoder Address pins [10, 11, 12]
// SN74HC148N
static int PE0 = 10;
static int PE1 = 11;
static int PE2 = 12;

void setup()
{
    // MUX Address is output
    pinMode(MUX0, OUTPUT);   
    pinMode(MUX1, OUTPUT); 
    pinMode(MUX2, OUTPUT); 
    // PE data is input
    pinMode(PE0, INPUT);  
    pinMode(PE1, INPUT); 
    pinMode(PE2, INPUT); 
    //  Monitor the data - baud rate doesn't matter
    Serial.begin(9600);

}

void loop()
{
  // Initialize data
  int MUX_address = 0;
  int PE_Out = 0;
  // Monitoring messages stored here
  char msg[100];

  while(1)
  {
    // Loop through MUX addresses [1 - 6]
   if(MUX_address >= 6)
   {
    MUX_address = 1;
   }
   else
   {
    MUX_address++;
   }
   // Address the MUX
   writeToMUX(MUX_address);
   // Read the output data
   PE_Out = readFromEncoder();

   // Monitor the data
   // sprintf is needed to use printf style printing

   // sprintf stores the message into msg
   sprintf(msg, "MUX address: %d\n", MUX_address);
   // Serial.print prints msg
   Serial.print(msg);
   
   sprintf(msg, "Output: %d\n", PE_Out);
   Serial.print(msg);

    // Wait 1 second before reading new data
    delay(1000);
  }
}

/*
 * writeToMUX takes a single integer address and writes it to 
 * the three MUX address pins. MUX2, MUX1 and MUX0 should be
 * defined as the pin numbers connected to the MUX address pins.
 * 
 * @param int $addr
 *  the MUX address
 */
void writeToMUX(int addr)
{
  digitalWrite(MUX2, (addr & B00000100 ));
  digitalWrite(MUX1, (addr & B00000010 ));
  digitalWrite(MUX0, (addr & B00000001 ));
}

/*
 * readFromEncoder reads the data on the priority encoder.
 * PE0, PE1, and PE2 should be defined as the pin numbers
 * connected to the PE output pins
 * 
 * @return int
 *  data on the PE pins
 * 
 */
int readFromEncoder(void)
{
  int data = 0;

  data = data + digitalRead(PE0);
  data = data + digitalRead(PE1)*2;
  data = data + digitalRead(PE2)*4;

  // Data returned is active low:
  // subtract it from 7 to get the number
  return (7-data);
}



