
#include <string.h>
#include <Arduino.h>
#include <bluefruit.h>
#include <Wire.h>

/* #################################################################### */

/*PINOUT*/
#define CHIP_INT             27      //Chip Interrpution

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
#define TEMP_ENABLE               B00000001     //Enable Temperature measurement
/*etc...*/

String Device_Name = "ProtoNode";
String Manu_Name = "AdafruitInc";
String Model_Name = "ProtoAda01";

boolean ComMode = false;              // Token reading protocol (false == UART , true == BLE)
boolean HWMode = false;               // Hardware Interaction protocol (false == Simulated , true == GPIOs)
String TOKEN = "NOP";                 // Token buffer
byte BLE_Payload[20] = { 0 };         // BLE payload to send

struct I2C_PACK {
  byte adress;        //I2C adress
  byte load;          //I2C buffer to send/read data
  boolean w;          //write ?
};

struct OXI_SETTING {
  int nb_pts;
  int sampling_freq;
  int range;
  int pulse_width;
  int intensity;
};

I2C_PACK      I2C_COM = {0x00,0x00,false};                // Global I2C structure
OXI_SETTING   OXI_SET = {600,50,4,118,1};                  // Global Oximetry Settings structure

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
//  PTS -> Set number of point (buffered)
//  SMP -> Set sampling freq.
//  AMP -> Set chip range
//  PUL -> Set chip pulse
//  INT -> Set current conso
//  ETC..

/* #################################################################### */

void Read_Token(void){
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
  if(ComMode==true){
    /* read trough BLE*/
  }
  else{
    /* read trough UART */
    while (Serial.available() == 0) {}                            // Wait until data comes
    (Serial.readString()).toCharArray((char*)BLE_Payload,20);     // Read serial datas as a string, then transform it as a char array, then put it into the Byte Payload
    Serial.flush();                                               // Clean the COM
  }
}
/* ------------------------------------------------------------------- */
void I2C_interact(void){
  if(HWMode==true){
    /* use GPIO*/
  }
  else{
    String temp;
    /* (simply) simulated */
    if(I2C_COM.w == true){
      Serial.printf("@%x Send: %x\n",(char)I2C_COM.adress,(char)I2C_COM.load);      //Displays what happened on the monitor
    }
    else{
      I2C_COM.load = byte(rand());                                                  // Simulate the I2C register reading
      Serial.printf("@%x Received: %x\n",I2C_COM.adress,I2C_COM.load);              //Displays what happened on the monitor
    }
  }
}
/* ------------------------------------------------------------------- */
void OXI_SETTINGS_UART(void){
  /* Displays OXI_SET if UART Mode is ON (ComMode = false) */
  if(ComMode==false){Serial.printf("nb_pts : %d\nSampling frequency : %d\nRange : %d\nPulse Width : %d\nIntensity : %d\n",OXI_SET.nb_pts,OXI_SET.sampling_freq,OXI_SET.range,OXI_SET.pulse_width,OXI_SET.intensity);}
}
/* ------------------------------------------------------------------- */
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
boolean TokenAction(void){
 boolean done = false;                      // Unset the "DONE" flag to generate error in some statement is passed over.
 BLE_Payload[20] = { 0 };                   // Reset the payload to avoid surprise
 
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
     I2C_COM.adress = MODE_CONFIG;          // Configure the I2C structure to store the MODE CONFIG adress, the load to send and set it to Write Mode
     I2C_COM.load = MODE_SETUP;             // Exit shutdown mode while keeping SPO2 Mode
     I2C_COM.w = true;
     I2C_interact();                        // Send the command via I2C
     
     I2C_COM.adress = FIFO_DR;              // Configure the I2C structure to store the DATA REGISTER adress and set it to Read Mode (DATA will be into I2C_COM.load)
     I2C_COM.w = false;
     for(int i=0;i<6;i++){                  // Tree request for two channels (IR/Vis - in "SPO2 MODE")
        I2C_interact();                     // Send the read request via I2C
        BLE_Payload[i]=I2C_COM.load;        // Store the data into the payload
        }

     I2C_COM.adress = MODE_CONFIG;          
     I2C_COM.load = SHUTDOWN|MODE_SETUP;    // Back to Shutdown mode while keeping SPO2 Mode
     I2C_COM.w = true;
     I2C_interact();                        // Send the command via I2C
        
     Send_Payload();                        // Send payload via BLE or UART
     done = true;                           // Token recognised
 }

 else if(TOKEN == "OXB"){
     /* to do */
     done = true;                           // Token recognised
 }

 else if(TOKEN == "TMP"){
     /* to do */
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

 else if(TOKEN == "PTS"){
     Read_Payload();                        //After the TOKEN is received, wait for more Serial data. The received Serial data will be the value to attribute (so ASCII Values needed)
     OXI_SET.nb_pts = (int)BLE_Payload;     // Byte array to Integer, then attributed to corresponding OXI_SET parameter /!\ SEE GITHUB ISSUE /!\
     OXI_SETTINGS_UART();                   // Display the structure if UART Enabbled to check if something changed the wrong way.
     done = true;                           // Token recognised
 }

 else if(TOKEN == "SMP"){
     Read_Payload();
     OXI_SET.sampling_freq = (int)BLE_Payload;
     OXI_SETTINGS_UART();
     /* need to transform the value into i2c command */
     done = true;                           // Token recognised
 }

 else if(TOKEN == "AMP"){
     Read_Payload();
     OXI_SET.range = (int)BLE_Payload;
     OXI_SETTINGS_UART();
     /* need to transform the value into i2c command */
     done = true;                           // Token recognised
 }

 else if(TOKEN == "PUL"){
     Read_Payload();
     OXI_SET.pulse_width = (int)BLE_Payload;
     OXI_SETTINGS_UART();
     /* need to transform the value into i2c command */
     done = true;                           // Token recognised
 }

 else if(TOKEN == "INT"){
     Read_Payload();
     OXI_SET.intensity = (int)BLE_Payload;
     OXI_SETTINGS_UART();
     /* need to transform the value into i2c command */
     done = true;                           // Token recognised
 }
      
 else{                                 // Token not recognised
     digitalWrite(LED_BUILTIN, LOW);        // Cancel error led if needed    
 }
  
  return done;  
}

/* #################################################################### */

void setup() {
  
  Serial.begin(9600);
  Serial.println("Prototype Device Initialisation");
  Serial.println(" * * Remember to change the ComMode and HWMode if needed ! * * ");
  Serial.println("--------------------------\n");

  /* Init I2C */
  
  /* Init GPIO */
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CHIP_INT, INPUT);
  
  /* Init BlueTooth */
  
  /* Init Oximetry Chip*/
     I2C_COM.w = true;   
     I2C_COM.adress = INT_EN_1;
     I2C_COM.load = INT1_SETUP;
     I2C_interact();
     I2C_COM.adress = INT_EN_2;
     I2C_COM.load = INT2_SETUP;
     I2C_interact();
     I2C_COM.adress = FIFO_CONFIG;
     I2C_COM.load = FIFO_SETUP;
     I2C_interact();
     I2C_COM.adress = FIFO_CONFIG;
     I2C_COM.load = FIFO_SETUP;
     I2C_interact();
     I2C_COM.adress = MODE_CONFIG;
     I2C_COM.load = SHUTDOWN|MODE_SETUP;
     I2C_interact();
     I2C_COM.adress = SPO2_CONFIG;
     I2C_COM.load = SPO2_SETUP;
     I2C_interact();
     I2C_COM.adress = LED1_AMP;
     I2C_COM.load = LED1_SETUP;
     I2C_interact();
     I2C_COM.adress = LED2_AMP;
     I2C_COM.load = LED1_SETUP;
     I2C_interact();
     I2C_COM.adress = TMP_CONFIG;
     I2C_COM.load = 0x00;
     I2C_interact();

  Serial.println("Prototype Device Routine");
  Serial.println("--------------------------\n");
}

/* #################################################################### */

void loop() {

  /* UART MAIN Version */
  while (Serial.available() > 0) {
    Read_Token();                // Read Token (from UART or BLE)
    Send_Error(TokenAction());  // Call functions designatd by the Token. Raise an error if needed.
  }
  
}
