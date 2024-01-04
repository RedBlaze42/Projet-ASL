import serial, time
from enum import Enum

class TrafficState(Enum):
    PEDESTRIANS_PASS = 0
    PEDESTRIANS_WARNING = 1
    CARS_WARNING = 2
    CARS_PASS = 3

# This must mirror the current configuration of the device
pedestrians_delay_ms = 8000   # Time for the Pedestrians to pass (ms)
warning_delay_ms = 4000       # Amount of time the traffic light stays in both warning states (ms)
cars_delay_ms = 16000         # Time for the cars to pass (ms)

device = serial.Serial("COM4", 9600) # Edit device address

delay_tolerance = 10 # Positive delay tolerance in milliseconds

test_duration = 300 # Seconds




# Utility function to get a monotonic time reference in milliseconds
def time_ms():
    return round(time.perf_counter_ns()/1_000_000)

# From the FSM diagram
allowed_transitions = {
    TrafficState.CARS_PASS:             TrafficState.CARS_WARNING,
    TrafficState.CARS_WARNING:          TrafficState.PEDESTRIANS_PASS,
    TrafficState.PEDESTRIANS_PASS:      TrafficState.PEDESTRIANS_WARNING,
    TrafficState.PEDESTRIANS_WARNING:   TrafficState.CARS_PASS,
}

# Maps the delays of the timers to the corresponding initial states
automatic_cycle_delays = {
    TrafficState.CARS_PASS:             cars_delay_ms,
    TrafficState.CARS_WARNING:          warning_delay_ms,
    TrafficState.PEDESTRIANS_PASS:      pedestrians_delay_ms,
    TrafficState.PEDESTRIANS_WARNING:   warning_delay_ms,
}

start_test_timestamp = time.time()
prev_transition_timeref = None

device_state = None

while time.time() - start_test_timestamp < test_duration:
    line = device.readline().decode().strip()
    
    if line.startswith("Switching to state "):
        
        incoming_state = TrafficState(int(line.lstrip("Switching to state ")))
        
        transition_timeref = time_ms()
        
        if prev_transition_timeref is not None and device_state is not None:
        
            if incoming_state != allowed_transitions[device_state]:
                raise Exception(f"Illegal switch from {device_state} to {incoming_state}")
        
            transition_delay = transition_timeref - prev_transition_timeref
            delay_error = transition_delay - automatic_cycle_delays[device_state]
            
            if delay_error < -delay_tolerance and device_state == TrafficState.CARS_PASS:
                print(f"WARNING: Negative delay error of {delay_error} ms, it's normal if you have pressed the pedestrian button, otherwise it's an error.")
            elif abs(delay_error) > delay_tolerance:
                raise Exception(f"Delay error of {delay_error} ms from {device_state} to {incoming_state}. Sensitivity is set at {delay_tolerance} ms")
            else:
                print(f"Switched from {device_state} to {incoming_state} after {transition_delay} ms with an error of {delay_error} ms")
        
        device_state = incoming_state
        prev_transition_timeref = transition_timeref
        
print("Test passed without any errors")