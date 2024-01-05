# RP2040 Traffic lights Finite State Machine in C

This program implements a traffic light state machine on an RP2040 microcontroller using hardware timers.
The traffic light consist of a RED, YELLOW and GREEN LEDs for the car lights and a RED and GREEN LEDs for the pedestrian ligths.

## Functionality

This state machine mirrors from the french traffic lights system.

There are four states which are automatically cycled by default. But there is a pedestrian button fo force the transition between CARS_PASS and CARS_WARNING states.

| State name              | Pedestrians lights color | Cars lights color | Description                                                     |
|-------------------------|--------------------------|-------------------|-----------------------------------------------------------------|
| **PEDESTRIANS_PASS**    | GREEN                    | RED               | The pedestrians can pass, the cars can't                        |
| **PEDESTRIANS_WARNING** | RED                      | RED               | The pedestrians can't begin crossing because cars will soon pass |
| **CARS_WARNING**        | RED                      | YELLOW            | The cars needs stop to prepare for the pedestrians to pass            |
| **CARS_PASS**           | RED                      | GREEN             | The cars can pass, the pedestrians can't                        |

The states are cycled automatically by hardware timers. The delays can be adjusted by editing this constants in the main.c file.

```C
const uint32_t pedestrians_delay_ms = 8000;   // Time for the Pedestrians to pass (ms)
const uint32_t warning_delay_ms = 4000;       // Amount of time the warning states are shown (ms)
const uint32_t cars_delay_ms = 16000;         // Time for the cars to pass (ms)
```

## FSM Diagram

![FSM_Diagram](https://raw.githubusercontent.com/RedBlaze42/RP2040-C-TrafficLights/main/images/FSM_diagram.svg)

## Video demonstration

[Here is a video showing the project in action](https://www.youtube.com/watch?v=MjUDOx1oxU8)

## Github actions

Binaries are available in the github actions tab.

## Build

First and foremost, clone this repository using git.

`git clone https://github.com/RedBlaze42/RP2040-C-TrafficLights.git`

### Linux

First, install all the packages required for building C for arm:

`sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib`

Then clone the pico sdk from inside this directory repository:

`cd RP2040-C-TrafficLights`

`git clone https://github.com/raspberrypi/pico-sdk.git`

Go inside and update the submodules to add tinyusb support:

`cd pico-sdk && git submodule update --init && cd ..`

You have to tell cmake where to fine the pico sdk, when still in the project root, you can run this command:

```export PICO_SDK_PATH=`pwd`/pico-sdk```

Create the build directory and go inside it:

`mkdir build && cd build`

Then you can configure and build the project:

`cmake .. && make`

The build files will be in your current directory (`RP2040-C-TrafficLights/build`).

### Windows
For windows, a full tutorial on how to insall the Raspberry Pi Pico C/C++ SDK for is available on [the official raspberry pi website](https://www.raspberrypi.com/news/raspberry-pi-pico-windows-installer/).

## Programming

While plugging the RP2040 to your computer, press the BOOTSEL button to program it. You can then mount the mass storage device and drop the `.uf2` file. The RP2040 should reboot and start the program.

## Wiring

You can edit the pin configuration by editing these lines in the main.c file
```c
// Led pins
const uint CARS_RED_PIN = 7;
const uint CARS_YELLOW_PIN = 6;
const uint CARS_GREEN_PIN = 5;
const uint PEDESTRIANS_RED_PIN = 4;
const uint PEDESTRIANS_GREEN_PIN = 28;

// Button pin
const uint PEDESTRIANS_BUTTON_PIN = 29;
```

The number here are refering to the GPIO pin numbers which is different from the board pin numbers.

You need to connect the other side of the pushbutton to `GND` because there is an internal pullup resistor activated on this pin.

Don't forget to connect resistors in series with the LEDs to protect them.

## Debug communication

This code uses the CMake build system recommended by the Raspberry Pi Pico C/C++ SDK.

By default, the debugging is done with the UART protocol via the usb port using the tinyusb submodule, but you can switch to the GPIO UART by changing this lines in the CMakeLists.txt file:
```Makefile
# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
```
by
```Makefile
# Disable usb output, enable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
```

## Testbench

A python testbench is available in the tests directory. It uses the USB communication to check if the state transitions are correct and made at the right time.

You need to install `Python 3` from the official [website](https://www.python.org/downloads/).
Or using `apt`: `sudo apt install python3 python3-pip python3-venv`

From inside the tests directory, create a virtual environment using `python -m venv rp2040_trafficlights_tb`.

Activate it with `source rp2040_trafficlights_tb/bin/activate` on Linux or `rp2040_trafficlights_tb\Scripts\bin\activate` on Windows.

Install the pyserial package by running the command `python -m pip install -r requirements.txt` from inside the `tests` directory.

Compile the project with the `DEBUG` macro, launch the testbench using `python device_test.py`.
By default, the testbench will run for 5 minutes.

This is the expected output:
```
Switched from TrafficState.PEDESTRIANS_WARNING to TrafficState.CARS_PASS after 4000 ms with an error of 0 ms
WARNING: Negative delay error of -9280 ms for CARS_PASS -> CARS_WARNING, it's normal if you have pressed the pedestrian button, otherwise it's an error.
Switched from TrafficState.CARS_WARNING to TrafficState.PEDESTRIANS_PASS after 4000 ms with an error of 0 ms
Switched from TrafficState.PEDESTRIANS_PASS to TrafficState.PEDESTRIANS_WARNING after 8000 ms with an error of 0 ms
Switched from TrafficState.PEDESTRIANS_WARNING to TrafficState.CARS_PASS after 4000 ms with an error of 0 ms
Switched from TrafficState.CARS_PASS to TrafficState.CARS_WARNING after 15999 ms with an error of -1 ms
Switched from TrafficState.CARS_WARNING to TrafficState.PEDESTRIANS_PASS after 4000 ms with an error of 0 ms
Switched from TrafficState.PEDESTRIANS_PASS to TrafficState.PEDESTRIANS_WARNING after 8000 ms with an error of 0 ms
Switched from TrafficState.PEDESTRIANS_WARNING to TrafficState.CARS_PASS after 4000 ms with an error of 0 ms
Switched from TrafficState.CARS_PASS to TrafficState.CARS_WARNING after 16000 ms with an error of 0 ms
...
Switched from TrafficState.PEDESTRIANS_WARNING to TrafficState.CARS_PASS after 4000 ms with an error of 0 ms
Switched from TrafficState.CARS_PASS to TrafficState.CARS_WARNING after 15999 ms with an error of -1 ms
Test passed without any errors
```
As you can see, I pressed the pedestrian button at the start of the test.

And this is when an error occurred:
```
Switched from TrafficState.PEDESTRIANS_WARNING to TrafficState.CARS_PASS after 4000 ms with an error of 0 ms
WARNING: Negative delay error of -14768 ms for CARS_PASS -> CARS_WARNING, it's normal if you have pressed the pedestrian button, otherwise it's an error.
Switched from TrafficState.CARS_WARNING to TrafficState.PEDESTRIANS_PASS after 4000 ms with an error of 0 ms
Switched from TrafficState.PEDESTRIANS_PASS to TrafficState.PEDESTRIANS_WARNING after 8000 ms with an error of 0 ms
Traceback (most recent call last):
  File "c:\Users\Hugo\OneDrive\Projet ASL\tests\device_test.py", line 61, in <module>       
    raise Exception(f"Illegal switch from {device_state} to {incoming_state}")
Exception: Illegal switch from TrafficState.PEDESTRIANS_WARNING to TrafficState.CARS_WARNING
```