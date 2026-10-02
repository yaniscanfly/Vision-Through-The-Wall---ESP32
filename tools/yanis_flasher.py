#!/usr/bin/env python3
"""
Yanis Vision Through The Wall - Python fallback flasher.

Erases the flash, then writes bootloader.bin, partition-table.bin and app.bin
to an ESP32 over a serial (COM) port using esptool.

Usage:
    pip install esptool pyserial
    python tools/yanis_flasher.py                  # auto-detect port
    python tools/yanis_flasher.py -p COM5          # Windows
    python tools/yanis_flasher.py -p /dev/ttyUSB0  # Linux
    python tools/yanis_flasher.py --firmware-dir build   # use a local ESP-IDF build
"""
import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DIR = ROOT / "web_installer" / "firmware"

# (file name, flash offset) - must match web_installer/manifest.json
PARTS = [
    ("bootloader.bin", 0x1000),
    ("partition-table.bin", 0xA000),
    ("app.bin", 0x20000),
]
# ESP-IDF build output names, used when --firmware-dir points at build/
IDF_NAMES = {
    "bootloader.bin": "bootloader/bootloader.bin",
    "partition-table.bin": "partition_table/partition-table.bin",
    "app.bin": "yanis_vision.bin",
}


def resolve(fw_dir: Path, name: str) -> Path:
    p = fw_dir / name
    if p.exists():
        return p
    alt = fw_dir / IDF_NAMES[name]
    return alt if alt.exists() else p


def esptool(*args):
    cmd = [sys.executable, "-m", "esptool", *args]
    print("\n>", " ".join(cmd))
    return subprocess.call(cmd)


def main():
    ap = argparse.ArgumentParser(description="Flash Yanis Vision firmware to an ESP32")
    ap.add_argument("-p", "--port", help="serial port, e.g. COM5 or /dev/ttyUSB0 (default: auto)")
    ap.add_argument("-b", "--baud", default="460800", help="baud rate (default 460800)")
    ap.add_argument("--chip", default="esp32", help="chip type (default esp32)")
    ap.add_argument("--firmware-dir", type=Path, default=DEFAULT_DIR)
    ap.add_argument("--no-erase", action="store_true", help="skip the full flash erase")
    a = ap.parse_args()

    try:
        import esptool as _  # noqa: F401
    except ImportError:
        sys.exit("esptool is not installed. Run:  pip install esptool pyserial")

    files = [(resolve(a.firmware_dir, n), off) for n, off in PARTS]
    missing = [str(f) for f, _ in files if not f.exists()]
    if missing:
        sys.exit("Missing firmware file(s):\n  " + "\n  ".join(missing))

    common = ["--chip", a.chip, "--baud", a.baud]
    if a.port:
        common += ["--port", a.port]

    if not a.no_erase:
        if esptool(*common, "erase_flash") != 0:
            sys.exit("Erase failed.")

    write = [*common, "write_flash", "-z", "--flash_mode", "dio", "--flash_freq", "40m", "--flash_size", "detect"]
    for f, off in files:
        write += [hex(off), str(f)]
    if esptool(*write) != 0:
        sys.exit("Flashing failed.")

    print("\nDone. Connect to Wi-Fi 'YanisVision-Setup', then open http://yanisvision.local after setup.")


if __name__ == "__main__":
    main()
