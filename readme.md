# CS5600 Practicum 2

This project consists of a server and a client application that enable USB devices to be synchronized remotely. The server runs on a machine with USB devices connected, and the client sends commands to request synchronization functions.<br>

## Authors
Zhenyu Wang, Chun Sheung Ng

2023 Apr 20

## Overview

The server application monitors the USB devices and performs file management and synchronization tasks based on the client's commands. The client can send commands to the server to trigger various tasks, such as getting files, querying file information, creating directories, uploading files, and deleting files or directories on the remote file system. Additionally, the server maintains a simple mirrored file system that writes information to two USB devices simultaneously.<br>

The server uses a multi-threaded approach to handle multiple clients simultaneously, providing efficient and scalable file management and synchronization services.<br>

## Features
- Scan connected USB devices<br>
- Synchronize files and directories between USB devices<br>
- Create new files or directories in the USB devices<br>
- Handle multiple clients simultaneously<br>
- Use fget executable to run each command by establishing a connection thread to the server<br>
- Use a high-level client to keep the connection and send as many as commands before manually exit<br>

## Usage
Compiling: make<br>
Cleaning all executables and unused files: make clean<br>

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
* GET folder/foo.txt data/localfoo.txt: Get the remote file folder/foo.txt and save it as data/localfoo.txt on the local file system
* INFO folder/foo.txt: Retrieve information about the remote file folder/foo.txt, such as ownership, date of last modification, permissions, and size
* MD folder/newfolder: Create a new directory folder/newfolder in the remote file system
* PUT local/afile.c folder/gfile.c: Upload the local file local/afile.c to the remote file system as folder/gfile.c
* RM folder/somefile.txt: Delete the file or directory folder/somefile.txt in the remote file system

To find the product ID and vendor ID of your USB device, you can use the following steps:<br>
Open a terminal on your computer.<br>
Type the command lsusb and press Enter. This will display a list of all USB devices connected to your computer.<br>
Find the entry for the USB device you are interested in. The entry will look something like this: Bus 002 Device 008: ID 0781:5567 SanDisk Corp. Cruzer Blade<br>
Take note of the numbers after "ID". The first number is the vendor ID, and the second number is the product ID. In this example, the vendor ID is 0781 and the product ID is 5567.<br>


### License
This project is licensed under the MIT License. See the LICENSE file for more information.
