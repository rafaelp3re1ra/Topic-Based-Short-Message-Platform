# Topic based short message platform

This repository contains an academic project developed as part of a practical assignment for the **Operating Systems** course. The project implements a platform for sending and receiving short messages organized by topics. Users can send messages to specific topics and subscribe to one or more topics. Whenever a message is sent to a subscribed topic, all users subscribed to that topic receive the message.
There is a server (manager) to manage users, topics and messages. There might be some errors, but overall it was a great project to learn about C programming for UNIX.

The system includes two programs:

1. **Manager Program:**
   - Handles message reception and distribution across topics.
   - Can remove users.
   - Can lock topics (no one can send and receive messages).

2. **Feed Program:**
   - Allows users to interact with the platform by sending and receiving messages.
   - Users can list active topics, subscribe to receive messages or unsubscribe. 

The user interface is text-based and operates in a terminal environment. Each user must use a separate terminal but remain on the same machine. Users are identified by a **unique** username during the session, without requiring prior registration or a password.

## Overview

The project was developed in C and addresses the following main components:

1. **Topic Management:**
   - Implementation of functionalities to create, remove, list topics, lock and unlock topics.
   
2. **User Management:**
   - Control and manipulation of users.

3. **Data Feed:**
   - Modules to process and display data related to users and topics.

4. **Modular Architecture:**
   - The project is structured modularly to facilitate organization and code reuse.

## Repository Structure

- **Headers and Implementations:**
  - `utils.h` and `utils.c`: Shared auxiliary functions.
  - `manager.c`: "Server".
  - `feed.c`: User platform.
  - Directory `managerUtils`:
    - `managerUtils.h` and `managerUtils.c`: Main functions for command handling and manager shutdown.
    - `userManagement.h` and `userManagement.c`: Functions for user management.
    - `topicManagement.h` and `topicManagement.c`: Functions for topic management.
  - Directory `feedUtils`:
    - `feedUtils.h` and `feedUtils.c`: Feed functions.

- **Makefile:**
  - Contains the rules to compile the project.

- **Report:**
  - [`Report.pdf`](Relatório_de_SO.pdf): Details about the development, functionalities, and results obtained.

## How to Compile and Run

1. Ensure you have a C compiler installed (e.g., GCC).

2. In the main project directory, run the following command to compile:
   ```
   make
   ```

3. After compilation, execute the manager program with the command:
   ```
   ./manager
   ```

4. To interact with the platform as a user, run the feed program in another terminal:
   ```
   ./feed <username (unique)>
   ```

5. Follow the on-screen instructions to send and receive messages. There's a *help* command to see the other commands.

## Key Results

The project demonstrated:

1. **Efficient management of users and topics:**
   - Operations to create, list, and remove entities were successfully implemented.

2. **Code modularity:**
   - Architecture that facilitates project maintenance and expansion.

3. **Integration of functionalities:**
   - Feed components interact cohesively with user and topic management.

## Authors

- [Rafael Pereira](https://github.com/rafaelp3re1ra)
- [Miguel Cardoso](https://github.com/miguelfcardoso)
