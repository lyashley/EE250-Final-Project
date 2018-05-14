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
 * Permission is hereby granted, flag of charge, to any person obtaining a copy 
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
 * @file       main.cpp
 * @brief      Example using an ESP8266 to pub/sub using MQTT on MBED OS.
 * 
 *             This example is specifically built for the FIE workshop.
 *
 * @author     Jason Tran <jasontra@usc.edu>
 * @author     Bhaskar Krishnachari <bkrishna@usc.edu>
 * 
 */

#include "mbed.h"
#include "m3pi.h"
#include "easy-connect.h"
#include "MQTTmbed.h"
#include "MQTTNetwork.h"
#include "MQTTClient.h"
#include "MailMsg.h"
#include "LEDThread.h"
#include "PrintThread.h"
#include <string>

extern "C" void mbed_reset();

/* connect this pin to both the CH_PD (aka EN) & RST pins on the ESP8266 just in case */
#define WIFI_HW_RESET_PIN       p26

/* Using a hostname instead of IP address has been unverified by us */
#define MQTT_BROKER_IPADDR      "128.125.124.160"  // eclipse.usc.edu == 128.125.124.160
#define MQTT_BROKER_PORT        11000

/* turn on easy-connect debug prints */
#define EASY_CONNECT_LOGGING    true

DigitalOut wifiHwResetPin(WIFI_HW_RESET_PIN);

/** Initialize the m3pi for robot movements. There is an atmega328p MCU in the
 *  3pi robot base. It's UART lines are connected to the LPC1768's p9 and p10.
 *  If you send the right sequence of UART characters to the atmega328p, it will
 *  move the robot for you. We provide a movement() function below for you to use
 */
m3pi m3pi(p23, p9, p10);

/* MQTTClient and TCPSocket (underneath MQTTNetwork) may not be thread safe. 
 * Lock this global mutex before any calls to publish(). 
 */
Mutex mqttMtx;

static char *topic = "m3pi-mqtt-ee250/move";

static char *topic1 = "m3pi-mqtt-ee250/ultrasonic";

int free_mode = 1;

/**
 * @brief      controls movement of the 3pi
 *
 *  If you want to use this function in a file outside of main.cpp, the easiest
 *  way to get access to it is to add
 *      
 *      extern movement(char command, char speed, int delta_t)
 *
 * in the .cpp file in which you want to use it.
 *
 * @param[in]  command  The movement command
 * @param[in]  speed    The speed of the movement (start by trying 25)
 * @param[in]  delta_t  The time for each movement in msec (start by trying 100)
 *
 * @return     { void }
 */
void movement(char command, char speed, int delta_t)
{
    if (command == 's')
    {
        m3pi.forward(speed);
        Thread::wait(delta_t);
        m3pi.stop();
    }
    else if (command == 'a')
    {
        m3pi.left(speed);
        Thread::wait(delta_t);
        m3pi.stop();
    }   
    else if (command == 'w')
    {
        m3pi.backward(speed);
        Thread::wait(delta_t);
        m3pi.stop();
    }
    else if (command == 'd')
    {
        m3pi.right(speed);
        Thread::wait(delta_t);
        m3pi.stop();
    }
}

/* Callback for any received MQTT messages */
void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    
    printf("msg len: %d\n", message.payloadlen);
    printf("msg : %c\n", message.payload);

    char* msg = (char*) message.payload;

    if( (string)msg == "119"){  // ASCII for lowercase w
        movement('s', 25, 500);
        wait(1);
        printf("w\n");
        free_mode = 0;
    }
    else if((string)msg == "101"){ // ASCII for lowercase a
        movement('a', 25, 500);
        wait(1);
        printf("a\n");
        free_mode = 0;
    }
    else if((string)msg == "115"){ // ASCII for lowercase s
        movement('w', 25, 500);
        wait(1);
        printf("s\n");
        free_mode = 0;
    }
    else if((string)msg == "100"){ // ASCII for lowercase d
        movement('d', 25, 500);
        wait(1);
        printf("d\n");
        free_mode = 0;
    }
    else if ((string)msg == "105"){
        free_mode = 1;
        printf("freemode\n");
        wait(2);
    }


}

// void messageSonar(MQTT::MessageData& md)
// {
//     MQTT::Message &message = md.message;
//     MailMsg *msg;

//      our messaging standard says the first byte denotes which thread to 
//        forward the packet payload to 

//     char* sonar_val = (char *)message.payload;

//     printf("Sonar reading: %c\n", sonar_val);
    
// }

int main()
{
    /* Uncomment this to see how the m3pi moves. This sequence of functions
       represent w-a-s-d like controlling. Each button press moves the robot
       at a speed of 25 (speed can be between -127 to 127) for 100 ms. Use
       functions like this in your program to move your m3pi when you get 
       MQTT messages! */
    // movement('w', 25, 100);
    // movement('w', 25, 100);
    // movement('w', 25, 100);
    // movement('w', 25, 100);
    // movement('a', 25, 100);
    // movement('a', 25, 100);
    // movement('a', 25, 100);
    // movement('a', 25, 100);
    // movement('d', 25, 100);
    // movement('d', 25, 100);
    // movement('d', 25, 100);
    // movement('d', 25, 100);
    // movement('s', 25, 100);
    // movement('s', 25, 100);
    // movement('s', 25, 100);
    // movement('s', 25, 100);

    wait(1); //delay startup 
    printf("Resetting ESP8266 Hardware...\n");
    wifiHwResetPin = 0;
    wait(1);
    wifiHwResetPin = 1;

    printf("Starting MQTT example with an ESP8266 wifi device using Mbed OS.\n");
    printf("Attempting to connect to access point...\n");

    /* wifi ssid/pw and wifi interface settings are set in mbed_app.json. 
     * NetworkInterface is mbed-OS's abstraction of any network interface */
    NetworkInterface* wifi = easy_connect(EASY_CONNECT_LOGGING);
    if (!wifi) {
        printf("Connection error! Your ESP8266 may not be responding.\n");
        printf("Try double checking your circuit or unplug/plug your LPC1768 board.\n");
        printf("Exiting.\n");
        return -1;
    }

    const char *ipAddr = wifi->get_ip_address();
    printf("Success! My IP addr: %s\n", ipAddr);
    char clientID[30];
    /* use ip addr as unique client ID */
    strcpy(clientID, ipAddr);

    /* wrapper for (NetworkInterface *wifi) to adapt to MQTTClient.h */
    MQTTNetwork mqttNetwork(wifi);
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    printf("Connecting to %s:%d\n", MQTT_BROKER_IPADDR, MQTT_BROKER_PORT);
    int retval = mqttNetwork.connect(MQTT_BROKER_IPADDR, MQTT_BROKER_PORT);
    if (retval != 0)
        printf("TCP connect returned %d\n", retval);

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;   //support only available up to ver. 3
    // data.keepAliveInterval = 60; //MQTTPacket_connectData_initializer sets this to 60
    data.clientID.cstring = clientID; 
    if ((retval = client.connect(data)) != 0)
        printf("connect returned %d\n", retval);


    /* define MQTTCLIENT_QOS2 as 1 to enable QOS2 (see MQTTClient.h) */
    /* This call attaches the messageArrived callback to handle MQTT messages 
       that arrive */
    if ((retval = client.subscribe(topic, MQTT::QOS0, messageArrived)) != 0)
        printf("MQTT subscribe returned %d\n", retval);

    /* This is a good point to launch your threads. If you want to create 
       another thread, you can look at the structure of the two threads we 
       provided and make a copy of it. Otherewise, you can gut out the two 
       threads and insert your application code. Read the LEDThread and 
       PrintThread files to understand how these threads work.*/
    Thread ledThr;
    Thread printThr;

    /* Here, we pass in a pointer to the MQTT client so the LED thread can 
       client.publish() messages */
    ledThr.start(callback(LEDThread, (void *)&client));

    /* Here, we do not pass the pointer in. This means the printing thread 
       won't be able to publish any MQTT messages. Modify this accordingly if
       you need to publish. */
    printThr.start(printThread);

    /* The main thread will now run in the background to keep the MQTT/TCP 
     connection alive. MQTTClient is not an asynchronous library. Paho does
     have MQTTAsync, but some effort is needed to adapt mbed OS libraries to
     be used by the MQTTAsync library. Please do NOT do anything else in this
     thread. Let it serve as your background MQTT thread. */
    AnalogIn sonar(p15);

    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = true;
    message.dup = false;

    float thres = 20.0; // stop and recalculate if sonar reading is <= 20cm
    while(1) {
        Thread::wait(500);
        // printf("main: yielding...\n", client.isConnected());
        if (free_mode == 1)
        {
            float num = 0.0;
            int cnt=0;
            float sum =0;
            while (cnt<5){
                num = sonar.read()*2.38/0.0032;
                sum+=num;
                cnt++;
            }
            cnt = 0;
            num = sum/5;

            char buff[20];
            sprintf(buff, "distance: %f cm\n", num);       
            message.payload = (void*)buff;
            message.payloadlen = strlen(buff)+1;

            mqttMtx.lock();
            client.publish(topic1, message);
            mqttMtx.unlock();

            printf("distance: %f cm\n", num);
            // wait(1);
            if (num > thres){
                for (int i = 0; i < 5; i++){
                    movement('s', 25, 100);
                }
            }
            else{
                int n = 20;
                int max_i;
                float max_d = 0;
                int con_speed = 25;
                int con_t = 220;
                // rotate every 360/n degrees
                for (int i = 0; i < n; i++){
                    num = 0.0;
                    cnt=0;
                    sum =0;
                    while (cnt<5){
                        num = sonar.read()*2.38/0.0032;
                        sum+=num;
                        cnt++;
                    }
                    cnt = 0;
                    num = sum/5;
                    if (num > max_d){
                        max_d = num;
                        max_i = i;
                    }  
                    movement('d', con_speed, con_t);
                    wait(0.5);
                }
                if (max_i < n/2){
                    int j = 0;
                    while (j!=max_i){
                        movement('d', con_speed, con_t);
                        wait(0.5);
                        j++;
                    }
                }else{
                    int k = n;
                    while (k!=max_i){
                        movement('a', con_speed, con_t);
                        wait(0.5);
                        k--;
                    }
                }

                char buff[20];
                sprintf(buff, "LOW: %f cm\n", max_d);       
                message.payload = (void*)buff;
                message.payloadlen = strlen(buff)+1;

                mqttMtx.lock();
                client.publish(topic1, message);
                mqttMtx.unlock();
               
                printf("LOW: %f cm\n", max_d);
            }
        }
        


        if(!client.isConnected())
            mbed_reset(); //connection lost! software reset

        /* yield() needs to be called at least once per keepAliveInterval. */
        client.yield(500);
    }

    return 0;
}