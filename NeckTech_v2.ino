/*
 * NeckTech.ino
 * 
 * This is the top-level software used to operate the NeckTech MIDI Guitar System
 * Note and pluck data is cyclically and continuously read from the hardware
 * and MIDI messages are communicated via USB.
 * 
 * J9 pin 1 (rightmost): PE Enable in - active low
 * J9 pin 2 : PE Enable out - active high
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
 *  March 22, 2019
 *  
 *  v2 Author: Bill Purcell
 *   April 10, 2019
 * 
 */

#include <MIDI.h>
#include <string.h>
#include <stdlib.h>
#include "noteList.h"
#include "pitches.h"

// Pins connected to the pluck detection circuits [1, 2, 3, 4, 5, 6]
const static int PLUCK1 = 1;
const static int PLUCK2 = 2;
const static int PLUCK3 = 3;
const static int PLUCK4 = 4;
const static int PLUCK5 = 5;
const static int PLUCK6 = 6;

// MUX Address pins [7, 8, 9]
// SN74HC151N
const static int MUX0 = 7;
const static int MUX1 = 8;
const static int MUX2 = 9;

// Priority Encoder Address pins [10, 11, 12]
// SN74HC148N
const static int PE0 = 10;
const static int PE1 = 11;
const static int PE2 = 12;

// Default velocity and channel
static byte velocity = 0x40;
static byte channel = 1;

// Holds current and past fret detection readings
static int fretData[2][6] = { {0,0,0,0,0,0},
                               {0,0,0,0,0,0} };

// Holds current and past pluck detection data
static int pluckData[2][6] = { {0,0,0,0,0,0},
                               {0,0,0,0,0,0} };

// Open string notes
const static int standardTuning[6] = {64, 59, 55, 50, 45, 40};
                               
// The pins the pluck detectors are connected to
const static int pluckPins[6] = {PLUCK1, PLUCK2, PLUCK3, PLUCK4, PLUCK5, PLUCK6};

const static int GND1 = 13;
const static int GND2 = 14;
const static int GND3 = 15;
const static int GND4 = 16;
const static int GND5 = 17;

MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  
    // MUX Address is output
    pinMode(MUX0, OUTPUT);   
    pinMode(MUX1, OUTPUT); 
    pinMode(MUX2, OUTPUT); 
    // PE data is input
    pinMode(PE0, INPUT);  
    pinMode(PE1, INPUT); 
    pinMode(PE2, INPUT); 
    // Set the pluck pins as inputs
    pinMode(PLUCK1, INPUT);
    pinMode(PLUCK2, INPUT);
    pinMode(PLUCK3, INPUT);
    pinMode(PLUCK4, INPUT);
    pinMode(PLUCK5, INPUT);
    pinMode(PLUCK6, INPUT);
    // Create extra ground pins on the board for power options
    pinMode(GND1, OUTPUT); digitalWrite(GND1, LOW);
    pinMode(GND2, OUTPUT); digitalWrite(GND2, LOW);
    pinMode(GND3, OUTPUT); digitalWrite(GND3, LOW);
    pinMode(GND4, OUTPUT); digitalWrite(GND4, LOW);
    pinMode(GND5, OUTPUT); digitalWrite(GND5, LOW);
    
    // Start the MIDI object
    MIDI.begin();
}


void loop() {

  // Variable number of strings supported
  int numStrings = 5;
  // The lowest number string used
  int lowString = 1;
  //byte n = lowString;

  int n = 0;
  
  while(1)
  {
    if(n+1 < numStrings)
    {
      n++;
    }
    else
    {
      n = 0;
    }
    // Address the PCB
    writeToMUX(n+1);
    
    // Store the old data
    fretData[1][n] = fretData[0][n];
    pluckData[1][n] = pluckData[0][n];

    // Read the new data
    do{
    fretData[0][n] = readFromEncoder() + standardTuning[n];
    delay(5);
    }while(readFromEncoder()+standardTuning[n] != fretData[0][n]);
    pluckData[0][n] = !(readPluck(pluckPins[n]));
    
    // Decide what MIDI messages need to be sent and send them
    sendMIDIMessage(fretData[0][n], fretData[1][n],
                    pluckData[0][n], pluckData[1][n]);

  
    //delay();    
  }

}

/*
 * sendMIDIMessage determines what MIDI messages need to be sent and sends
 * them by using the fret detection data, pluck detection data, and the
 * fret and pluck detection data read in the previous cycle.
 * NoteOn and NoteOff messages are currently supported.
 * 
 * @param int $note
 *  The data read from the note detector
 * 
 * @param int $lastNote
 *  The data read from the note detector on the last cycle
 * 
 * @param int $isOn
 *  The data read from the pluck detector
 *  
 * @param int $lastIsOn
 *  The data read from the pluck detectorlast cycle
 */
void sendMIDIMessage(int note, int lastNote, int isOn, int lastIsOn)
{
  // Concatenates the identified change in current note and last note:
  // bit 0 defines whether the note reading has changed (active high)
  // bit 1 defines whether the pluck reading has changed (active high)
  int state = 0b00000011 & (((isOn == lastIsOn) << 1) | (note == lastNote));
  
      if (isOn) 
      {
        switch (state) 
        {
          case 3: //same active status, same note, do nothing
            break;
          case 2: //same active status, new note - still active, turn off the last note, turn on the new one
            MIDI.sendNoteOff(lastNote, velocity, channel);
            MIDI.sendNoteOn(note, velocity, channel);
            break;
          case 1: //new active status, same note - in this portion of the code active must have changed from 0 to 1; play the note
            MIDI.sendNoteOn(note, velocity, channel);
            break;
          case 0: //new active status, new note - same scenario as above; the new note status is irrelevant because it was from an inactive status              
            MIDI.sendNoteOn(note, velocity, channel);
            break;
          default:
            break;
      }
    }

    else {
      if (isOn != lastIsOn) {
         MIDI.sendNoteOff(lastNote, velocity, channel);
      }
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

/*
 * readPluck reads data from the pluck detector pins.
 * The reading is hard-coded in this function because
 * sometimes digitalRead doesn't seem to want to accept
 * variable inputs from an array.
 * 
 * @param $n the pin to be read from
 * 
 * @return int
 *  data on the pluck pin
 * 
 */
int readPluck(int n)
{
  switch(n)
  {
    case 1:
      return digitalRead(1);
      break;
    case 2:
      return digitalRead(2);
      break;
    case 3:
      return digitalRead(3);
      break;
    case 4:
      return digitalRead(4);
      break;
    case 5:
      return digitalRead(5);
      break;
    case 6:
      return digitalRead(6);
      break;
    default:
      return n;
      break;
  }
}

