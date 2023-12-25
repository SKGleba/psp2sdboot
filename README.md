# psp2sdboot
This repo contains guides, payloads, schematics and scripts for use with PSP2's "SD boot mode".<br>
<br>
## About
#### JIG
On all Playstation Vita/TV units the System Controller ("syscon", "ernie") has an RPC server activated by a specific hardware circuit. When activated, syscon listens over UART for commands that can range from simple diag checks, fuse reads, calibrations, to special features such as NVS R/W that require authorization using handshakes with secret keys. More information as well as circuit schematics and RPC clients can be found in the [bert](https://github.com/SKGleba/bert) repository.
#### SD boot
An authorized JIG interface client can trigger a special, alternative boot mode inside the BootROM. In this mode, which we dubbed "sdboot", a signed second_loader.enp is loaded from an unauthenticated MMC/SD storage inside the GameCard slot and decrypted using an alternative set of per-console keys.<br>
#### Vulnerability
In sdboot, after copying itself from 0x40000 to 0x5c000, BootROM first reads a single block (0x200 bytes) from the external storage to 0x40000, verifies if second_loader offset and size fields are valid, then reads second_loader to 0x40000.<br>
The issue is that the exception vectors are hardcoded to 0x40000, so any exceptions in BootROM during (or after) second_loader is read will cause a "jump" to the read data - resulting in arbitrary code execution.
#### Code execution
A simple way to trigger an exception is to perform a Fault Injection attack. Because all faults/exceptions result in code execution, the FI setup does not need to be precise, and can be approached in a spray-and-pray manner.<br>
This project makes use of Voltage Fault Injection, specifically the "crowbar" method, performed by a low cost setup with the Teensy 4 mcu system at heart.<br>
The result is a consistent BootROM code execution of a arbitrary code blob loaded from an unauthenticated MMC/SD storage inside the GameCard slot.
#### BootROM RPC
With psp2sdboot, the default code blob is [bob](https://github.com/SKGleba/bob), it starts an RPC server that can be used for research or executing additional specialized payloads with functionality such as key retrieval, data dumping, unbricking and more.
#### Supported units
 - DEM-3000 : **only IRT-002**
   - Due to an unknown JIG interface-enable procedure on IRT-001 / "slideys"
 - CEM-3000 : **supported**
 - PCH-1000 : **supported**
 - VTE-1000 : **supported**
 - PCH-2000 : **not yet supported**
   - Due to lack of success triggering exceptions with Voltage Fault Injection
<br><br>
## Setup
### Requirements
 - working [syscon JIG setup](https://github.com/SKGleba/bert)
 - logic analyzer with at least two channels, a simple USB 8ch saleae/clone should work
 - GC-SD adapter such as ["sd2vita"](https://www.bing.com/search?q=sd2vita), with a micro/sd up to 2TiB
 - consistent, configurable pulse generator that can set 1v+ for 100ns or less.
    - for this project, the [Teensy 4.0](https://www.pjrc.com/store/teensy40.html) / [Teensy 4.1](https://www.pjrc.com/store/teensy41.html) mcu board was chosen for its high speed, tight-coupled memory and gpio controllers
    - a [chipwhisperer](https://www.newae.com/chipwhisperer) (lite) was also successfully used in this project's early iterations
 - fast switching, logic level n-mosfet
    - depending on your soldering skills/setup, a breakout board might be a good idea
    - for this project, [IRLML2502](https://www.infineon.com/cms/en/product/power/mosfet/n-channel/irlml2502/) and [IRLML6246](https://www.infineon.com/cms/en/product/power/mosfet/n-channel/irlml6246/) were used
 - 3.3v usb<->uart adapter for communicating with the Teensy 4 mcu board
### Teensy
 - Connect the Teensy 4 to a PC, enter bootloader mode
 - Flash the latest build of [teensy4vfi](https://github.com/SKGleba/teensy4vfi) using [Teensy Loader](https://www.pjrc.com/teensy/loader_win10.html)
 - Connect the usb<->uart adapter
    - PC TX -> Teensy pad 0 (RX)
    - PC RX -> Teensy pad 1 (TX)
    - GND
### Solder points
TODO
### Wiring
TODO
### Software
TODO
<br><br>
## SD Boot
### The Script
The default sdboot script, *sdboot.py*, is a python script that loops reboot->glitch->check until *bob* is successfully loaded and executed. <br>
Its accepted arguments/parameters and their descriptions can be listed with ```sdboot help```.
### Calibration
The first step is determining optimal timing parameters range for the main sdboot script, which will then find the more precise/consistent glitch timings.
#### The "treshold" width
The *width* of a glitch that always causes an unrecoverable crash is called the "treshold", this should be the upper *width* boundary - sdboot's *width_max* argument.<br>
It can be determined by the following procedure:
1. ```sdboot width=100 width_max=100000 width_step=100```
   - note the *width* at which *dat0* cut off
   - if there is no *dat0* cut off until 100000, the setup is wrong/faulty
2. ```sdboot width=<cutoff_width-100> width_max=<cutoff_width+100> width_step=20```
   - repeat at least 5 times
   - note the *width* at which *dat0* __always__ cuts off, this is the "treshold"
3. If the "treshold" is below 200, it might indicate a faulty circuit, slow mosfet, or a very isolated/clean setup.
#### The "*up_to_read*" offset
*up_to_read* is a special 0-width glitch queued to find an empty sector read op, which compensates for a high SD init/read jitter.<br>
The idea is that as long as it lands in the middle of an empty sector being read (*dat0* pulled down), the actual fault injection glitch can be precisely triggered and timed from *dat0* going up.<br>
It can be determined by the altering following command: ```sdboot up_to_read_mark=100 up_to_read=298400000```. Change *up_to_read* until the *mosfet* line spike happens around the middle of an empty sector read.
### The *offset* and *width* pair
The second step is running the sdboot python script, and hoping it finds a correct combination of *offset* and *width* parameters <br>
Script arguments are based on the values found during the Calibration step, with an added broad *offset* range:<br>
```sdboot up_to_read=<up_to_read> width=<treshold-(2*width_step)> width_max=<treshold+width_step> width_step=20 offset=100 offset_max=10000 offset_mult=10 offset_step=40```
 - *offset** parameters should initially be broad, with further loops being more precise (eg *offset_mult=1*)
 - if the treshold is below 500, *width_step*=10 might be a better choice
 - if the starting *width* (treshold - (2 x *width_step*)) is observed to cut off dat0, the starting *width* should be decreased by *width_step*-sized decrements
 - ultimately it depends on luck, it might take hours or even days to find a working *offset* and *width* pair
 - in case of success, a ```got sd boot: off=<offset>[<offset_mult>] width=<width>``` message will be displayed and the script will stop
<br><br>
## Post-Exploitation
TODO
<br><br>
## Credits
TODO
