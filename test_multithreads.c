/*
 * File Name: test_multithreads.c
 * Assignment Title: Practicum II - File Storage System
 *
 * CS5600 Computer Systems / Northeastern University
 * Spring 2023 / Apr 11, 2023
 * Created by Chun Sheung Ng (Derrick) & Zhenyu Wang (Sean)
 *
 */

#include "server_file_manager.c"

int main() {

    // Construct two usb_t for usb1 and usb2
    usb_t usb1 = create_USB_struct(USB1_MOUNT_PATH);
    usb_t usb2 = create_USB_struct(USB2_MOUNT_PATH);



    return 0;
}