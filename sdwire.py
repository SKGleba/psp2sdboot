#!/usr/bin/env python3

#
# sdwire.py - SDWire control script
#
# Copyright (c) 2024 skgleba
#
# This software may be modified and distributed under the terms
# of the MIT license.  See the LICENSE file for details.
#
# REQUIRES pyftdi, pyusb, libusb and/or WinUSB (Windows)
#

import usb.core
import usb.util
from pyftdi.ftdi import Ftdi
from pyftdi.eeprom import FtdiEeprom
import sys
import argparse

DEFAULT_PRODUCT = "sd-wire"

def list_usb_devices(): # list all USB devices
    devices = usb.core.find(find_all=True)
    if not devices:
        print ("No USB devices found")
        return []
    
    device_list = []
    for device in devices:
        try:
            product = usb.util.get_string(device, device.iProduct)
        except Exception as e:
            product = "Unknown"

        try:
            serial = usb.util.get_string(device, device.iSerialNumber)
        except Exception as e:
            serial = "Unknown"

        device_info = f"{device.bus:03d}:{device.address:03d} | {device.idVendor:04x}:{device.idProduct:04x} | {product} | {serial}"
        device_list.append(device_info)
    
    return device_list

def find_usb_device(product = None, serial = None, device = None, busaddr = None):
    if device or busaddr:
        search_args = {'find_all': True}
        if device:
            vid, pid = device.split(":")
            search_args['idVendor'] = int(vid, 16)
            search_args['idProduct'] = int(pid, 16)
        if busaddr:
            bus, addr = busaddr.split(":")
            search_args['bus'] = int(bus)
            search_args['address'] = int(addr)
        devices = usb.core.find(**search_args)
    else:
        devices = usb.core.find(find_all=True)
    if not devices:
        print ("No USB devices found")
        return None
    
    for device in devices:
        try:
            dev_product = usb.util.get_string(device, device.iProduct)
        except Exception as e:
            dev_product = "Unknown"

        try:
            dev_serial = usb.util.get_string(device, device.iSerialNumber)
        except Exception as e:
            dev_serial = "Unknown"

        if product and product != dev_product:
            continue
        if serial and serial != dev_serial:
            continue
        return device
    
    return None

def set_sdwire(target, product = None, serial = None, device = None, busaddr = None):
    if not product and not serial and not device and not busaddr:
        print("looking for product string: {}".format(DEFAULT_PRODUCT))
        product = DEFAULT_PRODUCT
    sdwire_dev = find_usb_device(product=product, serial=serial, device=device, busaddr=busaddr)
    if not sdwire_dev:
        print("SDWire device not found")
        return
    try:
        print(f"SDWire device found: {sdwire_dev.bus:03d}:{sdwire_dev.address:03d} | {sdwire_dev.idVendor:04x}:{sdwire_dev.idProduct:04x} | {usb.util.get_string(sdwire_dev, sdwire_dev.iProduct)} | {usb.util.get_string(sdwire_dev, sdwire_dev.iSerialNumber)}")
    except Exception as e:
        print(f"SDWire device found: {sdwire_dev.bus:03d}:{sdwire_dev.address:03d} | {sdwire_dev.idVendor:04x}:{sdwire_dev.idProduct:04x}")

    try:
        ftdi = Ftdi()
        ftdi.open_from_device(sdwire_dev)
    except Exception as e:
        print(f"Error opening FTDI device: {e}")
        return

    print(f"Set CBUS to 0x{0xF0 | target:02X}")
    try:
        ftdi.set_bitmode(0xF0 | target, Ftdi.BitMode.CBUS)
    except Exception as e:
        print(f"Error setting CBUS: {e}")
    
    ftdi.close()

def handle_cmd(args):
    if args.command == "list":
        usb_devices = list_usb_devices()
        print("busaddr |  device   | product | serial")
        for device in usb_devices:
            print(device)
    elif args.command == "attach":
        set_sdwire(0x01, product=args.product, serial=args.serial, device=args.device, busaddr=args.busaddr)
    elif args.command == "detach":
        set_sdwire(0x00, product=args.product, serial=args.serial, device=args.device, busaddr=args.busaddr)
    else:
        print(f"Unknown command: {args.command}")

def main():
    example_usage = """Example usage:
    python sdwire.py list
    python sdwire.py attach
    python sdwire.py attach --serial bdgrd_sdwirec_521
    python sdwire.py detach --device 04e8:6001

If no product, serial, device or busaddr is specified, the default product string "sd-wire" is used.
    """
    parser = argparse.ArgumentParser(description="SDWire control script", epilog=example_usage, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["list", "attach", "detach"], help="List all USB devices or attach/detach SDWire")
    parser.add_argument("--product", type=str, help="Match the product string of the device")
    parser.add_argument("--serial", type=str, help="Match the serial number of the device")
    parser.add_argument("--device", type=str, help="Find by device's VendorID:ProductID")
    parser.add_argument("--busaddr", type=str, help="Find by device's bus:address")
    args = parser.parse_args()
    handle_cmd(args)

if __name__ == "__main__":
    main()