#include <Wire.h> //for i2c
#include <MeetAndroid.h> //for android 
#include <OneWire.h> //for temperature sensor communication
#include <DallasTemperature.h> //for temperature sensor data
                                                                                                                                                                         

int trig; //power trigger current state
int prev_trig = 0; //power trigger previous state

#define ONE_WIRE_BUS 5
#define accPin 2 //D2
#define ledPin 13 
#define relayPin 3 //D3 output to power button
#define AUDIO_MUTE 4

#define TDA7318_I2C_ADDRESS 0x44
#define TDA_SW1 0x58
#define TDA_SW2 0x59
#define TDA_SW3 0x5A
#define TDA_SW4 0x5B


#define RADIO_I2C_ADDRESS 0x60
#define RADIO_I2C_SUB_ADDRESS 0x20

#define RADIO_MIN_FREQUENCY 880 
#define RADIO_MAX_FREQUENCY 1080

#define RADIO_SOURCE 2 //tda's input channel 2
#define MUSIC_SOURCE 3 //tda's input channel 3

DeviceAddress intTempSensor = { 0x28, 0x44, 0x0A, 0xD8, 0x02, 0x00, 0x00, 0x58 }; 
DeviceAddress extTempSensor = { 0x28, 0xA8, 0xE4, 0x7D, 0x02, 0x00, 0x00, 0x5C };

unsigned char frequencyH = 0;
unsigned char frequencyL = 0;

unsigned int frequencyB;
double frequency = 0;

byte freq;

byte volMap[] = {0x3F,0x3D,0x3B,0x39,0x37,0x35,0x33,0x31,   
                  0x2F,0x2D,0x2B,0x29,0x27,0x25,0x23,0x21,   
                  0x1F,0x1D,0x1B,0x19,0x17,0x15,0x13,0x11,   
                  0x0F,0x0D,0x0B,0x09,0x07,0x05,0x03,0x00};
                          
byte lfAttMap[] = {0x9F,0x9D,0x9B,0x99,0x97,0x95,0x93,0x91,
                   0x8F,0x8D,0x8B,0x89,0x87,0x85,0x83,0x80};                          

byte rfAttMap[] = {0xBF,0xBD,0xBB,0xB9,0xB7,0xB5,0xB3,0xB1,
                   0xAF,0xAD,0xAB,0xA9,0xA7,0xA5,0xA3,0xA0}; 

byte lrAttMap[] = {0xDF,0xDD,0xDB,0xD9,0xD7,0xD5,0xD3,0xD1,
                   0xCF,0xCD,0xCB,0xC9,0xC7,0xC5,0xC3,0xC0}; 

byte rrAttMap[] = {0xFF,0xFD,0xFB,0xF9,0xF7,0xF5,0xF3,0xF1,
                   0xEF,0xED,0xEB,0xE9,0xE7,0xE5,0xE3,0xE0}; 
                            
byte bassMap[] = {0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x6F,
                  0x6E,0x6D,0x6C,0x6B,0x6A,0x69,0x68}; 

byte trebleMap[] = {0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x7F,
                    0x7E,0x7D,0x7C,0x7B,0x7A,0x79,0x78}; 
                            
byte currentVolume = 16;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

MeetAndroid meetAndroid;

void setup() {
 
  Serial.begin(38400); 
  
  Serial.flush();
  setCallbacks();
  initTempSensors();
  initTda();
  
  pinMode(ledPin, OUTPUT); 
  pinMode(relayPin, OUTPUT); 
  pinMode(accPin, INPUT); //acc +5v input for power button trigger
  //pinMode(AUDIO_ON, OUTPUT);
 pinMode(AUDIO_MUTE, OUTPUT);

digitalWrite(ledPin, HIGH); 
    delay(500);  
    digitalWrite(ledPin, LOW); 
    delay(500);
    digitalWrite(ledPin, HIGH); 
    delay(500);  
    digitalWrite(ledPin, LOW);

Serial.println("Ready");
   
}

void loop() {
  meetAndroid.receive();

//trig = digitalRead(accPin);
//  if (prev_trig != trig ) {
//    Serial.println("Acc state now: ");
//    Serial.print(trig);
//   digitalWrite(relayPin, LOW);
//   delay(500); // waits for 0,5 seconds for short press
//  digitalWrite(relayPin, HIGH);
//    prev_trig = trig;
//  }
}
void setCallbacks() {
 
  meetAndroid.registerFunction(getInternalTemperature, 'A'); //Internal temperature: 1
  meetAndroid.registerFunction(setAudioBalance, 'B'); //Audio balance: 0-15
  meetAndroid.registerFunction(getExternalTemperature, 'E'); //External temperature: 1
  meetAndroid.registerFunction(setAudioBass, 'J'); //Audio bass: 0-14
  meetAndroid.registerFunction(setAudioTreble, 'K'); //Audio treble: 0-14
  meetAndroid.registerFunction(setAudioRearLeftVolume, 'L'); //Audio rear left volume: 0-15
  meetAndroid.registerFunction(setAudioMute, 'M'); //Audio mute: 1-ON, 0-OFF
  meetAndroid.registerFunction(setRadioFrequency, 'R'); //Radio frequency: (880-1080)-ON, 0-OFF
  meetAndroid.registerFunction(setAudioSource, 'S'); //Audio source: 2-radio, 3-music   
  meetAndroid.registerFunction(setAudioVolume, 'V'); //Audio volume: 0-31
  meetAndroid.registerFunction(setAudioRearRightVolume, 'X'); //Audio rear right volume: 0-15
 
} 

void initTempSensors() {
  sensors.begin();
  // set the resolution to 9 bit
  sensors.setResolution(intTempSensor, 9);
  sensors.setResolution(extTempSensor, 9);
}

void initTda() {
  sendAudioMute(1);
  delay(1000);
  Wire.begin();
  sendAudioSwitch(2);
  sendAudioVolume(16);
  sendAudioLFAttenuator(15);
  sendAudioRFAttenuator(15);
  sendAudioLRAttenuator(15);
  sendAudioRRAttenuator(15);
  sendAudioBass(7);
  sendAudioTreble(7);
  sendAudioMute(0);
Serial.println("TDA init complete");

}

  
void writeI2c(byte address, byte value) {
  Wire.beginTransmission(address); 
  Wire.write(value);
  Wire.endTransmission();  
 Serial.print("I2C bytes: ");
 Serial.print(value, HEX);
 Serial.println();
 }


void sendAudioMute(byte value) {
  digitalWrite(AUDIO_MUTE, value == 1 ? LOW : HIGH);
}


void sendAudioVolume(byte value) {
  if (value > 31) return;
  currentVolume = value;
  writeI2c(TDA7318_I2C_ADDRESS, volMap[value]);
    Serial.print("VOLUME: ");
  Serial.println(value);
  
}


void sendAudioLFAttenuator(byte value) {   
  if (value > 15) return;
  writeI2c(TDA7318_I2C_ADDRESS, lfAttMap[value]);   
}

void sendAudioRFAttenuator(byte value) {   
  if (value > 15) return;
  writeI2c(TDA7318_I2C_ADDRESS, rfAttMap[value]);   
}

void sendAudioLRAttenuator(byte value) {   
  if (value > 15) return;
  writeI2c(TDA7318_I2C_ADDRESS, lrAttMap[value]);   
}

void sendAudioRRAttenuator(byte value) {   
  if (value > 15) return;
  writeI2c(TDA7318_I2C_ADDRESS, rrAttMap[value]);   
}

void sendAudioBass(byte value) {   
  if (value > 14) return;   
  writeI2c(TDA7318_I2C_ADDRESS, bassMap[value]);   
}

void sendAudioTreble(byte value) {   
  if (value > 14) return;   
  writeI2c(TDA7318_I2C_ADDRESS, trebleMap[value]);   
}

void sendAudioSwitch(byte value) {
  switch (value) {
    case 1:
      writeI2c(TDA7318_I2C_ADDRESS, TDA_SW1);   
      break;
    case 2:
      writeI2c(TDA7318_I2C_ADDRESS, TDA_SW2);   
      break;
    case 3:
      writeI2c(TDA7318_I2C_ADDRESS, TDA_SW3);   
      break;
    case 4:
      writeI2c(TDA7318_I2C_ADDRESS, TDA_SW4);   
      break;      
  }  
} 

void sendRadioFrequency(int frequency, boolean power) 
{
 frequencyB = frequency * 2 + 0x2000;
Serial.println(frequencyB, HEX);
frequencyH = frequencyB >> 8;
Serial.println(frequencyH, HEX);
frequencyL = frequencyB & 0XFF;
Serial.println(frequencyL, HEX);

  delay(100);
  Wire.beginTransmission(0x60);
 Wire.write(0x20);
  Wire.write(frequencyH);
  Wire.write(frequencyL);
    Wire.endTransmission();
  delay(100); 
}

void getInternalTemperature(byte flag, byte numOfValues) {
  sensors.requestTemperaturesByAddress(intTempSensor);
  float tempC = sensors.getTempC(intTempSensor);
  if (tempC == -127.00) {
    tempC = 0;
  }  
  char buf[50];
  sprintf(buf, "IT:%u", (int)tempC);
  meetAndroid.send(buf);
}

void getExternalTemperature(byte flag, byte numOfValues) {
  sensors.requestTemperaturesByAddress(extTempSensor);
  float tempC = sensors.getTempC(extTempSensor);
  if (tempC == -127.00) {
    tempC = 0;
  }  
  char buf[50];
  sprintf(buf, "ET:%u", (int)tempC);
  meetAndroid.send(buf);
}

void setAudioBalance(byte flag, byte numOfValues) {
  int value = meetAndroid.getInt();
  sendAudioLFAttenuator(value);
  sendAudioRFAttenuator(15 - value);
}


void setAudioBass(byte flag, byte numOfValues) {
  sendAudioBass(meetAndroid.getInt());
}

void setAudioTreble(byte flag, byte numOfValues) {
  sendAudioTreble(meetAndroid.getInt());
}

void setAudioRearLeftVolume(byte flag, byte numOfValues) {
  sendAudioLRAttenuator(meetAndroid.getInt());
}

void setAudioMute(byte flag, byte numOfValues) {
  sendAudioMute(meetAndroid.getInt());
}

void setRadioFrequency(byte flag, byte numOfValues) {
  double frequency = meetAndroid.getInt();
  if (frequency == 0) {
    sendRadioFrequency(frequency, false);
   sendAudioSwitch(MUSIC_SOURCE);
  }
  else if (frequency >= RADIO_MIN_FREQUENCY && frequency <= RADIO_MAX_FREQUENCY){
    sendRadioFrequency(frequency, true);
   sendAudioSwitch(RADIO_SOURCE);
  }  
}

void setAudioSource(byte flag, byte numOfValues) {
  sendAudioSwitch(meetAndroid.getInt());
}

void setAudioVolume(byte flag, byte numOfValues) {
  sendAudioVolume(meetAndroid.getInt());
  
}

void setAudioRearRightVolume(byte flag, byte numOfValues) {
  sendAudioRRAttenuator(meetAndroid.getInt());
}

