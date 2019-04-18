ReadMe.txt for the ST STM32F407 demo project.

This project was porting from STM32F746 board to STM32F407VET6-Demo board.

You may download this project as well from our site:

https://kkmspb.ru/development/STM-microcontrollers/USB-development/SEGGER-Embedded-Studio-USB-Host-example.php


This project was built for Segger Embedded Studio V4.12a.
It has been tested with the following versions:
- V4.12

Supported hardware:
===================
The sample project for ST STM32F407vet6 is prepared
to run on a ST STM32F407VET6-Demo, but may be used on other
target hardware as well.

Using different target hardware may require modifications.

Configurations:
===============
- Debug
This configuration is prepared for download into
internal Flash using J-Link.
An embOS debug and profiling library is used.

- Release
This configuration is prepared for download into
internal Flash using J-Link.
An embOS release library is used.

Prerequisites:
==============
1.  Embedded Studio https://www.segger.com/es
2.a J-Link or J-Link OB https://www.segger.com/products/debug-probes/j-link/
2.b Alternatively an on-board ST-Link can be reflashed into a J-Link https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/

How to run the demo application:
====================================
1. Using Embedded Studio program the default application into the board.
2. When using the "HID_Keyboard" configuration connect a keyboard or mouse, when using the "HID_Barcode" configuration connect a barcode scanner.
3. For a connected keyboard the application will display any keystrokes, for a mouse - the mouse movement, for a barcode scanner any scanned codes.
