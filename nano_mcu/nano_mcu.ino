#include <Wire.h> //for i2c
#include <MeetAndroid.h> //for android 
#include <OneWire.h> //for temperature sensor communication
#include <DallasTemperature.h> //for temperature sensor data
                                                                                                                                                                         

int trig; //power trigger current state
int prev_trig = 0; //power trigger previous state

#define ONE_WIRE_BUS 15
#define accPin 2 //D2
#define ledPin 13 
#define relayPin 3 //D3 output to power button

#define TDA7318_I2C_ADDRESS 0x44
#define TDA_SW1 0x3A
#define TDA_SW2 0x39
#define TDA_SW3 0x20
#define TDA_SW4 0x3B
#define TDA_SUB_INPUT 0x0
//#define TDA_SUB_VOL0 0x11
//#define TDA_SUB_VOL1 0x10
#define TDA_SUB_VOL 0x2
#define TDA_SUB_BASS_TREBLE 0x3
#define TDA_SUB_ATT_LF 0x4
#define TDA_SUB_ATT_LR 0x5
#define TDA_SUB_ATT_RF 0x6
#define TDA_SUB_ATT_RR 0x7
#define TDA_SUB_MUTE 0x8
#define TDA_SOFT_MUTE 0x8

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

byte volMap[] = { 0xFF,0xFE,0xF0,0xE3,0xD8,0xCE,0xC5,0xBD,   
                  0xB5,0xAE,0xA7,0xA0,0x99,0x93,0x8D,0x87,   
                  0x81,0x7B,0x75,0x6F,0x6A,0x65,0x60,0x5B,
		  0x57,0x53,0x4F,0x48,0x43,0x3A,0x2F,0x23};
                          
byte lfAttMap[] = {0x1F,0x1C,0x1A,0x18,0x16,0x14,0x12,0x10,
                   0xE,0xC,0xA,0x8,0x6,0x4,0x2,0x1B};                          

byte rfAttMap[] =  {0x1F,0x1C,0x1A,0x18,0x16,0x14,0x12,0x10,
                   0xE,0xC,0xA,0x8,0x6,0x4,0x2,0x1B};

byte lrAttMap[] = {0x1F,0x1C,0x1A,0x18,0x16,0x14,0x12,0x10,
                   0xE,0xC,0xA,0x8,0x6,0x4,0x2,0x1B}; 

byte rrAttMap[] = {0x1F,0x1C,0x1A,0x18,0x16,0x14,0x12,0x10,
                   0xE,0xC,0xA,0x8,0x6,0x4,0x2,0x1B}; 
                            
byte bassMap[] = {0x20,0x30,0x40,0x50,0x60,0xF0,0xE0,0xD0,
                  0xC0,0xB0,0xA0,0x90,0x80,0x10,0x0}; 

byte trebleMap[] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0xF,
                    0xE,0xD,0xC,0xB,0xA,0x9,0x8}; 
                            
byte currentVolume = 7;

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
 // pinMode(AUDIO_MUTE, OUTPUT);

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

trig = digitalRead(accPin);
  if (prev_trig != trig ) {
    Serial.println("Acc state now: ");
    Serial.print(trig);
   digitalWrite(relayPin, LOW);
   delay(500); // waits for 0,5 seconds for short press
  digitalWrite(relayPin, HIGH);
    prev_trig = trig;
  }
}
void setCallbacks() {
 
  meetAndroid.registerFunction(getInternalTemperature, 'A'); //Internal temperature: 1
  meetAndroid.registerFunction(setAudioBalance, 'B'); //Audio balance: 0-15
  meetAndroid.registerFunction(getExternalTemperature, 'E'); //External temperature: 1
  meetAndroid.registerFunction(setAudioBass, 'J'); //Audio bass: 0-14
  meetAndroid.registerFunction(setAudioTreble, 'K'); //Audio treble: 0-14
  meetAndroid.registerFunction(setAudioRearLeftVolume, 'L'); //Audio rear left volume: 0-15
//  meetAndroid.registerFunction(setAudioMute, 'M'); //Audio mute: 1-ON, 0-OFF
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
//sendAudioMute(0);
//sendAudioOn(1);
//  delay(3000);
  Wire.begin(); // join i2c bus (address optional for master)
//  sendAudioMute(1);
  sendAudioVolume(7);
  sendAudioLFAttenuator(15);
  sendAudioRFAttenuator(15);
  sendAudioLRAttenuator(15);
  sendAudioRRAttenuator(15);
  sendAudioSwitch(3);  
  sendAudioBass(7);
  sendAudioTreble(7);
//  sendAudioMute(0);

}

  
void writeI2c(byte address, byte subaddress, byte value) {

  Wire.beginTransmission(address); 
  Wire.write(subaddress);
  Wire.write(value);
  Wire.endTransmission();
 }

void writeI2c_vol(byte address, byte subaddress0, byte subaddress1, byte value) {

  Wire.beginTransmission(address); 
  Wire.write(subaddress0);
  Wire.write(subaddress1);
  Wire.write(value);
  Wire.endTransmission();
}

void sendAudioMute(byte value) {
  writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_MUTE, TDA_SOFT_MUTE);
}

//void sendAudioVolume(byte value) {
//  if (value > 31) return;
//  currentVolume = value;
//  writeI2c_vol(TDA7318_I2C_ADDRESS, TDA_SUB_VOL0, TDA_SUB_VOL1, volMap[value]);
//  Serial.println(value);
//  
//} 

void sendAudioVolume(byte value) {
  if (value > 31) return;
  currentVolume = value;
  writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_VOL, volMap[value]);
  Serial.println(value);
  
}


void sendAudioLFAttenuator(byte value) {   
  if (value > 15) return;
  writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_ATT_LF, lfAttMap[value]);   
}

void sendAudioRFAttenuator(byte value) {   
  if (value > 15) return;
  writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_ATT_RF, rfAttMap[value]);   
}

void sendAudioLRAttenuator(byte value) {   
  if (value > 15) return;
  writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_ATT_LR, lrAttMap[value]);   
}

void sendAudioRRAttenuator(byte value) {   
  if (value > 15) return;
  writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_ATT_RR, rrAttMap[value]);   
}

void sendAudioBass(byte value) {   
  if (value > 14) return;   
  writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_BASS_TREBLE, bassMap[value]);   
}

void sendAudioTreble(byte value) {   
  if (value > 14) return;   
  writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_BASS_TREBLE, trebleMap[value]);   
}

void sendAudioSwitch(byte value) {
  switch (value) {
    case 1:
      writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_INPUT, TDA_SW3);   
      break;
    case 2:
      writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_INPUT, TDA_SW1);   
      break;
    case 3:
      writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_INPUT, TDA_SW2);   
      break;
    case 4:
      writeI2c(TDA7318_I2C_ADDRESS, TDA_SUB_INPUT, TDA_SW4);   
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

