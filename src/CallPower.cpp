#include "CallPower.h"

uint64_t chipid;
String ssid = "USR";
String password = "12345678";

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

const char *time_zone = "CST-8";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

uint32_t lastMillis;
char buf[30];
// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
    Serial.println("Got time adjustment from NTP, Write the hardware clock");

    // Write synchronization time to hardware
    watch.hwClockWrite();
}

/* Mqtt thread */
static void MQTT_Thread(void *argument)
{
    for (;;)
    {
        Mqtt_Update();
        taskYIELD();
        vTaskDelay(100);
    }
}

/* hardware clock */
static void Clock_Thread(void *argument)
{
    for (;;)
    {
        // hardware clock
        struct tm hwTimeinfo;
        watch.getDateTime(&hwTimeinfo);
        Serial.print("Hardware clock :");
        Serial.println(&hwTimeinfo, "%A, %B %d %Y %H:%M:%S");
        strftime(buf, 30, "%H:%M", &hwTimeinfo);
        lv_label_set_text_fmt(ui_time, "%s", buf);
        taskYIELD();
        vTaskDelay(1000);
    }
}

void CallPowerInit()
{
    ui_init();

    chipid = ESP.getEfuseMac();                  // The chip ID is essentially its MAC address(length: 6 bytes).
    Serial.printf("ESP32 Chip ID = %d", chipid); // print High 2 bytes
    Serial.println("");

    WiFi.mode(WIFI_STA);
    // WiFi.onEvent(onWiFiEvent);

    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED)
    { // WiFi.status()函数的返回值是由NodeMCU的WiFi连接状态所决定的。
        Serial.print(".");
        lv_task_handler();
        vTaskDelay(4500); // 如果WiFi连接成功则返回值为WL_CONNECTED
    }

    // set notification call-back function
    sntp_set_time_sync_notification_cb(timeavailable);

    /**
     * NTP server address could be aquired via DHCP,
     *
     * NOTE: This call should be made BEFORE esp32 aquires IP address via DHCP,
     * otherwise SNTP option 42 would be rejected by default.
     * NOTE: configTime() function call if made AFTER DHCP-client run
     * will OVERRIDE aquired NTP server address
     */
    sntp_servermode_dhcp(1); // (optional)

    /**
     * This will set configured ntp servers and constant TimeZone/daylightOffset
     * should be OK if your time zone does not need to adjust daylightOffset twice a year,
     * in such a case time adjustment won't be handled automagicaly.
     */
    // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

    /**
     * A more convenient approach to handle TimeZones with daylightOffset
     * would be to specify a environmnet variable with TimeZone definition including daylight adjustmnet rules.
     * A list of rules for your zone could be obtained from https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
     */
    configTzTime(time_zone, ntpServer1, ntpServer2);

    /* MQTT init */
    Mqtt_init();

    /* Crete a higher level thread */
    xTaskCreatePinnedToCore(MQTT_Thread, "MQTT_Thread", 1024 * 4, NULL, 1, NULL, 0);

    /* Crete a higher level thread */
    xTaskCreatePinnedToCore(Clock_Thread, "Clock_Thread", 1024 * 4, NULL, 2, NULL, 0);
}