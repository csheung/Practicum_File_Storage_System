# USB Synchronization Server

This project consists of a server and a client application that enable USB devices to be synchronized remotely. The server runs on a machine with USB devices connected, and the client sends commands to request synchronization functions.<br>

## Overview

The server application monitors the USB devices and performs synchronization tasks based on the client's commands. The client can send commands to the server to trigger various synchronization tasks, such as scanning the USB devices, synchronizing files and directories, and creating new files or directories in the USB devices.<br>

The server uses a multi-threaded approach to handle multiple clients simultaneously, providing efficient and scalable synchronization services.<br>

## Features
- Scan connected USB devices<br>
- Synchronize files and directories between USB devices<br>
- Create new files or directories in the USB devices<br>
- Handle multiple clients simultaneously<br>

## Usage
Execution: make<br>
Clear executables: make clean<br>

### Server
1. Compile the server application with the following command:
`gcc server.c -o server -lpthread`
2. Run the server application:
`./server`

### Client
1. Compile the client application with the following command:
`gcc client.c -o client`
2. Run the client application:
`./client`
3. Send commands to the server to request synchronization functions. Example commands:
* SCAN: Scan connected USB devices
* SYNC: Synchronize files and directories between USB devices
* CREATE_FILE [file_path] [file_content]: Create a new file in the USB devices with the specified file path and content
* CREATE_DIR [dir_path]: Create a new directory in the USB devices with the specified directory path

To find the product ID and vendor ID of your USB device, you can use the following steps:<br>
Open a terminal on your computer.<br>
Type the command lsusb and press Enter. This will display a list of all USB devices connected to your computer.<br>
Find the entry for the USB device you are interested in. The entry will look something like this: Bus 002 Device 008: ID 0781:5567 SanDisk Corp. Cruzer Blade<br>
Take note of the numbers after "ID". The first number is the vendor ID, and the second number is the product ID. In this example, the vendor ID is 0781 and the product ID is 5567.<br>

