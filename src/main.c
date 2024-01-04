/*
 * File: traffic_light_controller.c
 * Author: Hugo Allaire
 * Description: This C file implements a simple traffic light controller state machine using the RP2040 microcontroller.
 *              By default the states lights are cycled between the four states using timers.
 *              When the pedestrian button is pressed, the SM goes to the WARNIN_CARS state directly, to signal the cars that the pedestrian will soon pass.
 *              The traffic light cycles are made to mimic the french system.
 */


#include <stdio.h>
#include "pico/stdlib.h"

#define DEBUG

const uint32_t pedestrians_delay_ms = 8000;   // Time for the Pedestrians to pass (ms)
const uint32_t warning_delay_ms = 4000;       // Amount of time the traffic light stays in both warning states (ms)
const uint32_t cars_delay_ms = 16000;         // Time for the cars to pass (ms)

// Stores an hardware alarm pool
alarm_pool_t* traffic_alarm_pool;

// Remember if the timers are set
// The name of the alarm is the name of it's destination state
alarm_id_t pedestrians_pass_alarm, pedestrians_warning_alarm, cars_warning_alarm, cars_pass_alarm;

// Led pins
const uint CARS_RED_PIN = 7;
const uint CARS_YELLOW_PIN = 6;
const uint CARS_GREEN_PIN = 5;
const uint PEDESTRIANS_RED_PIN = 4;
const uint PEDESTRIANS_GREEN_PIN = 28;

// Button pin
const uint PEDESTRIANS_BUTTON_PIN = 29;

typedef enum {
    PEDESTRIANS_PASS,   // Pedestrians can pass
    PEDESTRIANS_WARNING,// Notify the pedestrians that cars will soon pass
    CARS_WARNING,       // Notify the cars that pedestrians will soon pass
    CARS_PASS           // Cars can pass
} TrafficState;

// Remembers the state of the traffic lights
volatile TrafficState state;

int64_t set_pedestrians_pass() {
    state = PEDESTRIANS_PASS;
    gpio_put(CARS_RED_PIN, 1);
    gpio_put(PEDESTRIANS_GREEN_PIN, 1);

    gpio_put(CARS_YELLOW_PIN, 0);
    gpio_put(CARS_GREEN_PIN, 0);
    gpio_put(PEDESTRIANS_RED_PIN, 0);

    #ifdef DEBUG
    printf("Switching to state %d\n", state);
    #endif

    return 0;
}

int64_t set_pedestrians_warning() {
    state = PEDESTRIANS_WARNING;
    gpio_put(CARS_RED_PIN, 1);
    gpio_put(PEDESTRIANS_RED_PIN, 1);

    gpio_put(CARS_YELLOW_PIN, 0);
    gpio_put(CARS_GREEN_PIN, 0);
    gpio_put(PEDESTRIANS_GREEN_PIN, 0);

    #ifdef DEBUG
    printf("Switching to state %d\n", state);
    #endif

    return 0;
}

int64_t set_cars_warning() {
    state = CARS_WARNING;
    gpio_put(CARS_YELLOW_PIN, 1);
    gpio_put(PEDESTRIANS_RED_PIN, 1);

    gpio_put(CARS_RED_PIN, 0);
    gpio_put(CARS_GREEN_PIN, 0);
    gpio_put(PEDESTRIANS_GREEN_PIN, 0);

    #ifdef DEBUG
    printf("Switching to state %d\n", state);
    #endif

    return 0;
}

int64_t set_cars_pass() {
    state = CARS_PASS;
    gpio_put(PEDESTRIANS_RED_PIN, 1);
    gpio_put(CARS_GREEN_PIN, 1);

    gpio_put(CARS_RED_PIN, 0);
    gpio_put(CARS_YELLOW_PIN, 0);
    gpio_put(PEDESTRIANS_GREEN_PIN, 0);

    #ifdef DEBUG
    printf("Switching to state %d\n", state);
    #endif

    return 0;
}

int main(){

    stdio_init_all(); // Initiate all the RP2040 GPIOs to the default state

    gpio_init(CARS_RED_PIN);
    gpio_set_dir(CARS_RED_PIN, GPIO_OUT);

    gpio_init(CARS_YELLOW_PIN);
    gpio_set_dir(CARS_YELLOW_PIN, GPIO_OUT);

    gpio_init(CARS_GREEN_PIN);
    gpio_set_dir(CARS_GREEN_PIN, GPIO_OUT);

    gpio_init(PEDESTRIANS_RED_PIN);
    gpio_set_dir(PEDESTRIANS_RED_PIN, GPIO_OUT);

    gpio_init(PEDESTRIANS_GREEN_PIN);
    gpio_set_dir(PEDESTRIANS_GREEN_PIN, GPIO_OUT);

    gpio_init(PEDESTRIANS_BUTTON_PIN);
    gpio_set_dir(PEDESTRIANS_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(PEDESTRIANS_BUTTON_PIN);


    traffic_alarm_pool = alarm_pool_create_with_unused_hardware_alarm(4); // Create a hardware timer pool with 2 concurrent timers max
    
    set_pedestrians_pass(); // Start in pedestrians state
    
    while(true){
        switch (state) {
            case PEDESTRIANS_PASS:
                #ifdef DEBUG
                printf("Current state is %d\n", state);
                #endif

                if(pedestrians_warning_alarm == 0) // Set PEDESTRIANS_PASS -> PEDESTRIANS_WARNING timer if not already set 
                    pedestrians_warning_alarm = alarm_pool_add_alarm_in_ms(traffic_alarm_pool, pedestrians_delay_ms, set_pedestrians_warning, NULL, false);

                if(pedestrians_pass_alarm != 0){ // Reset warning
                    alarm_pool_cancel_alarm(traffic_alarm_pool, pedestrians_pass_alarm);
                    pedestrians_pass_alarm = 0;
                }
                
                break;

            case PEDESTRIANS_WARNING:
                #ifdef DEBUG
                printf("Current state is %d\n", state);
                #endif
                
                if(cars_pass_alarm == 0) // Set PEDESTRIANS_WARNING -> CARS_PASS timer if not already set
                    cars_pass_alarm = alarm_pool_add_alarm_in_ms(traffic_alarm_pool, warning_delay_ms, set_cars_pass, NULL, false);
                
                if(pedestrians_warning_alarm != 0){ // Reset cars timer
                    alarm_pool_cancel_alarm(traffic_alarm_pool, pedestrians_warning_alarm);
                    pedestrians_warning_alarm = 0;
                }
                
                break;

            case CARS_WARNING:
                #ifdef DEBUG
                printf("Current state is %d\n", state);
                #endif
                
                if(pedestrians_pass_alarm == 0) // Set CARS_WARNING -> PEDESTRIANS_PASS timer if not already set
                    pedestrians_pass_alarm = alarm_pool_add_alarm_in_ms(traffic_alarm_pool, warning_delay_ms, set_pedestrians_pass, NULL, false);
                
                if(cars_warning_alarm != 0){ // Reset cars timer
                    alarm_pool_cancel_alarm(traffic_alarm_pool, cars_warning_alarm);
                    cars_warning_alarm = 0;
                }
                
                break;

            case CARS_PASS:
                #ifdef DEBUG
                printf("Current state is %d\n", state);
                #endif
                
                if(cars_warning_alarm == 0) // Set CARS_PASS -> CARS_WARNING timer if not already set
                    cars_warning_alarm = alarm_pool_add_alarm_in_ms(traffic_alarm_pool, cars_delay_ms, set_cars_warning, NULL, false);

                if(cars_pass_alarm != 0){ // Reset pedestrians timer
                    alarm_pool_cancel_alarm(traffic_alarm_pool, cars_pass_alarm);
                    cars_pass_alarm = 0;
                }

                if(!gpio_get(PEDESTRIANS_BUTTON_PIN)) // If a pedestrian presses the button
                    set_cars_warning();
                
                break;
            
            default: return -1;
        }
        sleep_ms(250);
    }
    
    return 0;
}