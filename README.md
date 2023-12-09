# psp2sdboot
This repo contains guides, payloads, schematics and scripts for use with PSP2's "SD boot mode".
<br><br>
## About
### JIG
On all Playstation Vita/TV units the System Controller ("syscon", "ernie") has an RPC server activated by a specific hardware circuit. When activated, syscon listens over UART for commands that can range from simple diag checks, fuse reads, calibrations, to special features such as NVS R/W that require authorization using handshakes with secret keys. More information as well as circuit schematics and RPC clients can be found in the [bert](https://github.com/SKGleba/bert) repository.
### SD boot
An authorized JIG interface client can trigger a special, alternative boot mode inside the BootROM. In this mode, which we dubbed "sdboot", a signed second_loader.enp is loaded from an unauthenticated MMC/SD storage inside the GameCard slot and decrypted using an alternative set of per-console keys.<br>
### Vulnerability
In sdboot, after copying itself from 0x40000 to 0x5c000, BootROM first reads a single block (0x200 bytes) from the external storage to 0x40000, verifies if second_loader offset and size fields are valid, then reads second_loader to 0x40000.<br>
The issue is that the exception vectors are hardcoded to 0x40000, so any exceptions in BootROM during (or after) second_loader is read will cause a "jump" to the read data - resulting in arbitrary code execution.
### Code execution
A simple way to trigger an exception is to perform a Fault Injection attack. Because all faults/exceptions result in code execution, the FI setup does not need to be precise, and can be approached in a spray-and-pray manner.<br>
This project makes use of Voltage Fault Injection, specifically the "crowbar" method, performed by a low cost setup with the Teensy 4 mcu system at heart.<br>
The result is a consistent BootROM code execution of a arbitrary code blob loaded from an unauthenticated MMC/SD storage inside the GameCard slot.
### BootROM RPC
With psp2sdboot, the default code blob is [bob](https://github.com/SKGleba/bob), it starts an RPC server that can be used for research or executing additional specialized payloads with functionality such as key retrieval, data dumping, unbricking and more.<br>
<br><br>
## Requirements
TODO
<br><br>
## Setup
TODO
<br><br>
## Calibration
TODO
<br><br>
## SD Boot
TODO
<br><br>
## Post-Exploitation
TODO
<br><br>
## Credits
TODO
