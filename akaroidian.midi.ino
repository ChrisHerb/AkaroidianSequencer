/**
 * Akaroidian Sequencer (Midi Implementation)
 * Copyright (C) 2020  Christopher Herb
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MIDIUSB.h"

byte shape1Length = 5;
byte shape2Length = 7;
byte shape3Length = 13;

byte shape1Rotation = 0;
byte shape2Rotation = 0;
byte shape3Rotation = 0;

byte measureLength = 16;

byte shape1Index = shape1Rotation;
byte shape2Index = shape2Rotation;
byte shape3Index = shape3Rotation;

byte channel1MeasureIndex = 0;
byte channel2MeasureIndex = 0;
byte channel3MeasureIndex = 0;

// Pulse per quarter note. Each beat has 24 pulses.
// Tempo is based on software inner BPM.
int ppqn = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  
  Serial.print("shape 1: ");
  Serial.print(shape1Length);
  Serial.print(", shape 2: ");
  Serial.print(shape2Length);
  Serial.print(", shape 3: ");
  Serial.print(shape3Length);
  Serial.println();

  // do midi stuff
  midiEventPacket_t rx;
  
  do {
    rx = MidiUSB.read();

    // Count pulses and send note 
    if(rx.byte1 == 0xF8) {
       ++ppqn;
       // play note on every 16th
       if(ppqn == 6){
          playNotes();
          ppqn = 0;
       };
    }
    // Clock start byte
    else if(rx.byte1 == 0xFA){
      playNotes();
      ppqn = 0;
    }
    // Clock stop byte
    else if(rx.byte1 == 0xFC){
      // reset
      ppqn = 0;
      shape1Index = shape1Rotation;
      shape2Index = shape2Rotation;
      shape3Index = shape3Rotation;
      channel1MeasureIndex = 0;
      channel2MeasureIndex = 0;
      channel3MeasureIndex = 0;
    }
    // Midi CC
    else if(rx.header == 0xB) {

      controlChange(
        rx.byte1 & 0xF,  // channel
        rx.byte2,        // control
        rx.byte3         // value
      );
    }
  } while (rx.header != 0);
}

/**
 * Controls the length and rotation of all 3 shapes.
 * CC1 = shape 1 length
 * CC2 = shape 2 length
 * CC3 = shape 3 length
 * CC4 = shape 1 rotation
 * CC5 = shape 2 rotation
 * CC6 = shape 3 rotation
 */
void controlChange(byte channel, byte control, byte value) {  
  // shape 1 length
  if(control == 1) {
    shape1Length = validateLength(value);
  }
  // shape 2 length
  else if(control == 2) {
    shape2Length = validateLength(value);
  }
  // shape 3 length
  else if(control == 3) {
    shape3Length = validateLength(value);
  }
  // shape 1 rotation
  else if(control == 4) {
    shape1Rotation = value;
    shape1Index = validateIndex(shape1Length, shape1Rotation);
  }
  // shape 2 rotation
  else if(control == 5) {
    shape2Rotation = value;
    shape2Index = validateIndex(shape2Length, shape2Rotation);
  }
  // shape 3 rotation
  else if(control == 6) {
    shape3Rotation = value;
    shape3Index = validateIndex(shape3Length, shape3Rotation);
  }
}

/**
 * Validates if the given index 'fits' in the new length and returns a new index if it didn't.
 */
byte validateIndex(byte &newLength, byte &shapeIndex) {
  if(newLength<shapeIndex-1) {
    shapeIndex = 0;
  }
  return shapeIndex;
}

/**
 * Defaults the length to 1 if the given length is smaller than 1;
 */
byte validateLength(int newLength) {
  if(newLength<1) {
    return 1;
  }
  return newLength;
}

/**
 * Initialzes the shapes and plays the notes of all shapes at the current index. 
 * Advances the indices at the end.
 */
void playNotes() {
  
  // initialize shapes
  byte shape1[shape1Length];
  byte shape2[shape2Length];
  byte shape3[shape3Length];
  createShape(shape1, shape1Length, shape2Length, shape3Length);
  createShape(shape2, shape2Length, shape1Length, shape3Length);
  createShape(shape3, shape3Length, shape2Length, shape1Length);

  // get shape values
  byte shape1Value = shape1[shape1Index];
  byte shape2Value = shape2[shape2Index];
  byte shape3Value = shape3[shape3Index];

  // play values
  playNote(shape1Value, 0, false);
  playNote(shape2Value, 1, false);
  playNote(shape3Value, 2, false);

  // additional funky logic outputs
  byte andVal = (shape1Value == 1) && (shape2Value == 1) && (shape3Value == 1) ? 1 : 0;
  playNote(andVal, 3, false);
  byte xOrVal = (shape1Value == 0) ^ (shape2Value == 0) ^ (shape3Value == 0) ? 1 : 0;
  playNote(xOrVal, 4, false);
  byte orVal = (shape1Value == 1) || (shape2Value == 1) || (shape3Value == 0) ? 1 : 0;
  byte combVal = andVal ^ xOrVal ^ orVal ? 1 : 0;
  playNote(combVal, 5, false);

  // advance indices
  advanceIndex(shape1Index, shape1Length, shape1Rotation, measureLength, 1, channel1MeasureIndex);
  advanceIndex(shape2Index, shape2Length, shape2Rotation, measureLength, 2, channel2MeasureIndex);
  advanceIndex(shape3Index, shape3Length, shape3Rotation, measureLength, 4, channel3MeasureIndex);
}

/**
 * Advances the index of the given shape under consideration of the index position, the shape length and position in measure.
 */
void advanceIndex(byte &shapeIndex, byte &shapeLength, byte &shapeRotation, byte &measureLength, byte measures, byte &measureIndex) {
  // advance shape index as long as it's shorter than the shape's length
  if(shapeIndex<shapeLength-1) {
    shapeIndex++;
  }
  else {
    shapeIndex = 0;
  }
  // advance measure index as long as it's shorter than the measure's length
  if(measureIndex<measures*measureLength-1) {
    measureIndex++;
  }
  else { // otherwise reset all indices
    shapeIndex = shapeRotation;
    measureIndex = 0;
  }
}

/**
 * Fills the shape with 1's and 0's according to the shapes length
 */
void createShape(byte shape1[], byte &shape1Length, byte &shape2Length, byte &shape3Length) {
  for(byte i=0; i<shape1Length; i++) {
    if(i == 0 || i == shape2Length || i == shape3Length) {
      shape1[i] = 1;
    }
    else if(i % shape2Length == 0) {
      shape1[i] = 1;
    }
    else if(i % shape3Length == 0) {
      shape1[i] = 1;
    }
    else {
      shape1[i] = 0;
    }
  }
}

/**
 * Plays a midi note if the value = 1 at the given channel. Logic can be inverted.
 */
void playNote(byte &value, byte channel, boolean invert) {
  if(!invert && value == 1 || invert && value == 0) {
    noteOn(channel, 48, 100);
    MidiUSB.flush();
  }
  else {
    noteOff(channel, 48, 100);
    MidiUSB.flush();
  }
}

/**
 * Sends a midi note on message.
 */
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

/**
 * Sends a midi note off message.
 */
void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}
