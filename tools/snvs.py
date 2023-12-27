from Crypto.Cipher import AES
from Crypto.Hash import CMAC
import sys

def xor(data, tweak):
    return bytes([a ^ b for a, b in zip(data, tweak)])

def aes_xts_sector(sector, key, idx, encrypt):
    cipher = AES.new(key[:16], AES.MODE_ECB)
    tweak_cipher = AES.new(key[16:], AES.MODE_ECB)

    output_blocks = []

    tweak = tweak_cipher.encrypt(idx.to_bytes(16, byteorder='little'))
    if encrypt == True:
        output_blocks.append(xor(cipher.encrypt(xor(sector[0:16], tweak)), tweak))
    else:
        output_blocks.append(xor(cipher.decrypt(xor(sector[0:16], tweak)), tweak))

    tweak = int.from_bytes(tweak, byteorder='little')
    tweak = ((tweak << 1) & (2**128 - 1)) ^ (135 if tweak >> 127 else 0)
    tweak = tweak.to_bytes(16, byteorder='little')
    if encrypt == True:
        output_blocks.append(xor(cipher.encrypt(xor(sector[16:32], tweak)), tweak))
    else:
        output_blocks.append(xor(cipher.decrypt(xor(sector[16:32], tweak)), tweak))

    return b''.join(output_blocks)
    
    
if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("usage: " + sys.argv[0] + " [encrypt/decrypt/cmac] <key> <idx> <input> <output>")
    else:
        mode = sys.argv[1]
        key = bytes.fromhex(sys.argv[2])
        sector_idx = int(sys.argv[3], 16)
        with open(sys.argv[4], 'rb') as f:
            input = f.read()

        if mode == "cmac":
            cmac = bytearray.fromhex(CMAC.new(key[:16], ciphermod=AES).update(bytes(bytearray(input[(sector_idx * 0x20):((sector_idx * 0x20) + 0x10)]) + bytearray(0x30))).hexdigest())
            output = bytearray(input)
            for i in range(0x10):
                output[(sector_idx * 0x20) + 0x10 + i] = cmac[i]
            with open(sys.argv[5], 'wb') as f:
                f.write(output)
        else:
            output_sectors = []

            for i in range(sector_idx * 0x20, len(input), 0x20):
                output_sectors.append(aes_xts_sector(input[i:(i + 0x20)], key, sector_idx + (i // 0x10), mode == "encrypt"))

            output = b''.join(output_sectors)

            with open(sys.argv[5], 'wb') as f:
                f.write(output)
    