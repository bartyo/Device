# Prototype

This folder contains the files and studies that have been made to build a prototype design of the wearable device.

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

### The Sensors HUB => Sensor Prototype Board (SPB)
