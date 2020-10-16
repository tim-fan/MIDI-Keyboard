#include <Bounce.h>

// Notes
// Written for Teensy 3.2 board.
// Make sure to select USB Type - MIDI in tools menu
//
// Pin configuration:
// Outputs:
//  pins 2 -> 8 (inclusive)
// Inputs:
//  pins 14 -> 21 (inclusive)
//
// Main algorithm:
// for each output, set high, sample inputs
// on detected rising edge, send appropriate NOTE_ON event
// on detected falling edge, send appropriate NOTE_OFF event

#define NUM_OUTPUTS 7
#define NUM_INPUTS 8

unsigned int OUTPUT_PINS[NUM_OUTPUTS] = {2,3,4,5,6,7,8};
unsigned int INPUT_PINS[NUM_INPUTS] = {14,15,16,17,18,19,20,21};

unsigned int notes[NUM_OUTPUTS][NUM_INPUTS];
Bounce* buttons[NUM_OUTPUTS][NUM_INPUTS];

// the MIDI channel number to send messages
const int CHANNEL = 1;
const int NOTE_VELOCITY = 99;
const unsigned int WAIT_DELAY = 100; //us, between setting an output high and reading inputs
const unsigned int BOUNCE_PERIOD = 1; //ms

bool led_state = false;

void setup() {
  //create the button objects
  for (int o=0; o<NUM_OUTPUTS; o++){
    for (int i=0; i<NUM_INPUTS; i++){
      int input_pin = INPUT_PINS[i];
      buttons[o][i] = new Bounce(input_pin, BOUNCE_PERIOD);
    }
  }

  //define notes:
  //refer https://en.scratch-wiki.info/wiki/MIDI_Notes 
  //took a bit of playing around to figure out mapping of keys
  //to notes
  int MIDDLE_C = 72;
  int MEASURED_MIDDLE_C = -31;
  // measured + offset = correct
  // offset = correct - measured
  int MIDI_OFFSET = MIDDLE_C - MEASURED_MIDDLE_C;
  for (int o=0; o<NUM_OUTPUTS; o++){
    for (int i=0; i<NUM_INPUTS; i++){
      notes[o][i] = MIDI_OFFSET -(o*NUM_INPUTS + i);
    }
  }

  //set pinmodes:
  for (int o=0; o<NUM_OUTPUTS; o++){
    int out_pin = OUTPUT_PINS[o];
    pinMode(out_pin, OUTPUT);
    digitalWrite(out_pin, LOW);
  }
  for (int i=0; i<NUM_INPUTS; i++){
    int input_pin = INPUT_PINS[i];
    pinMode(input_pin, INPUT_PULLDOWN);
  }

  pinMode(LED_BUILTIN, OUTPUT);  

  // for debugging
//  Serial.begin(960/0);

  delayMicroseconds(WAIT_DELAY);

}

void loop() {

  //scan 
  for (int o=0; o<NUM_OUTPUTS; o++){
    //set current output high and sample all inputs
    int out_pin = OUTPUT_PINS[o];
    digitalWrite(out_pin, HIGH);
    delayMicroseconds(WAIT_DELAY);
    for (int i=0; i<NUM_INPUTS; i++){
      buttons[o][i]->update();
    }
    digitalWrite(out_pin, LOW);
  }

  //make beautiful music
  for (int o=0; o<NUM_OUTPUTS; o++){
    for (int i=0; i<NUM_INPUTS; i++){
      int note = notes[o][i];
      if (buttons[o][i]->risingEdge()) {
        //blink
        digitalWrite(LED_BUILTIN, led_state);
        led_state = !led_state;

//        /Serial.println(note); 
               
        usbMIDI.sendNoteOn(note, NOTE_VELOCITY, CHANNEL);
      }
      else if (buttons[o][i]->fallingEdge()) {
        usbMIDI.sendNoteOff(note, 0, CHANNEL);
      }
    }
  }
  
  // MIDI Controllers should discard incoming MIDI messages.
  // http://forum.pjrc.com/threads/24179-Teensy-3-Ableton-Analog-CC-causes-midi-crash
  while (usbMIDI.read()) {
//     ignore incoming messages
  }
}
