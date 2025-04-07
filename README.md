# CSCN72050 Term Project: Robotic Communication and Control


## Project Overview <a name="project-overview"></a>

This project involves designing and developing a webserver-based application to serve as an operations station for a tele-operated robot. The project focuses on implementing a unique application layer protocol, handling UDP socket communications, and providing a RESTful web GUI to control the robot and receive its telemetry data. Through this project, you will simulate robot control in a competitive demonstration exercise.

### Key Features
- **Application Layer Protocol Implementation**:
  - Review, interpret, and implement the predefined application layer protocol.
  - Create and test packet structures for sending commands and receiving telemetry.

- **Socket Communication**:
  - Develop a robust class to manage UDP socket communications.
  - Ensure reliable command transmission and real-time telemetry reception.

- **RESTful Webserver GUI**:
  - Build a webserver application that offers a user-friendly interface for robot control.
  - Integrate the GUI with the backend socket layer using RESTful principles.

- **Robot Operation Modes**:
  - **Waiting**: The robot listens for incoming command packets.
  - **Drive**: Executes drive commands, sends ACK packets, then returns to waiting mode.
  - **Sleep**: Resets internal counters and telemetry data upon receiving a sleep command.

- **Competition Simulation**:
  - Demonstrate system performance in a live task execution competition.
  - Performance judged on execution times, accuracy, and operational procedures.

## Technology Stack <a name="technology-stack"></a>

[![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://isocpp.org) [![HTML](https://img.shields.io/badge/HTML-1572B6?style=for-the-badge&logo=html5&logoColor=white)](https://developer.mozilla.org/en-US/docs/Web/HTML) [![CSS](https://img.shields.io/badge/CSS-1572B6?style=for-the-badge&logo=css3&logoColor=white)](https://developer.mozilla.org/en-US/docs/Web/CSS) [![JavaScript](https://img.shields.io/badge/JavaScript-F7DF1E?style=for-the-badge&logo=javascript&logoColor=black)](https://developer.mozilla.org/en-US/docs/Web/JavaScript) [![Visual Studio](https://img.shields.io/badge/Visual%20Studio-5C2D91?style=for-the-badge&logo=visual-studio&logoColor=white)](https://visualstudio.microsoft.com)



**Dependencies**:
- Visual Studio



## Getting Started <a name="getting-started"></a>

1. **Clone the Repository**
    ```bash 
    git clone https://github.com/Nathancecixx/CPPRobot.git
    ```
2. **Run the Project**
    - Open the provided solution in Visual Studio.
    - Run the Project

## Milestones
The project is broken down into the following deliverables:

1. Milestone 1:Application Layer Protocol
    - Design, implement, and test a class to handle the creation and parsing of command/telemetry packets.

    - Validate packet structures according to the protocol specifications.

2. Milestone 2: Socket Communications

    - Develop and test a class for managing UDP socket communications.

    - Ensure reliable sending of commands and receiving of telemetry data.

3. Milestone 3: Webserver GUI

    - Build a RESTful webserver application that serves as the command and control interface.

    - Integrate the protocol and socket communication classes to enable real-time robot control.

4. Milestone 4: Competition Day

    - Demonstrate the complete system by controlling the physical robot in a live competition setting.

    - Performance evaluation based on execution time, accuracy, and procedural efficiency.

