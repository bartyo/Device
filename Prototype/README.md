# Prototype

## Current Status
Building the Adafruit nRF52 base code / Check the SPB solders

## Summary

This folder contains the files and studies that have been made to build a prototype design of the wearable device.

+ Follow.ER Goals
+ Global Architecture
+ Device Architecture
+ Device Prototype Goals
+ Elements
  + The MCU
  + The Sensors HUB

## Follow.ER Goals

 Follow.ER is a project that wants to build an easy patients-monitoring system for the ER thanks to wearable devices following thoses rules :
 
 - Able to follow Oximetry , HeartBeat and Localisation  (+ optionnal data)
 - Easy to Use
 - Safe
 - Costless

## Global Architecture

The Follow.ER system consist of wearables devices following patients in a mesh-designed BLE network. This allow centralized server management, distant access to data trough a WebApp and patient localisation (by BLE meshing & proximity)

![GlobalArch](https://tof.cx/images/2019/10/11/c6c83df04b66c02350eef43009cafc8b.png)

## Device Architecture

The device system is a wearable sensor hub dedicated to Oximetry and Heartbeat measurement (+ optionnal Temperature or Movements). The system should be able to transmit trustful data, alarms and identifications thanks to the [BLE protocol]() in a secure way (ex: crypto).

![DeviceArch](https://tof.cx/images/2019/10/11/bb2a043a4df29cc7149559e702aebc85.png)

## Device Prototype Goals

Before strating the "industry compatible" final designs, we need to be able to build a Proof of Concept to ensure that all the things we want to deploy is possible to realise. And so :

 - Being able to gather oximetry and pulse form transmissive or reflective measurement
 - Being able to tramsit data via BLE in a secure way 
 - Reach a current consumption level compatible with a wearable device

## Elements

The Device Prototype System (DPS) is make of two elements :

 - A MCU able to transmit trought BLE
 - A sensors HUB board able to return the targeted data and more if needed. 
 
 This system could be displayed as :
 
 ![DPSystem](https://tof.cx/images/2019/10/11/0f6a248f9ba60e40c8a29580ea7357eb.png)

### The MCU => Feather nRF52 BlueFruit LE (Adafruit BLE)

The Feather nRF52 Bluefruit LE , from Adafruit Industires have been choosen because it allow to deploy an embedded BLE solution in a simple way (Arduino IDE compatible + breadboard pinout).

#### Board PinOut

The board PinOut can be found [HERE](https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/device-pinout)

#### Protoype Code Architecure

### The Sensors HUB => Sensor Prototype Board (SPB)

To allow us to prototype with oximetry measurement, **we've built a Sensors Prototype Board** that gather **oximetry signal** by **reflectance method**. The chip is also able to send **die temperature measurements**.

In addition, we choose to put **two current measurement channels** so we should be able to **plot consumption curves** during the system study and reach better programming solution if needed.

Of course, the eletronical environnement is designed for a bug free use (I2C pull-up , decoupling resistors, voltage regulation, etc..)

The board B.O.M and the GERBER files could be found [HERE](https://github.com/ProjSante/Device/tree/master/Prototype/KiCAD-Files-SensorBoard)

The simplified board architecture could be displayed as :

![SPB_Arch](https://tof.cx/images/2019/10/11/435c2b7e5b95a3bc33b57376aa90a550.png)

#### SPB PinOut

![SPB_Pinout](https://tof.cx/images/2019/10/11/cf41bafe9fde8d505ec1f96dae9f8209.png)

> /!\ The input voltage should be 3.3v if you just want to use the Red and the IR LED; 5v if you want to use the Red, the IR and the Green LED

> Current measurement : Current_Analog or Current_Digital (in Volts) / 2000 =** ***Real Current*** (ex: 1V == 0.5mA)

To avoid ESB hazards, a protection could be put over the board thanks to the 4 fixation holes :

![Shield](https://tof.cx/images/2019/10/11/5aa6f919fd9bc5461db9d073169f5693.png)

#### I2C methods

See the [MAX30101 Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX30101.pdf)

|**Function**|Adress|Effects|Main Direction|
|:------:|:----:|:-----:|:-----:|
| Interrupt enable 1 | 0x02 | Use the bit 7 to 5 to set the interrupt conditions about dataflow | Write |
| Interrupt enable 2 | 0x03 | Use the bit 1 to set the interrupt condition about die temperature| Write |
| Interrupt enable 2 | 0x03 | Use the bit 1 to set the interrupt condition about die temperature| Write |
| FIFO Write Pointer | 0x04 | Last data adress location , bytes 4 to 0 | Read |
| OF Counter | 0x05 | Overflow counter (if FIFO Full) , bytes 4 to 0 | Read |
| FIFO Read Pointer | 0x06 | I2C read adress location , bytes 4 to 0 | Read & Write |
| FIFO Data Register | 0x07 | Pointed data register (not always auto-incremented), 3x1 byte for one channel| Read |
| FIFO Config| 0x08 | Averaging [7:5] , FIFO RollOver Enable [4] , FIFO status when almost full [3:0] | Read & Write |
| Mode Config| 0x09 | Shutdown (register keeped and Int==0) [7] , Reset [6] , Measurement Mode [2:0] | Write |
| SpO2 Config| 0x0A | ADC Range [6:5] , Sample Rate [4:2] , Pulse Width [1:0] | Write |
| LED 1 Amplitude | 0x0C | 1 full byte (see datasheet) | Write |
| LED 2 Amplitude | 0x0D | 1 full byte (see datasheet) | Write |
| Temperature (Int) | 0x1F | 1 full byte (ex : the 12 in 12.5) | Read |
| Temperature (Fractional) | 0x20 | bit 4 to 0 (ex: the 0.5 in 12.5) | Read |
| Temperature Config | 0x21 | Use the bit 0 to enable die temperature measurment| Read |
