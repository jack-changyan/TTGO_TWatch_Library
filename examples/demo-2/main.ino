
#include "lv_example_dome-2.h"
#include "CallPower.h"

void msg_zhendong()
{
    watch.setWaveform(0, 58); // play effect
    watch.run(); // play the effect!
}

void setup()
{
    Serial.begin(115200);

    watch.begin();

    beginLvglHelper();

    CallPowerInit();
}

void loop()
{
    lv_task_handler();
    delay(5);
}