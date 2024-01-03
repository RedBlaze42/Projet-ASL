/*
 * File: traffic_light_controller.c
 * Author: Hugo Allaire
 * Description: This C file implements a simple traffic light controller state machine using the RP2040 microcontroller.
 *              By default lights are cycled between the tree states using timers.
 *              When a pedestrian presses the "boot" button on the rp2040, it will go directly to the yellow state.
 */


#include <stdio.h>
#include "pico/stdlib.h"
#include "../include/bootsel_button.h"
#include <tusb.h>

const uint32_t red_delay_ms = 8000;      // Time for the Pedestrians to pass (ms)
const uint32_t yellow_delay_ms = 4000;   // Amount of time the traffic light stays yellow (ms)
const uint32_t green_delay_ms = 16000;   // Maximum amount of time the traffic light can stay red (ms)

// Stores an hardware alarm pool
alarm_pool_t* traffic_alarm_pool;

// Remember if the timers are set
alarm_id_t red_alarm_id, yellow_alarm_id, green_alarm_id;

typedef enum {
    RED,        // Pedestrians can pass
    YELLOW,     // Waiting for the cars to pass before switching to red
    GREEN       // Cars can pass
} TrafficState;

// Remembers the state of the traffic lights
volatile TrafficState state;

int64_t set_yellow_callback() {
    state = YELLOW;
    return 0;
}

int64_t set_red_callback() {
    state = RED;
    return 0;
}

int64_t set_green_callback() {
    state = GREEN;
    return 0;
}

int main(){

    stdio_init_all(); // Initiate all the RP2040 GPIOs to the default state

    traffic_alarm_pool = alarm_pool_create_with_unused_hardware_alarm(3); // Create a hardware timer pool with 3 timers max
    
    state = RED; // Start in RED state
    
    while(true){
        switch (state) {
            case RED:
                printf("Current state is red\n");

                if(red_alarm_id ==0) // Set red -> green timer if not already set 
                    red_alarm_id = alarm_pool_add_alarm_in_ms(traffic_alarm_pool, red_delay_ms, set_green_callback, NULL, false);

                if(yellow_alarm_id != 0){ // Reset yellow
                    yellow_alarm_id = 0;
                    alarm_pool_cancel_alarm(traffic_alarm_pool, yellow_alarm_id);
                }
                
                break;

            case YELLOW:
                printf("Current state is yellow\n");
                
                if(yellow_alarm_id == 0) // Set yellow -> red timer if not already set
                    yellow_alarm_id = alarm_pool_add_alarm_in_ms(traffic_alarm_pool, yellow_delay_ms, set_red_callback, NULL, false);
                
                if(green_alarm_id != 0){ // Reset green timer
                    green_alarm_id = 0;
                    alarm_pool_cancel_alarm(traffic_alarm_pool, green_alarm_id);
                }
                
                break;

            case GREEN:
                printf("Current state is green\n");
                
                if(green_alarm_id == 0) // Set green -> yellow timer if not already set
                    green_alarm_id = alarm_pool_add_alarm_in_ms(traffic_alarm_pool, green_delay_ms, set_yellow_callback, NULL, false);

                if(red_alarm_id != 0){ // Reset red timer
                    red_alarm_id = 0;
                    alarm_pool_cancel_alarm(traffic_alarm_pool, green_alarm_id);
                }

                if(get_bootsel_button()) // If a pedestrian presses the button
                    state = YELLOW;
                
                break;
            
            default: return -1;
        }
        sleep_ms(250);
    }
    
    return 0;
}