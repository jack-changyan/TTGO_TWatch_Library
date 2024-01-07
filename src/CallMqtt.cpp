#include "CallPower.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <cJSON.h>

#define MSG_BUFFER_SIZE (200)
// LinkedList
// Let's define a new class
Animal animal;
LinkedList<Animal *> myAnimalList = LinkedList<Animal *>();
// Update these with values suitable for your network.

// extern TaskHandle_t xledLight;
// extern uint32_t led_mode;
extern uint64_t chipid;

const char *mqtt_server = "lab.laochu.cc";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char push_msg[MSG_BUFFER_SIZE];
int value = 0;
String msgmsg;

MSG_DATA msg_data;

void callback(char *topic, byte *payload, unsigned int length)
{
    String userControlString = "";
    cJSON *reqObject = NULL;
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        userControlString += (char)payload[i];
    }

    reqObject = cJSON_Parse(userControlString.c_str());
    if (reqObject == NULL)
    {
        Serial.println("JSON parse error");
        return;
    }
    cJSON *msgObject = cJSON_GetObjectItem(reqObject, "msg");
    msg_data.msg = msgObject->valuestring;

    cJSON *levelObject = cJSON_GetObjectItem(reqObject, "level");
    msg_data.level = levelObject->valuestring;

    cJSON *timelObject = cJSON_GetObjectItem(reqObject, "time");
    msg_data.time = timelObject->valueint;

    cJSON *stationObject = cJSON_GetObjectItem(reqObject, "station");
    msg_data.station = stationObject->valuestring;

    cJSON *alarmObject = cJSON_GetObjectItem(reqObject, "alarm_id");
    msg_data.alarm_id = alarmObject->valuestring;

    // Serial.println(msg_data.msg);
    // Serial.println(msg_data.level);
    // Serial.println(msg_data.time);

    // Create a Cat
    Animal *cat = new Animal();
    cat->msg = msg_data.msg;
    cat->level = msg_data.level;
    cat->time = msg_data.time;
    cat->station = msg_data.station;
    cat->alarm_id = msg_data.alarm_id;
    // Add animals to list
    myAnimalList.add(cat);

    Serial.print("There are ");
    Serial.print(myAnimalList.size());
    Serial.print(" animals in the list. The mammals are: ");
    Serial.println("");
    Notice_Animation(ui_Panel2, 0);
    lv_label_set_text(ui_Label1, msg_data.station.c_str());
    lv_label_set_text(ui_Label2, msg_data.level.c_str());
    lv_label_set_text(ui_Label3, msg_data.msg.c_str());
    watch.setWaveform(0, 58); // play effect
    watch.run();              // play the effect!

    // int current = 0;
    // Animal *animal;
    // for (int i = 0; i < myAnimalList.size(); i++)
    // {

    //     // Get animal from list
    //     animal = myAnimalList.get(i);

    //     // If its a mammal, then print it's name

    //     // Avoid printing spacer on the first element
    //     if (current++)
    //         Serial.print(", ");

    //     // Print animal name
    //     Serial.print(animal->msg);
    //     Serial.print(animal->station);
    // }
    // Serial.println(".");

    // led_mode = 2;
    // xTaskNotifyGive(xledLight); // 激活LED任务进站状态
    // HAL::Moto_Play();
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        // String clientId = "ESP832Client-";

        // clientId += String(chipid);
        // Serial.print(clientId);
        char macValue[17]; // Don't forget one byte for the terminating NULL...

        sprintf(macValue, "%d", chipid);

        // Attempt to connect
        if (client.connect(macValue))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish("outTopic", "hello world");
            // ... and resubscribe
            String inTopic = "alarm/";
            inTopic += String(macValue);
            Serial.print(inTopic);
            client.subscribe(inTopic.c_str());
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void Mqtt_init()
{
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void Mqtt_Update()
{

    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    // unsigned long now = millis();
    // if (now - lastMsg > 2000)
    // {
    //     lastMsg = now;
    //     ++value;
    //     snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    //     Serial.print("Publish message: ");
    //     Serial.println(msg);
    //     client.publish("outTopic", msg);
    // }
}

void Mqtt_Push(String order, String alarm_id)
{
    // {'sn': '1', 'order': '1', 'timestamp': '123456789', 'alarm_id': '1702806080'}
    // unsigned long now = millis();
    // if (now - lastMsg > 2000)
    // {
    // lastMsg = now;
    // ++value;
    // snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    // Serial.print("Publish message: ");
    String msg = "{\"sn\":\"1\", \"order\": \"";
    msg += order;
    msg += "\", \"timestamp\": \"123456789\", \"alarm_id\": \"";
    msg += alarm_id;
    msg += "\"}";
    snprintf(push_msg, MSG_BUFFER_SIZE, "%ld", msg);
    Serial.println(push_msg);
    client.publish("order/du", msg.c_str());
    // }
}