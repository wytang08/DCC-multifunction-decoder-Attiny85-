// This function decoder is modified from nmraDCC example by Henry Tang
// Circuit was tested on LokProgrammer (receive data)
// Reading CV may fail.  
// Writting CV is possible.  Error message may be given by Lokprogrammer but the EEPROM can be written with the new CV.

#include <NmraDcc.h>

/*  Uncomment for debugging using Serial Port
 *  Connection to FTDI serial comm. module
 *  Attiny85 --> FTDI
 *  PB3 (rx) --> TX
 *  PB4 (tx) --> RX
 
#include <SoftwareSerial.h>

#define rx 3 
#define tx 4
SoftwareSerial Serial(rx, tx);
*/

// Define the Arduino input Pin number for the DCC Signal 
#define DCC_PIN     2

// Attiny PWM pins 0,1 can adjust brightness of LED
// pin 3,4 are used as digital outputs
// pin 5 is used as reset and cannot be used as output
// pin 2 is used as INTERRUPT handling
// All room lights are turned on/off together for YW
int FunctionPin0 = 0;  // Door and RW corridor lights
int FunctionPin1 = 1;  // Room lights and YW corridor lights 
int FunctionPin3 = 3;  // EXT1
int FunctionPin4 = 4;  // EXT2 
/*
int FunctionPin5 = 5;  // Reset pin
int FunctionPin5 = 11;  // 
int FunctionPin6 = 13;  // For testing the onboard LED
int DccAckPin = 8 ;  // Pin for acknowledging programming
*/
int CV_BRIGHTNESS = 20;  // CV20 for adjusting LED brightness of room and corridor lights
int DEFAULT_BRIGHTNESS = 160;

/*

int CV_BRIGHTNESS_EXT1 = 21;
int DEFAULT_BRIGHTNESS_EXT1 = 255;  // CV21 for adjusting PWM frequency of EXT1 

int CV_BRIGHTNESS_EXT2 = 22;
int DEFAULT_BRIGHTNESS_EXT2 = 255;  // CV22 for adjusting PWM frequency of EXT2
*/
int CV_RESET_FACTORY_DEFAULT = 30; //CV for resetting the decoder to default values
int RESET_FACTORY_DEFAULT = 16;  // Default value of CV30
int RESET_FACTORY_VALUE = 2; // The value written to CV30 for resetting the decoder

int CV_DC_DEFAULT_OUTPUT = 23;  // Set DC mode output
int DC_DEFAULT_OUTPUT = 0; 

struct CVPair
{
  uint16_t  CV;
  uint8_t   Value;
};

CVPair FactoryDefaultCVs [] =
{
	// The CV Below defines the Short DCC Address
  {CV_MULTIFUNCTION_PRIMARY_ADDRESS, DEFAULT_MULTIFUNCTION_DECODER_ADDRESS}, // CV1 default value is 3

  // These two CVs define the Long DCC Address
  {CV_MULTIFUNCTION_EXTENDED_ADDRESS_MSB, CALC_MULTIFUNCTION_EXTENDED_ADDRESS_MSB(DEFAULT_MULTIFUNCTION_DECODER_ADDRESS)}, // CV17, default is 192
  {CV_MULTIFUNCTION_EXTENDED_ADDRESS_LSB, CALC_MULTIFUNCTION_EXTENDED_ADDRESS_LSB(DEFAULT_MULTIFUNCTION_DECODER_ADDRESS)},// CV18, default is 3 

// ONLY uncomment 1 CV_29_CONFIG line below as approprate
//  {CV_29_CONFIG,                                      0}, // Short Address 14 Speed Steps
  {CV_29_CONFIG,                       CV29_F0_LOCATION}, // Short Address 28/128 Speed Steps
//  {CV_29_CONFIG, CV29_EXT_ADDRESSING | CV29_F0_LOCATION},  // Long  Address 28/128 Speed Steps

  {CV_RESET_FACTORY_DEFAULT, RESET_FACTORY_DEFAULT}, // CV8 for reset factory default
  {CV_BRIGHTNESS, DEFAULT_BRIGHTNESS},               // CV20 for LED brightness of rooms and corridor 
//  {CV_BRIGHTNESS_EXT1, DEFAULT_BRIGHTNESS_EXT1},     // CV21 for PWM frequency of EXT1
//  {CV_BRIGHTNESS_EXT2, DEFAULT_BRIGHTNESS_EXT2}      // CV22 for PWM frequency of EXT2
  {CV_DC_DEFAULT_OUTPUT, DC_DEFAULT_OUTPUT}

};

NmraDcc  Dcc ;

uint8_t FactoryDefaultCVIndex = 0;

// Uncomment this line below to force resetting the CVs back to Factory Defaults
// FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs)/sizeof(CVPair);

void notifyCVResetFactoryDefault()
{
  // Make FactoryDefaultCVIndex non-zero and equal to num CV's to be reset 
  // to flag to the loop() function that a reset to Factory Defaults needs to be done
  FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs)/sizeof(CVPair);
};
/*
// Uncomment the #define below to print all Speed Packets
#define NOTIFY_DCC_SPEED
#ifdef  NOTIFY_DCC_SPEED
void notifyDccSpeed( uint16_t Addr, DCC_ADDR_TYPE AddrType, uint8_t Speed, DCC_DIRECTION Dir, DCC_SPEED_STEPS SpeedSteps )
{
  Serial.print("notifyDccSpeed: Addr: ");
  Serial.print(Addr,DEC);
  Serial.print( (AddrType == DCC_ADDR_SHORT) ? "-S" : "-L" );
  Serial.print(" Speed: ");
  Serial.print(Speed,DEC);
  Serial.print(" Steps: ");
  Serial.print(SpeedSteps,DEC);
  Serial.print(" Dir: ");
  Serial.println( (Dir == DCC_DIR_FWD) ? "Forward" : "Reverse" );

};
#endif
*/
// Uncomment the #define below to print all Function Packets
#define NOTIFY_DCC_FUNC
#ifdef  NOTIFY_DCC_FUNC



void notifyDccFunc(uint16_t Addr, DCC_ADDR_TYPE AddrType, FN_GROUP FuncGrp, uint8_t FuncState)
{
/*  
  Serial.print("notifyDccFunc: Addr: ");
  Serial.print(Addr,DEC);
  Serial.println( (AddrType == DCC_ADDR_SHORT) ? 'S' : 'L' );
  Serial.print("  Function Group: ");
  Serial.print(FuncGrp,DEC);
*/
  switch( FuncGrp )
   {
/*
#ifdef NMRA_DCC_ENABLE_14_SPEED_STEP_MODE    
     case FN_0:
       Serial.print(" FN0: ");
       Serial.println((FuncState & FN_BIT_00) ? "1  " : "0  "); 
       break;
#endif
*/       
     case FN_0_4:
       if(Dcc.getCV(CV_29_CONFIG) & CV29_F0_LOCATION) // Only process Function 0 in this packet if we're not in Speed Step 14 Mode
       {
//         Serial.print(" FN 0: ");
//         Serial.print((FuncState & FN_BIT_00) ? "1  ": "0  ");
         analogWrite(FunctionPin0, ((FuncState & FN_BIT_00) ? 1 : 0)*Dcc.getCV(CV_BRIGHTNESS) ); // Send the Function state to Function pin 0 - Door and RW corridor lights
            
       }
       
//       Serial.print(" FN 1-4: ");
//       Serial.print((FuncState & FN_BIT_01) ? "1  ": "0  ");
       if (((FuncState & FN_BIT_01) ? "1  ": "0  ")) {
         analogWrite(FunctionPin1, ((FuncState & FN_BIT_01) ? 1 : 0)*Dcc.getCV(CV_BRIGHTNESS) ); // Send the Function state to Function pin 1 - Room lights and YW corridor lights
//         analogWrite(FunctionPin5, ((FuncState & FN_BIT_01) ? 1 : 0)*Dcc.getCV(CV_BRIGHTNESS) ); // Send the Function state to Function pin 5 - YW Room lights 2
//         analogWrite(FunctionPin1, ((FuncState & FN_BIT_01) ? 1 : 0)*Dcc.getCV(CV_BRIGHTNESS) ); // Send the Function state to Function pin 1 - YW Room lights 3 

//         break;             
       }
      
//       Serial.print((FuncState & FN_BIT_02) ? "1  ": "0  ");
       digitalWrite(FunctionPin3, ((FuncState & FN_BIT_02) ? 1 : 0) ); // Send the Function state to Function pin 3 - EXT 1          
          
//       Serial.print((FuncState & FN_BIT_03) ? "1  ": "0  ");
       digitalWrite(FunctionPin4, ((FuncState & FN_BIT_03) ? 1 : 0) ); // Send the Function state to Function pin 4 - EXT 2    
              
/*       
       Serial.println((FuncState & FN_BIT_04) ? "1  ": "0  ");
       digitalWrite(FunctionPin5, ((FuncState & FN_BIT_04) ? 1 : 0) ); // Send the Function state to Function pin 1 - RW Room lights 3
       break;
    
     case FN_5_8:
       Serial.print(" FN 5-8: ");
       Serial.print((FuncState & FN_BIT_05) ? "1  ": "0  ");
//       analogWrite(FunctionPin0,((FuncState & FN_BIT_05) ? 1 : 0 )*Dcc.getCV(CV_BRIGHTNESS_EXT1)); // Send the Function state to Function pin 0 - EXT1  
           
       Serial.print((FuncState & FN_BIT_06) ? "1  ": "0  ");
//       analogWrite(FunctionPin2,((FuncState & FN_BIT_06) ? 1 : 0 )*Dcc.getCV(CV_BRIGHTNESS_EXT2)); // Send the Function state to Function pin 2 - EXT2
       
       Serial.print((FuncState & FN_BIT_07) ? "1  ": "0  ");
//       digitalWrite(FunctionPin6,(FuncState & FN_BIT_07) ? 1 : 0 ); // Send the Function state to Function pin 6 - On board LED for testing
       break;
    
     case FN_9_12:
       Serial.print(" FN 9-12: ");
       Serial.print((FuncState & FN_BIT_09) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_10) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_11) ? "1  ": "0  ");
       Serial.println((FuncState & FN_BIT_12) ? "1  ": "0  ");
       break;

     case FN_13_20:
       Serial.print(" FN 13-20: ");
       Serial.print((FuncState & FN_BIT_13) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_14) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_15) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_16) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_17) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_18) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_19) ? "1  ": "0  ");
       Serial.println((FuncState & FN_BIT_20) ? "1  ": "0  ");
       break;
  
     case FN_21_28:
       Serial.print(" FN 21-28: ");
       Serial.print((FuncState & FN_BIT_21) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_22) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_23) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_24) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_25) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_26) ? "1  ": "0  ");
       Serial.print((FuncState & FN_BIT_27) ? "1  ": "0  ");
       Serial.println((FuncState & FN_BIT_28) ? "1  ": "0  ");
       break;  
*/
   }

}
#endif

/*
// Uncomment the #define below to print all DCC Packets
#define NOTIFY_DCC_MSG
#ifdef  NOTIFY_DCC_MSG

void notifyDccMsg( DCC_MSG * Msg)
{

  
  Serial.print("notifyDccMsg: ") ;
  for(uint8_t i = 0; i < Msg->Size; i++)
  {
    Serial.print(Msg->Data[i], HEX);
    Serial.write(' ');
  }

  Serial.println();


}
#endif
*/

// This function is called by the NmraDcc library when a DCC ACK needs to be sent
// Calling this function should cause an increased 60ma current drain on the power supply for 6ms to ACK a CV Read
 
int i = 0;

void notifyCVAck(void)
{
//  Serial.println("notifyCVAck") ;


  digitalWrite( FunctionPin0, HIGH );
  digitalWrite( FunctionPin1, HIGH );  
  delay( 6 );  
  digitalWrite( FunctionPin0, LOW );
  digitalWrite( FunctionPin1, LOW );
/*
  // Check CV values stored in EEPROM 
  FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs)/sizeof(CVPair);
  for(i=0; i<FactoryDefaultCVIndex; i++) {
      Serial.print(" CV");
      Serial.print(FactoryDefaultCVs[i].CV); 
      Serial.print(" : ");   
      Serial.println(FactoryDefaultCVs[i].Value);          
  }
  Serial.println();
*/   
  if (Dcc.getCV(CV_RESET_FACTORY_DEFAULT) == RESET_FACTORY_VALUE)  // Write 2 to CV30 to reset the decoder
     notifyCVResetFactoryDefault();  

}



void setup()
{
//  Serial.begin(9600);
//  Serial.println("NMRA Dcc Multifunction Decoder Demo 1");

  // Configure the DCC CV Programing ACK pin for an output
//  pinMode( DccAckPin, OUTPUT );
//  digitalWrite( DccAckPin, LOW );
  pinMode( FunctionPin0, OUTPUT); 
  pinMode( FunctionPin1, OUTPUT);
  pinMode( FunctionPin3, OUTPUT);
  pinMode( FunctionPin4, OUTPUT);
//  pinMode( FunctionPin5, OUTPUT);
//  pinMode( FunctionPin5, OUTPUT);
//  pinMode( FunctionPin6, OUTPUT);            
  
  // Setup which External Interrupt, the Pin it's associated with that we're using and enable the Pull-Up
  // Many Arduino Cores now support the digitalPinToInterrupt() function that makes it easier to figure out the
  // Interrupt Number for the Arduino Pin number, which reduces confusion. 
#ifdef digitalPinToInterrupt
  Dcc.pin(DCC_PIN, 0);
#else
  Dcc.pin(0, DCC_PIN, 1);
#endif
  
  // Call the main DCC Init function to enable the DCC Receiver
  //Dcc.init( MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER | CV29_OUTPUT_ADDRESS_MODE, 0 );

  Dcc.init( MAN_ID_DIY, 10, FLAGS_MY_ADDRESS_ONLY, 0 );

  // Uncomment to force CV Reset to Factory Defaults
  // notifyCVResetFactoryDefault();

  analogWrite(FunctionPin0,  (Dcc.getCV(CV_DC_DEFAULT_OUTPUT) && 0b00000001)*Dcc.getCV(CV_BRIGHTNESS) );  // Check bit 0 to set DC output
  analogWrite(FunctionPin1, ((Dcc.getCV(CV_DC_DEFAULT_OUTPUT) && 0b00000010)>>1)*Dcc.getCV(CV_BRIGHTNESS) );  // Check bit 1 to set DC output
  analogWrite(FunctionPin3, ((Dcc.getCV(CV_DC_DEFAULT_OUTPUT) && 0b00000100)>>2)*Dcc.getCV(CV_BRIGHTNESS) );  // Check bit 2 to set DC output
  analogWrite(FunctionPin4, ((Dcc.getCV(CV_DC_DEFAULT_OUTPUT) && 0b00001000)>>3)*Dcc.getCV(CV_BRIGHTNESS) );  // Check bit 3 to set DC output
 
}

void loop()
{
  // You MUST call the NmraDcc.process() method frequently from the Arduino loop() function for correct library operation
  Dcc.process();
  
  if( FactoryDefaultCVIndex && Dcc.isSetCVReady())
  {
    FactoryDefaultCVIndex--; // Decrement first as initially it is the size of the array 
    Dcc.setCV( FactoryDefaultCVs[FactoryDefaultCVIndex].CV, FactoryDefaultCVs[FactoryDefaultCVIndex].Value);
  }
  
}
