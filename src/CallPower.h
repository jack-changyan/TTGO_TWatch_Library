#include <stdint.h>
#include <arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <LinkedList.h>
#include <LilyGoLib.h>
#include <LV_Helper.h>
#include <time.h>
#include <sntp.h>
#include "freertos/task.h"
#include "ui.h"

class Animal
{
public:
    String msg;
    String level;
    int time;
    String station;
    String alarm_id;
};

/* MSG数据 */
typedef struct
{
    String msg;
    String level;
    int time;
    String station;
    String alarm_id;
} MSG_DATA;

void CallPowerInit();

/* MQTT */
void Mqtt_init();
void Mqtt_Update();
void Mqtt_Push(String order, String alarm_id);
