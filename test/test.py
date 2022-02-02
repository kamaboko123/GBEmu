import yaml
import os
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TEST_DATA = "%s/test_data.yml" % SCRIPT_DIR
EMU = "%s/../bin/emu" % SCRIPT_DIR
ROM_FILE_DIR = SCRIPT_DIR

def exec_emu(rom_file):
    flg = False
    regs = {}
    _rom_file = "%s/%s" % (ROM_FILE_DIR, rom_file)
    result = subprocess.run(
        [EMU, _rom_file, "--regs"],
        text=True,
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL
    )
    for line in result.stdout.splitlines():
        if line == "[registers:start]":
            flg = True
            continue
        if line == "[registers:end]":
            flg = False
        if flg:
            m = line.replace(" ", "").split(":")
            regs.update({
                m[0]: int(m[1], 16)
            })
    return regs

def exec_test(test):
    ret = {
        "error": False,
        "test_rom": "",
        "regs": []
    }
    regs = exec_emu(test["rom"])
    for k, v in test["regs"].items():
        if v != regs[k]:
            ret["test_rom"] = test["rom"]
            ret["regs"].append({
                "expect": "0x%02x" % v,
                "actual" : "0x%02x" % regs[k]
            })
            ret["error"] = True
    return ret

if __name__ == '__main__':
    with open(TEST_DATA, "r") as f:
        CONF = yaml.safe_load(f)

    for test in CONF["test"]:
        result = exec_test(test)
        if result["error"]:
            print("[TEST] %s: NG" % test["name"])
            print(result)
        else:
            print("[TEST] %s: OK" % test["name"])

