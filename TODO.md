======================= Nov 13 2024 ==========================

# * Off boarding Things TODO *

# HIGH
- Test integrated steering_wheel_canduino.ino with all hardware components connected to make sure it works. (BIG TASK)

- Could use the send_interrupt file to test how Simulink interacts with the CAN messages? Connect to TI?

# MEDIUM
- Start assigning CAN ID's to various sensor data and things to transmit. Need to define how to handle that ID in the CAN_message_handler() switch case.
    CAN_read_from_network() is only meant for putting sensor values into containers (global vars) to be used however you want. 
    For example, in the steering canduino, the sensor values are put into global variables then would be displayed on the OLED screen.

- Test sending and receiving larger array CAN messages more extensively. I only tested with Arrays of size 1. The CAN lib's send function requires an array just FYI.

- SOLDER MORE CANDUINOS! The one of the USB ports are finnicky on the square one. Spares would be nice. DON'T SOLDER R8 (termination resistor for CAN bus). 
    If we are doing linear CAN bus, we have enough terminating nodes.


# LOW

- Wiring Harness of CAN network, figure out topology (Linear, star?), canduino's CAN_H and CAN_L need to be spliced if using linear topology.
    (Need wire going to CAN_H and leaving CAN_H, same for CAN_L. i.e need 2 wires per CAN connection)

- Created test files (CAN_send_interrupt_test, CAN_send_test, CAN_receive_test in CAN folder).
	- send_interrupt_test mimics how the steering wheel canduino should behave, receiving many CAN messages and occasionally sending shift messages with an interrupt.
	  This uses an Pin Change interrupt rather than hardware interrupt which the completed steering canduino will NOT use. This was only done for testing as I had no access to interrupt pins on the circle canduino.
	  Change this to a hardware interrupt and test.

- Solder Pin Headers on Canduinos? Would be convenient to have for testing, but idk how the wiring harness is looking like, plus PCB for steering wheel idk.

- Change sending strategy from individual sensors each loop to multiple sensor values each loop. I.E longer messages,
	- Pros: Less risk of missing CAN message with reduced messages. If a lot of data is being sent, and receive takes too long, could overwrite in receive buffers?
	- Cons: Would have to decode within the loop and assign containers based on received message buffer indexes. gross.
