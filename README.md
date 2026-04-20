![Python](https://img.shields.io/badge/Python-3.10-blue)
![ROS2](https://img.shields.io/badge/ROS2-Humble-22314E)
![Ubuntu](https://img.shields.io/badge/Ubuntu-22.04-E95420)
![Raspberry Pi](https://img.shields.io/badge/Raspberry%20Pi-4-A22846)
![OpenCV](https://img.shields.io/badge/OpenCV-Computer%20Vision-green)

# Smart Elevator Control System

Smart Elevator Control System is a Raspberry Pi and ROS 2 based elevator prototype that combines hardware control, order scheduling, GUI interaction, and AI-ready input modules. The project is designed as a secure and expandable elevator platform that can accept requests from internal controls, external panels, hand gestures, keypad input, and network clients.

The current prototype uses ROS 2 Humble on Ubuntu 22.04, Arduino-based motor control, and Python modules for computer vision and user interaction. The architecture is modular, which makes it easy to extend with mobile control, face recognition, and additional IoT services.

## Project Demo

<p align="center">
  <img src="media/simple_elevator.gif" alt="Elevator demo" height="300"/>
  <img src="media/door.gif" alt="Door demo" height="300"/>
</p>

## System Diagrams

### System Overview

<p align="center">
  <img src="media/system_info.jpg" alt="System overview diagram" width="800"/>
</p>

### ROS 2 Communication Graph

<p align="center">
  <img src="media/rosgraph.jpg" alt="ROS2 graph" width="800"/>
</p>

### Hardware Diagram

<p align="center">
  <img src="media/circuit_complete.png" alt="Hardware circuit diagram" width="800"/>
</p>

## Key Features

### Implemented in the current prototype

- ROS 2 node-based elevator architecture
- Elevator motion control through Arduino and PID control
- Order scheduling with FCFS or SJF with aging
- GUI simulation for internal and external elevator panels
- Serial communication between Raspberry Pi and Arduino
- Socket-based request handling for remote elevator calls
- Raspberry Pi input integration for keypad, hand gestures, and cabin load monitoring
- OpenCV and MediaPipe ready pipeline for lightweight vision processing

### Planned or expandable features

- Face recognition for authorized-user access
- Speech-based command input
- Mobile application control over Wi-Fi
- User database and access logging
- Full floor and building mapping for larger deployments

## How The System Works

1. A user sends a request using one of the supported input methods.
2. The request is published to the `order` topic as an `Order` message.
3. The scheduler selects the next target using the configured scheduling algorithm.
4. The hardware interface sends motion targets to the Arduino controller.
5. The Arduino drives the elevator motor and door actuator, then reports state feedback.
6. ROS 2 nodes publish the current floor, door position, geometry, and load information.

## Core Software Components

| Component | Description |
| --- | --- |
| `scheduler.py` | Selects the next elevator request using FCFS or SJF with aging. |
| `HW_interface.py` | ROS 2 action server that communicates with the Arduino controller over serial. |
| `GUI.py` | Desktop GUI that simulates internal and external elevator buttons. |
| `raspberry_interface.py` | Raspberry Pi integration node for hand gestures, face/load monitoring, keypad input, and socket input. |
| `android_interface.py` | TCP interface for Android or remote clients that publish elevator orders. |
| `HW_controller.ino` | Arduino firmware that executes motor and door control and returns sensor feedback. |

## AI and Input Modules

### Hand Gesture Input

- Uses MediaPipe Hands for real-time finger counting
- Can map a detected finger count to the requested floor
- Supports confirmation logic before publishing an order

### Face and Cabin Load Monitoring

- Uses OpenCV Haar Cascade face detection
- Estimates how many people are inside the cabin
- Can block new internal orders when cabin capacity is exceeded

### Keypad Security

- Simulates a keypad-based floor request workflow
- Supports clear and confirm operations
- Can be extended to real hardware keypad input on Raspberry Pi GPIO

### Network and Mobile Input

- Accepts remote requests using TCP socket messages
- Can be connected to a mobile app or another client device
- Designed for future IoT expansion

## Technologies Used

- Python 3.10
- ROS 2 Humble
- Ubuntu 22.04
- OpenCV
- MediaPipe
- Tkinter
- PySerial
- Arduino Uno

## Hardware Used

- Raspberry Pi 4
- Arduino Uno
- Pi Camera Module v2 or USB webcam
- DC motor
- Servo motor
- Ultrasonic sensor for current prototype position feedback
- Keypad
- LCD display or GUI panel for user interaction

The hardware component list is also available in [media/components.csv](media/components.csv).

## Project Structure

```text
elevator-main/
|-- arduino/
|   |-- HW_controller/
|   |   `-- HW_controller.ino
|-- media/
|   |-- circuit_complete.png
|   |-- rosgraph.jpg
|   |-- simple_elevator.gif
|   `-- system_info.jpg
`-- ROS2_pkg/
    `-- simple_elevator/
        |-- action/
        |   `-- Go.action
        |-- launch/
        |   `-- simple_elevator_launch.py
        |-- msg/
        |   |-- Geometry.msg
        |   |-- Order.msg
        |   |-- Scheduled.msg
        |   `-- State.msg
        `-- src/
            |-- GUI.py
            |-- HW_interface.py
            |-- android_interface.py
            |-- raspberry_interface.py
            |-- scheduler.py
            `-- socket_client.py
```

## Setup Requirements

Before running the project, make sure the Raspberry Pi or Ubuntu machine has:

- ROS 2 Humble installed
- Python 3.10
- OpenCV
- PySerial
- Tkinter
- MediaPipe for hand tracking

Example packages:

```bash
sudo apt update
sudo apt install python3-opencv python3-serial python3-tk
python3 -m pip install mediapipe
```

## How To Run

Build the ROS 2 workspace, source it, then launch the system:

```bash
colcon build --packages-select simple_elevator
source install/setup.bash
ros2 launch simple_elevator simple_elevator_launch.py
```

## Future Improvements

- Replace the ultrasonic sensor with an encoder for more precise position tracking
- Add real face recognition with authorized user profiles
- Add speech recognition for voice-controlled elevator calls
- Add persistent logging and database-backed users
- Connect the system to a mobile dashboard or cloud service

## Project Photos

<p align="center">
  <img src="media/in_AI-faculty.jpg" alt="Prototype photo 1" width="32%"/>
  <img src="media/in_Engineering-Faculty.jpg" alt="Prototype photo 2" width="32%"/>
  <img src="media/GUI.jpg" alt="GUI photo" width="32%"/>
</p>

## Video

[Watch the project on YouTube](https://youtu.be/9bC5t68XpNU)

## Author

Error 404 Team
