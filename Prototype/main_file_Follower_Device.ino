
#include <string.h>
#include <Arduino.h>
#include <bluefruit.h>
#include <Wire.h>

/* #################################################################### */
/* To Do */
// Check if I2C is working or not
// Check what is needed to trigger a data conversion (important!)
// Do BLE Functions
// Do buffered Oximetry function
/* #################################################################### */

/*PINOUT*/
#define CHIP_INT             27           //Chip Interrpution

/*I2C Adresses*/
#define INT_EN_1                  0x02    //Interrupt condition for data (W)
#define INT_EN_2                  0x03    //Interrupt condition for temperature (W)
#define FIFO_WP                   0x04    //Last added data adress (write pointer) (R)
#define OF_COUNT                  0x05    //Overflow counter (R)
#define FIFO_RP                   0x06    //Last read data adress (read pointer) (W/R)
#define FIFO_DR                   0x07    //DATA REGISTER (3x1 byte for 1 channel , see datasheet) (R)
#define FIFO_CONFIG               0x08    //FIFO configuration (W/R)
#define MODE_CONFIG               0x09    //Chip mode (W)
#define SPO2_CONFIG               0x0A    //Oximetry configuration (W)
#define LED1_AMP                  0x0C    //Led 1 amplitude (W)
#define LED2_AMP                  0x0D    //Led 2 amplitude (W)
#define TMP_INT                   0x1F    //Temperature troncated into Integer (ex:12) (R)
#define TMP_FRA                   0x20    //Temperature into Fractional (ex:12.5) (R)
#define TMP_CONFIG                0x21    //Temperaure measurment enable
/*etc...*/

/*I2C Commands - Modify if needed*/
#define INT1_SETUP                B01000000     //Interrupt when a new oximetry data is available
#define INT2_SETUP                B00000010     //Interrupt when a new temperature data is available
#define FIFO_SETUP                B00000000     //No averaging, No roll-over
#define SHUTDOWN                  B10000000     //Shutdown the chip (keeps memory on)
#define RESET                     B01000000     //Reset the chip (keeps memory on)
#define MODE_SETUP                B00000011     //SPO2 Mode
#define SPO2_SETUP                B00100001     //4mA range, 50sps, 118µs pulse (16bit resolution)
#define LED1_SETUP                0x3F          //12.6mA
#define LED2_SETUP                0x3F          //12.6mA
#define TMP_ENABLE               B00000001     //Enable Temperature measurement
/*etc...*/

String Device_Name = "ProtoNode";
String Manu_Name = "AdafruitInc";
String Model_Name = "ProtoAda01";

boolean ComMode = false;              // Token reading protocol (false == UART , true == BLE)
boolean HWMode = false;               // Hardware Interaction protocol (false == Simulated , true == GPIOs)
String TOKEN = "NOP";                 // Token buffer
byte BLE_Payload[20] = { 0 };         // BLE payload to send

#define MAX_SIZE   4000              //Maximum accepted samples value (need < 16bit)
#define SAMPLES     6
byte Data_Buffer[MAX_SIZE][SAMPLES] = { 0 };  // Max 4000 samples made of 6 bytes - Buffer for buffered oximetry mode

struct I2C_PACK {
  byte adress;        //I2C adress
  byte load;       //I2C buffer to send/read data
  boolean w;          //write ?
};

struct OXI_SETTING {
  byte nb_pts[2];
  byte sampling_freq;
  byte range;
  byte pulse_width;
  byte intensity;
};

I2C_PACK      I2C_COM = {0x00,0x00,false};                        // Global I2C structure
OXI_SETTING   OXI_SET = {{0x00,0x32},0x0,0x1,0x1,0x3F};           // Global Oximetry Settings structure

/* #################################################################### */

//TOKENS :
//  NOP -> Do Nothing
//  ERR -> Error test
//  BLK -> Blink test
//  OXI -> Gather All Photodiodes Data (single)
//  OXB -> Gather All Photodiodes Data (buffered)
//  TMP -> Gather Die Temperature
//  MOD -> Switch the COM mode
//  HWP -> Switch the Hardware Interaction mode
//  STR -> See the Oximetry Structure Variable (UART Only, for now)
//  PTS -> Set number of point (buffered)
//  SMP -> Set sampling freq.
//  AMP -> Set chip range
//  PUL -> Set chip pulse
//  INT -> Set current conso
//  ETC..

/* #################################################################### */

void Read_Token(void){
  TOKEN = "NOP";                                                  // Reset TOKEN to avoid surprises
  if(ComMode==true){
    /* read trough BLE -> buffer */
  }
  else{
    /* read trough UART -> buffer */
    TOKEN = Serial.readString();                                  // Read TOKEN as a string
    TOKEN = TOKEN.substring(0,3);                                 // Avoid the \n
    Serial.flush();                                               // Clean the COM
  }
}
/* ------------------------------------------------------------------- */
void Send_Payload(void){
  if(ComMode==true){
    /* write trough BLE*/
  }
  else{
    /* write trough UART */
    Serial.println(String((char*)BLE_Payload));                   // Transform the byte array into a char array so it can be sent as a string into hte serial COM
  }
}
/* ------------------------------------------------------------------- */
void Read_Payload(void){
  BLE_Payload[20] = { 0 };                                        // Reset the payload to avoid surprises
  if(ComMode==true){
    /* read trough BLE*/
  }
  else{
    /* read trough UART */
    while (Serial.available() == 0) {}                            // Wait until data comes
    (Serial.readString()).getBytes(BLE_Payload,20);               // Read serial datas as a string, then transform it as a char array, then put it into the Byte Payload; 
    Serial.flush();                                               // Clean the COM
  }
}
/* ------------------------------------------------------------------- */
/* Allow to send command and read data via I2C.*/
/* As the I2C structure buffer is 1 byte long, is cannot manage to gather multiples data in burst mode */
/* So you need to re-send a read command each time. If it doesn't fit, make a new dedicated function :) */
void I2C_interact(byte A,byte B,boolean w){
  
     I2C_COM.adress = A;
     I2C_COM.load = B;
     I2C_COM.w = w;
     
  if(HWMode==true){
    /* use GPIO*/
    if(I2C_COM.w == true){
      Wire.beginTransmission(0x00);                                                                         //Master I2C Send
      Wire.write(I2C_COM.adress);                                                                           //Register Adress
      Wire.write(I2C_COM.load);                                                                             //Register Content
      Wire.endTransmission();                                                                               //STOP
      if(ComMode==false){Serial.printf("@%02x Send: %02x\n",(char)I2C_COM.adress,(char)I2C_COM.load);}      //Displays what happened on the monitor if ComMode == UART
    }
    else{
      /* Get one octet */
      Wire.beginTransmission(0x00);
      Wire.write(I2C_COM.adress);
      Wire.endTransmission(false);                                                                           // Condition RESTART
      Wire.requestFrom(0x00, 1);                                                                             // One byte is required
      if (Wire.available()) {I2C_COM.load = Wire.read();}                                                    // Returned byte stored into the I2C Load
      if(ComMode==false){Serial.printf("@%02x Received: %02x\n",I2C_COM.adress,I2C_COM.load);}
    }    
  }
  
  else{
    /* (simply) simulated */
    if(I2C_COM.w == true){
      if(ComMode==false){Serial.printf("@%02x Send: %02x\n",(char)I2C_COM.adress,(char)I2C_COM.load);}
    }
    else{
      I2C_COM.load = byte(rand());                                                                            //Simulate the I2C register reading
      if(ComMode==false){Serial.printf("@%02x Received: %02x\n",I2C_COM.adress,I2C_COM.load);}
    }
  }
  
}
/* ------------------------------------------------------------------- */
/* Displays OXI_SET structure if UART Mode is ON (ComMode = false) */
void OXI_SETTINGS_UART(void){
  if(ComMode==false){Serial.printf("nb_pts : Ox%02x%02x\nSampling frequency Index: Ox%02x\nRange Index: Ox%02x\nPulse Width Index: Ox%02x\nIntensity : Ox%02x\n",OXI_SET.nb_pts[0],OXI_SET.nb_pts[1],OXI_SET.sampling_freq,OXI_SET.range,OXI_SET.pulse_width,OXI_SET.intensity);}
}
/* ------------------------------------------------------------------- */
/* Send an error message if TokenAction or other events returns false */
void Send_Error(boolean is_success){
  if(ComMode==true){
    /* send error message trough BLE */
    if(!is_success){}
  }
  else{
    /* send error message trough UART */
    if(!is_success){Serial.println("Error Raised.");}
  }
}
/* ------------------------------------------------------------------- */
/* Token to Functions attributions */
boolean TokenAction(void){
 boolean done = false;                      // Unset the "DONE" flag to generate error in some statement is passed over.
 BLE_Payload[20] = { 0 };                   // Reset the payload to avoid surprises
 
 if(TOKEN == "NOP"){
     digitalWrite(LED_BUILTIN, LOW);        // Cancel error led if needed
     done = true;                           // Token recognised
 }
      
 else if(TOKEN == "ERR"){                   // Simulate an error
     digitalWrite(LED_BUILTIN, HIGH);       // Red led ON
 }

 else if(TOKEN == "BLK"){                   // Blink red led
     for(int i=0;i<5;i++){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
     }
     done = true;                           // Token recognised
 }

 else if(TOKEN == "OXI"){
     
     for(int i=0;i<6;i++){                                  // Tree request for two channels (IR/Vis - in "SPO2 MODE")
        I2C_interact(FIFO_DR,0x00,false);                   // (DATA will be into I2C_COM.load) - read mode
        BLE_Payload[i]=I2C_COM.load;                        // Store the data into the payload
        }   
        
     Send_Payload();                        // Send payload via BLE or UART
     done = true;                           // Token recognised
 }

 else if(TOKEN == "OXB"){

     /*toDo*/
     /*need interrupt function and nb_pts conversion*/
     done = true;                           // Token recognised
 }

 else if(TOKEN == "TMP"){
     I2C_interact(TMP_CONFIG,TMP_ENABLE,true);
     
     I2C_interact(TMP_FRA,0x00,false);
     BLE_Payload[0]=I2C_COM.load;
     
     I2C_interact(TMP_CONFIG,0x00,true);    // Disabling
       
     Send_Payload();
     done = true;                           // Token recognised
 }

 else if(TOKEN == "MOD"){
     ComMode = !ComMode;                    // Switch Communication Mode (BLE<>UART), Careful to have both of coms operationnal to not get stuck or Add a conditional statement later
     done = true;                           // Token recognised
 }

 else if(TOKEN == "HWP"){
     HWMode = !HWMode;                      // Switch Hardware Interaction Mode (SIMULATED<>GPIOs)
     done = true;                           // Token recognised
 }

 /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
 /* In UART Mode, the value you need to send are CHAR with the DESIRED HEXA VALUE (Ex: "!" == 0x23 == 33). For SMP,PUL,AMP the CHAR == HEXA VALUE (Ex : "5" == 0x05) */
 
 else if(TOKEN == "STR"){
    OXI_SETTINGS_UART();
    done = true;                            // Token recognised
 }
 
 else if(TOKEN == "PTS"){
     Read_Payload();                                                  //After the TOKEN is received, wait for more Serial data. 
     OXI_SET.nb_pts[0] = (int)BLE_Payload[0]%(MAX_SIZE>>8);          // Byte array to define the number of point to gather in buffered mode.
     OXI_SET.nb_pts[1] = (int)BLE_Payload[1]%(MAX_SIZE&0x00FF);      // Modulo Low and High Byte of MAX_SIZE to clamp the values
     OXI_SETTINGS_UART();                                             // Display the structure if UART Enabbled to check if something changed the wrong way.
     done = true;                                                     // Token recognised
 }

 else if(TOKEN == "SMP"){
     byte register_status = 0x00;
     
     Read_Payload();
     OXI_SET.sampling_freq = B00000111&BLE_Payload[0];                                             //See datasheet (low value bytes)
     OXI_SETTINGS_UART();
     
     I2C_interact(SPO2_CONFIG,0x00,false);                                                         //Gather the actual register content
     register_status = I2C_COM.load;
     
     I2C_interact(SPO2_CONFIG,(OXI_SET.sampling_freq << 2)|register_status,true);      //Modify only the targeted part of the register without changing the rest. Mask for the UART Mode ("3" to 0x03)
     
     done = true;                                                                                  // Token recognised
 }

 else if(TOKEN == "AMP"){
    byte register_status = 0x00;
    
     Read_Payload();
     OXI_SET.range = B00000011&BLE_Payload[0];                                             //See datasheet (low value bytes)
     OXI_SETTINGS_UART();
     
     I2C_interact(SPO2_CONFIG,0x00,false);                                                 //Gather the actual register content
     register_status = I2C_COM.load;
     
     I2C_interact(SPO2_CONFIG,(OXI_SET.range << 5)|register_status,true);      //Modify only the targeted part of the register without changing the rest. Mask for the UART Mode ("3" to 0x03)

     done = true;                                                                          // Token recognised
 }

 else if(TOKEN == "PUL"){
     byte register_status = 0x00;
     
     Read_Payload();
     OXI_SET.pulse_width = B00000011&BLE_Payload[0];                                      //See datasheet (low value bytes)
     OXI_SETTINGS_UART();
     
     I2C_interact(SPO2_CONFIG,0x00,false);                                                //Gather the actual register content
     register_status = I2C_COM.load;
     
     I2C_interact(SPO2_CONFIG,OXI_SET.pulse_width|register_status,true);      //Modify only the targeted part of the register without changing the rest. Mask for the UART Mode ("3" to 0x03)

     done = true;                                                                         // Token recognised
 }

 else if(TOKEN == "INT"){
     Read_Payload();
     OXI_SET.intensity = BLE_Payload[0];                                                //See datasheet
     OXI_SETTINGS_UART();
     
     I2C_interact(LED1_AMP,OXI_SET.intensity,true);
     I2C_interact(LED2_AMP,OXI_SET.intensity,true);
     
     done = true;                                                                       // Token recognised
 }
      
 else{                                                                                  // Token not recognised
     digitalWrite(LED_BUILTIN, LOW);                                                    // Cancel error led if needed    
 }
  
  return done;  
}

/* #################################################################### */

void setup() {
  
  Serial.begin(9600);
  Serial.println("Prototype Device Initialisation");
  Serial.println(" * * Remember to change the ComMode and HWMode if needed ! * * ");
  Serial.println(" * * Remember to change the loop() content if needed ! * * ");
  Serial.println("--------------------------\n");

  /* Init I2C */
  Wire.begin();
  
  /* Init GPIO */
  pinMode(LED_BUILTIN, OUTPUT);           //LED RED
  pinMode(CHIP_INT, INPUT);               //CHIP INTERRPUT PIN
  
  /* Init BlueTooth */
    /* => To Do */
  
  /* Init Oximetry Chip*/
     I2C_interact(INT_EN_1,INT1_SETUP,true); 
     I2C_interact(INT_EN_2,INT2_SETUP,true); 
     I2C_interact(FIFO_CONFIG,FIFO_SETUP,true);
     I2C_interact(MODE_CONFIG,MODE_SETUP,true);
     I2C_interact(SPO2_CONFIG,SPO2_SETUP,true);
     I2C_interact(LED1_AMP,LED1_SETUP,true);
     I2C_interact(LED2_AMP,LED2_SETUP,true);
     I2C_interact(TMP_CONFIG,0x00,true);

  Serial.println("Prototype Device Routine");
  Serial.println("--------------------------\n");
}

/* #################################################################### */

void loop() {
  /* UART MAIN Version */
  while (Serial.available() > 0) {
    Read_Token();                 // Read Token (from UART or BLE)
    Send_Error(TokenAction());    // Call functions designatd by the Token. Raise an error if needed.
  } 
}
