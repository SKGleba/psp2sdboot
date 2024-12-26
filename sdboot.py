import sys
import time
import re
import bert
import teensy_rpc

## my pstv:
# stage1: 158000000 / 900 / 130
# small: 341000000 / 100000 / 130
# big: 385000000 / 1100 / 130
# max (32kb): 498000000 / 160 / 130
# some logic: (read+ib) * ((first_read / (read+ib)) + 5 + (payload_size / 512)) ~= up_to_read=5215000 up_to_read_mult=(34 + (payload_size / 512))
# 470/125
# 300,2700/130
DEFAULT_VARS_DICT = {
    "mosfet" : [22, "teensy pad to which the mosfet gate is connected"],
    "dat0" : [23, "teensy pad to which the dat0 probe is connected"],
    "up_to_read" : [158000000, "delay between dat0 going up and the ~middle of an empty, post-payload sector read"],
    "up_to_read_mult" : [1, "multiplier for the [up_to_read] delay"],
    "up_to_read_mark" : [0, "(debug) set this if you want to insert a glitch after the up_to_read delay"],
    "offset" : [200, "delay between end of sector read and glitch insertion, starting value for the [width->width_max] loop"],
    "offset_mult" : [1, "multiplier for the [offset] delay"],
    "width" : [130, "how long the glitch/mosfet is held for, starting value for the glitch loop"],
    "state" : [1, "dat0 up pad logic level, set this to 0 if dat0 is inverted"],
    "width_max" : [130, "max [width] value for the glitch loop"],
    "width_step" : [5, "[width] increment size between each attempt in the glitch loop"],
    "delay_next" : [1, "delay (in seconds) between each attempt in the glitch loop"],
    "delay_boot" : [1, "delay (in seconds) between console shutdown and glitch attempt"],
    "delay_check" : [3, "delay (in seconds) between sdboot/glitch attempt and success confirmation checks"],
    "offset_max" : [200, "max [offset] value for the [width->width_max] loop"],
    "offset_step" : [2400, "[offset] increment size after each [width->width_max] loop"],
    "loops" : [0, "glitch loop count, 0 - infinite"]
}

VAR_ALIASES_DICT = {
    "u" : "up_to_read",
    "ux" : "up_to_read_mult",
    "o" : "offset",
    "ox" : "offset_mult",
    "om" : "offset_max",
    "os" : "offset_step",
    "w" : "width",
    "wm" : "width_max",
    "ws" : "width_step",
    "dn" : "delay_next",
    "db" : "delay_boot",
    "dc" : "delay_check",
    "l" : "loops"
}

def prep_glitch(argd, offset, width):
    uptoread = {param: value[0] for param, value in teensy_rpc.DEFAULT_ARG_DICT.copy().items()}
    uptoread["offset"] = argd["up_to_read"]
    uptoread["offset_mult"] = argd["up_to_read_mult"]
    if argd["up_to_read_mark"] != 0:
        uptoread["width"] = argd["up_to_read_mark"]
    else:
        uptoread["no_driver"] = 1
    uptoread["trigger"] = argd["dat0"]
    uptoread["trigger_state"] = argd["state"]
    uptoread["driver"] = argd["mosfet"]

    glitch = {param: value[0] for param, value in teensy_rpc.DEFAULT_ARG_DICT.copy().items()}
    glitch["offset"] = offset
    glitch["offset_mult"] = argd["offset_mult"]
    glitch["width"] = width
    glitch["trigger"] = argd["dat0"]
    glitch["trigger_state"] = argd["state"]
    glitch["driver"] = argd["mosfet"]
    glitch["queue"] = 1

    if teensy_rpc.glitch_add_dfl(uptoread, max_wait=5) < 0:
        print("Failed to communicate with teensy, reset?")
        return False
    teensy_rpc.glitch_add_dfl(glitch)
    teensy_rpc.send_rpc_cmd("glitch_arm", [1])
    return True


def try_sdboot(argd, offset, width):
    print("off")
    bert.handle_cmd("power-off", ["","",0])
    print("delay_boot")
    time.sleep(argd["delay_boot"])
    print("try sdboot")
    bert.handle_cmd("shbuf-write", ["","","00000000"])
    if prep_glitch(argd, offset, width) == False:
        print("E: failed to prep glitch")
        return -1
    bert.handle_cmd("unlock-sdboot", ["","",0])
    print("delay_check")
    time.sleep(argd["delay_check"])
    print("check result")
    bert.client.send_cmd(bytearray.fromhex("0103000000"), 0)
    if bert.client.get_resp().hex().upper()[10:18] == "BEBAFECA":
        return 1
    return 0


def glitch_loop(argd):
    if teensy_rpc.send_rpc_cmd("set_clk", [600000000], max_wait=5) < 0:
        print("Failed to communicate with teensy, reset?")
        return False
    loopc = 0
    while argd["loops"] == 0 or loopc < argd["loops"]:
        for offset in range(argd["offset"], argd["offset_max"] + 1, argd["offset_step"]):
            for width in range(argd["width"], argd["width_max"] + 1, argd["width_step"]):
                print("try off={} width={}".format(offset, width))
                ret = try_sdboot(argd, offset, width)
                if ret == 1:
                    print("--------------------------------------------")
                    print("got sd boot: off={}[x{}] width={}".format(offset, argd["offset_mult"], width))
                    print("--------------------------------------------")
                    return True
                elif ret == -1:
                    return False
                print("delay_next")
                time.sleep(argd["delay_next"])
        loopc += 1
    return False

if __name__ == "__main__":
    if len(sys.argv) == 2 and sys.argv[1] == "help":
        print("\nUsage: " + sys.argv[0] + " param=value par6+val6 par3-val3 ...\n")
        print("Descr: " + "insert glitches during sdboot in goal of executing the flashed payload" + "\n")
        print(f"{'PARAM':>16}" + " : " + f"{'DEFAULT':^11}" + " : " + "DESCRIPTION")
        print(f"{'-----':>16}" + " : " + f"{'-------':^11}" + " : " + "-----------")
        for arg in DEFAULT_VARS_DICT:
            print(f"{arg:>16}" + " : " + f"{str(DEFAULT_VARS_DICT[arg][0]):^11}" + " : " + DEFAULT_VARS_DICT[arg][1])
    else:
        arg_dict = {param: value[0] for param, value in DEFAULT_VARS_DICT.copy().items()}
        for arg in sys.argv[1:]:
            key, val = re.split(r'[=\+\-\/\*]', arg, maxsplit=1)
            if key in VAR_ALIASES_DICT:
                    key = VAR_ALIASES_DICT[key]
            if val.startswith('0x'):
                val = int(val, 16)
            elif '.' in val:
                val = float(val)
            else:
                val = int(val)
            if '=' in arg:
                arg_dict[key] = val
            elif '+' in arg:
                arg_dict[key] += val
            elif '-' in arg:
                arg_dict[key] -= val
            elif '*' in arg:
                arg_dict[key] *= val
            elif '/' in arg:
                arg_dict[key] /= val
        glitch_loop(arg_dict)