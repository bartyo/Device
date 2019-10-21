
#include <string.h>
#include <Arduino.h>
#include <bluefruit.h>
#include <Wire.h>

/* #################################################################### */

/* TO DO : */
// Set I2C Adresses
// Init everything
// Build Read (UART) Token function
// Build TokenAction Function
// Send_Error function (if TokenAction return false)

/* #################################################################### */

/*I2C Adresses*/
#define SPO2_CONFIG                   0x0A
/*etc...*/

String Device_Name = "ProtoNode";
String Manu_Name = "AdafruitInc";
String Model_Name = "ProtoAda01";

boolean ReadMode = false;     // Token reading protocol (false == UART , true == BLE)
String TOKEN = "NOP";       // Token buffer
byte BLE_Trame[20];           // BLE payload to send

/* #################################################################### */

//TOKENS :
//  NOP -> Do Nothing
//  OXI -> Gather All Photodiodes Data
//  TMP -> Gather Temperature
//  LIR -> Gather IR Absorbance
//  LRD -> Gather RED Absorbance
//  LGD -> Gather GREEN Absorbance
//  ERR -> Error test
//  BLK -> Blink test
//  ETC..

/* #################################################################### */

void Read_Token(boolean mode){
  if(mode==true){
    /* read trough BLE -> buffer */
  }
  else{
    /* read trough UART -> buffer */
    TOKEN = Serial.readString();
    TOKEN = TOKEN.substring(0,3);
    Serial.println(TOKEN);
    Serial.flush(); 
  }
}
/* ------------------------------------------------------------------- */
boolean TokenAction(void){
 boolean done = false;
 
 if(TOKEN == "NOP"){
     digitalWrite(LED_BUILTIN, LOW);        // Cancel error led if needed
     done = true;                           // Token recognised
 }
 
 else if(TOKEN == "BLK"){           // Blink red led
     for(int i=0;i<5;i++){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
     }
     done = true;                           // Token recognised
 }
      
 else if(TOKEN == "ERR"){           // Simulate an error
     digitalWrite(LED_BUILTIN, HIGH);       // Red led ON
 }
      
 else{                                 // Token not recognised
     digitalWrite(LED_BUILTIN, LOW);        // Cancel error led if needed    
 }
  
  return done;  
}
/* ------------------------------------------------------------------- */
void Send_Error(boolean is_success, boolean mode){
  if(mode==true){
    /* send error message trough BLE */
    if(!is_success){}
  }
  else{
    /* send error message trough UART */
    if(!is_success){Serial.println("Error Raised.");}
  }
}

/* #################################################################### */

void setup() {
  
  Serial.begin(9600);
  Serial.println("Prototype Device Initialisation");
  Serial.println("--------------------------\n");

  /* Init some variable and memory */
  /* Init I2C */
  /* Init GPIO */
  pinMode(LED_BUILTIN, OUTPUT);
  /* Init BlueTooth */

  Serial.println("Prototype Device Routine");
  Serial.println("--------------------------\n");
}

/* #################################################################### */

void loop() {

  /* UART MAIN Version */
  while (Serial.available() > 0) {
    Read_Token(ReadMode);                // Read Token (from UART or BLE)
    Send_Error(TokenAction(),ReadMode);  // Call functions designatd by the Token. Raise an error if needed.
  }
  
}
