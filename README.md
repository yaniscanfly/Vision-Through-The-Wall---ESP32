# Yanis Vision Through The Wall

Wi-Fi motion sensing with an ESP32 using **Channel State Information (CSI)**. No camera, no PIR sensor.

## 👉 Install Yanis Vision on your ESP32

No ESP-IDF, VS Code, Python, or compilation required. Connect your ESP32 with USB and install from the browser using `web_installer/index.html`
(Chrome or Edge on desktop; the page must be served over HTTPS, e.g. GitHub Pages, or from `localhost`).

Local test: `cd web_installer && python3 -m http.server 8000`, then open http://localhost:8000

## After installation

1. The ESP32 creates a Wi-Fi network called **`YanisVision-Setup`**.
2. Connect with your phone or computer.
3. The setup portal opens automatically (captive portal), or browse to `http://192.168.4.1`.
4. Enter your Wi-Fi credentials.
5. Open the live dashboard at **http://yanisvision.local**

The dashboard ("Yanis Vision CSI Radar") shows **NO MOTION DETECTED** (cyan) or **MOTION DETECTED - WALL PENETRATION ACTIVE** (flashing magenta) plus a live CSI variance graph. **RESET WI-FI** returns the device to `YanisVision-Setup` mode.

## Building the firmware binaries (required before the installer works)

The installer needs `web_installer/firmware/app.bin` built from this source (the bootloader and partition table are included).
Pick one:

- **GitHub Actions**: push the repo, run the *Build Yanis Vision firmware* workflow; it commits the binaries into `web_installer/firmware/`.
- **Locally** (ESP-IDF 5.5 environment active): `./build.sh`

## Python fallback flasher

```bash
pip install esptool pyserial
python tools/yanis_flasher.py -p COM5      # erase + write bootloader, partition table, app
```

## Layout

```
main/                 firmware (app_main.c, yanis_wifi.c, wifi_web_server.c, ...)
main/web/             captive portal, saved page, dashboard, shared PRX theme (embedded in the firmware)
components/dns_server captive-portal DNS
web_installer/        index.html + manifest.json + firmware/*.bin
tools/                yanis_flasher.py, web_serial_monitor.html
```

## Origin & license

Based on the open-source **DrWalls** project by Dr. Maker, itself built on Espressif's ESP-CSI `wifi_sensing_demo`.
Apache License 2.0; original copyright and SPDX notices are retained (see `LICENSE`).
