/*
 * Yanis Vision CSI Radar - Web UI
 *
 * Based on Espressif ESP-CSI wifi_sensing_demo (originally DrWalls).
 * Adds a local HTTP interface for viewing sensing results
 * from phones and computers on the same network.
 */

#include <stdio.h>
#include <string.h>
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"

#include "wifi_web_server.h"

#include "yanis_wifi.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "yanis_web";

static esp_wifi_sensing_fsm_handle_t s_fsm = NULL;
static uint8_t s_ap_mac[6] = {0};
static volatile bool s_motion_active = false;
static volatile int64_t s_last_motion_us = 0;
#define MOTION_DISPLAY_HOLD_US (3LL * 1000000LL)

extern const uint8_t dashboard_html_start[] asm("_binary_dashboard_html_start");
extern const uint8_t dashboard_html_end[]   asm("_binary_dashboard_html_end");
extern const uint8_t theme_css_start[]      asm("_binary_theme_css_start");
extern const uint8_t theme_css_end[]        asm("_binary_theme_css_end");

static esp_err_t root_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, (const char *)dashboard_html_start,
                           dashboard_html_end - dashboard_html_start);
}

static esp_err_t theme_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/css");
    return httpd_resp_send(req, (const char *)theme_css_start,
                           theme_css_end - theme_css_start);
}

static esp_err_t status_handler(httpd_req_t *req)
{
    char json[96];

    int64_t now = esp_timer_get_time();

    bool display_active =
        s_motion_active ||
        ((now - s_last_motion_us) < MOTION_DISPLAY_HOLD_US);

    /* CSI variance amplitude of the AP channel (jitter metric of the sensing FSM). */
    float variance = 0.0f;
    esp_wifi_sensing_fsm_channel_diag_t diag = {0};
    if (s_fsm &&
        esp_wifi_sensing_fsm_get_channel_diag(s_fsm, s_ap_mac, &diag) == ESP_OK) {
        variance = diag.jitter_value;
    }

    snprintf(json, sizeof(json),
             "{\"motion\":%s,\"variance\":%.4f}",
             display_active ? "true" : "false",
             (double)variance);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");

    return httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
}

void wifi_web_server_set_motion(bool active)
{
    s_motion_active = active;

    if (active)
    {
        s_last_motion_us = esp_timer_get_time();
    }
}

static esp_err_t reset_wifi_handler(httpd_req_t *req)
{
    ESP_LOGW(TAG, "Wi-Fi reset requested");

    esp_err_t err = yanis_wifi_reset();

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to erase Wi-Fi credentials: %s",
                 esp_err_to_name(err));

        httpd_resp_send_err(
            req,
            HTTPD_500_INTERNAL_SERVER_ERROR,
            "Failed to reset Wi-Fi");

        return err;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, "{\"success\":true}");

    /*
     * Give the HTTP response time to reach the browser
     * before restarting the ESP32.
     */
    vTaskDelay(pdMS_TO_TICKS(1000));

    esp_restart();

    return ESP_OK;
}

esp_err_t wifi_web_server_start(
    esp_wifi_sensing_fsm_handle_t fsm,
    const uint8_t ap_mac[6])
{
    s_fsm = fsm;
    memcpy(s_ap_mac, ap_mac, 6);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    ESP_LOGI(TAG, "Starting HTTP server");

    esp_err_t err = httpd_start(&server, &config);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "HTTP server failed: %s", esp_err_to_name(err));
        return err;
    }

    static const httpd_uri_t root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = NULL};

    err = httpd_register_uri_handler(server, &root);

    static const httpd_uri_t status = {
        .uri = "/api/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = NULL};

    err = httpd_register_uri_handler(server, &status);

    static const httpd_uri_t theme = {
        .uri = "/theme.css",
        .method = HTTP_GET,
        .handler = theme_handler,
        .user_ctx = NULL};

    err = httpd_register_uri_handler(server, &theme);

    static const httpd_uri_t reset_wifi = {
        .uri = "/api/reset-wifi",
        .method = HTTP_POST,
        .handler = reset_wifi_handler,
        .user_ctx = NULL};

    err = httpd_register_uri_handler(server, &reset_wifi);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register /api/reset-wifi");
        return err;
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register /api/status");
        return err;
    }
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register /");
        return err;
    }

    ESP_LOGI(TAG, "HTTP server started");

    return ESP_OK;
}