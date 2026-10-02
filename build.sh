#!/usr/bin/env bash
# Local build -> web_installer/firmware/*.bin (requires ESP-IDF 5.5 environment: `. $IDF_PATH/export.sh`)
set -euo pipefail
idf.py set-target esp32
idf.py build
cp build/bootloader/bootloader.bin web_installer/firmware/bootloader.bin
cp build/partition_table/partition-table.bin web_installer/firmware/partition-table.bin
cp build/yanis_vision.bin web_installer/firmware/app.bin
echo "Binaries copied to web_installer/firmware/"
