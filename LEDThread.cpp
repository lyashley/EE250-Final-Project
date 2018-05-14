/**
 * Copyright (c) 2017, Autonomous Networks Research Group. All rights reserved.
 * Developed by:
 * Autonomous Networks Research Group (ANRG)
 * University of Southern California
 * http://anrg.usc.edu/
 *
 * Contributors:
 * Jason A. Tran <jasontra@usc.edu>
 * Bhaskar Krishnamachari <bkrishna@usc.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * with the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * - Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimers.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *     this list of conditions and the following disclaimers in the 
 *     documentation and/or other materials provided with the distribution.
 * - Neither the names of Autonomous Networks Research Group, nor University of 
 *     Southern California, nor the names of its contributors may be used to 
 *     endorse or promote products derived from this Software without specific 
 *     prior written permission.
 * - A citation to the Autonomous Networks Research Group must be included in 
 *     any publications benefiting from the use of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH 
 * THE SOFTWARE.
 */

/**
 * @file       LEDThread.cpp
 * @brief      Implementation of thread that handles LED requests.
 *
 * @author     Jason Tran <jasontra@usc.edu>
 * @author     Bhaskar Krishnachari <bkrishna@usc.edu>
 */

#include "LEDThread.h"
#include "MQTTmbed.h"
#include "MQTTNetwork.h"

#include "MQTTClient.h"

Mail<MailMsg, LEDTHREAD_MAILBOX_SIZE> LEDMailbox;

//static DigitalOut led2(LED2);

static char *topic = "m3pi-mqtt-ee250/move";
static char *topic1 = "m3pi-mqtt-ee250/ultrasonic";

//int state;
int count = 0;
int flag = 0;

extern void movement(char command, char speed, int delta_t);

void LEDThread(void *args) 
{
    int i = 0;
    int state = 0;
    int state_wasd = 0;
    float ADC;
    float buf[125];
    float distance;

    MQTT::Client<MQTTNetwork, Countdown> *client = (MQTT::Client<MQTTNetwork, Countdown> *)args;
    MailMsg *msg;
    MQTT::Message message;
    osEvent evt;
    char pub_buf[16];

    // AnalogIn Ain(p15); // Ultrasonic senser plugged into pin 15
    
    // while(state_wasd == 0) {
    //     evt = LEDMailbox.get();
    //     if(evt.status == osEventMail){
    //         msg = (MailMsg *)evt.value.p;
    //         if(msg->content[1] == 119){  // ASCII for lowercase w
    //             state_wasd = 1;
    //             printf("w");
    //         }
    //         else if(msg->content[1] == 97){ // ASCII for lowercase a
    //             state_wasd = 2;
    //             printf("a");
    //         }
    //         else if(msg->content[1] == 115){ // ASCII for lowercase s
    //             state_wasd = 3;
    //             printf("s");
    //         }
    //         else if(msg->content[1] == 100){ // ASCII for lowercase d
    //             state_wasd = 4;
    //             printf("d");
    //         }
    //     }
    // }

    /*while(state_wasd == 1) {
        evt = LEDMailbox.get();
        if(evt.status == osEventMail){
            msg = (MailMsg *)evt.value.p;
            if(msg->content[1] == 119){  // ASCII for lowercase w
                state_wasd = 1;
            }
            else if(msg->content[1] == 97){ // ASCII for lowercase a
                state_wasd = 2;
            }
            else if(msg->content[1] == 115){ // ASCII for lowercase s
                state_wasd = 3;
            }
            else if(msg->content[1] == 100){ // ASCII for lowercase d
                state_wasd = 4;
            }
            message.qos = MQTT::QOS0;   // Quality of Service level
            message.retained = false;
            message.dup = false;
            message.payload = (void*)pub_buf;
            message.payloadlen = 10; //MQTTclient.h takes care of adding null char?
            Lock the global MQTT mutex before publishing
            mqttMtx.lock();
            client->publish(topic, message);
            mqttMtx.unlock();
    }*/
    // while(1) {
    //     evt = LEDMailbox.get();
    //     if(evt.status == osEventMail){
    //         msg = (MailMsg *)evt.value.p;
    //         // printf(msg->content);

    //         if(msg->content == "119"){  // ASCII for lowercase w
    //             movement('w', 25, 100);
    //             printf("w\n");
    //         }
    //         else if(msg->content == "97"){ // ASCII for lowercase a
    //             movement('a', 25, 100);
    //             printf("a\n");
    //         }
    //         else if(msg->content == "115"){ // ASCII for lowercase s
    //             movement('s', 25, 100);
    //             printf("s\n");
    //         }
    //         else if(msg->content == "100"){ // ASCII for lowercase d
    //             movement('d', 25, 100);
    //             printf("d\n");
    //         }


           

    //         message.qos = MQTT::QOS0;   // Quality of Service level
    //         message.retained = true;
    //         message.dup = false;
    //         message.payload = (void*)pub_buf;
    //         message.payloadlen = 10; //MQTTclient.h takes care of adding null char?
    //         /* Lock the global MQTT mutex before publishing */
    //         mqttMtx.lock();
    //         client->publish(topic, message);
    //         mqttMtx.unlock();
    // }
    // if(state == 0){
    //     ADC = Ain.read();
            
    //     distance = ADC * (2.38/0.0032); // distance in cm
    //     buf[i] = distance;
    //     printf("Distance: %d: %f\n", i, buf[i]);
    //     i++;
    // }

    /*while(1) {

        evt = LEDMailbox.get();

        if(evt.status == osEventMail) {
            msg = (MailMsg *)evt.value.p;

            // the second byte in the message denotes the action type
            switch (msg->content[1]) {
                case LED_THR_PUBLISH_MSG:   
                    printf("LEDThread: received command to publish to topic"
                            "m3pi-mqtt-ee250/move\n");
                    message.qos = MQTT::QOS0;   // Quality of Service level
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 2; //MQTTclient.h takes care of adding null char?
                    // Lock the global MQTT mutex before publishing
                    mqttMtx.lock();
                    client->publish(topic_move, message);
                    mqttMtx.unlock();
                    break;
               	case LED_THR_PUBLISH_MSG:
                    printf("LEDThread: received command to publish to topic"
                           "m3pi-mqtt-example/led-thread\n");
                    pub_buf[0] = 'h';
                    pub_buf[1] = 'i';
                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 2; //MQTTclient.h takes care of adding null char?
                    // Lock the global MQTT mutex before publishing
                    mqttMtx.lock();
                    client->publish(topic, message);
                    mqttMtx.unlock();
                    break;
                case LED_ON_ONE_SEC:
                    printf("LEDThread: received message to turn LED2 on for"
                           "one second...\n");
                    led2 = 1;
                    wait(1);
                    led2 = 0;
                    break;
                case LED_BLINK_FAST:
                    printf("LEDThread: received message to blink LED2 fast for"
                           "one second...\n");
                    for(int i = 0; i < 10; i++)
                    {
                        led2 = !led2;
                        wait(0.1);
                    }
                    led2 = 0;
                    break;
                default:
                    printf("LEDThread: invalid message\n");
                    break;
            }    */        

            // LEDMailbox.free(msg);

            
        // }
} /* while */

   /* this should never be reached */ 

Mail<MailMsg, LEDTHREAD_MAILBOX_SIZE> *getLEDThreadMailbox() 
{
    return &LEDMailbox;
}


