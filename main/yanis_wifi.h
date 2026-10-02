#pragma once

#include "esp_err.h"

/*
 * Starts Yanis Vision Wi-Fi.
 *
 * If credentials are already saved in NVS:
 *   -> Connect to Wi-Fi.
 *
 * If no credentials are saved:
 *   -> Start the YanisVision-Setup configuration portal.
 */
esp_err_t yanis_wifi_start(void);

/*
 * Erases the Wi-Fi credentials saved by Yanis Vision.
 */
esp_err_t yanis_wifi_reset(void);