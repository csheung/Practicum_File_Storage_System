/*
 * File Name: test_multithreads.c
 * Assignment Title: Practicum II - File Storage System
 *
 * CS5600 Computer Systems / Northeastern University
 * Spring 2023 / Apr 11, 2023
 * Created by Chun Sheung Ng (Derrick) & Zhenyu Wang (Sean)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "functions.c"


// variablet multithreads
const int NUM_THREADS = 5;
pthread_t threads[NUM_THREADS];
char *commands[];

// create variables for testing purposes
char unique_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
int unique_path_count = 0;

/**
 * A function to construct/typecast thread_arg for using pthread
 * @param void*     args
 * @return void
 */
void *run_fget(void *arg) {
    char command[256];
    char *cmd_str = (char *)arg;

    // Create the command string to call the "fget" executable with the given argument
    snprintf(command, sizeof(command), "./fget %s", cmd_str);

    // Execute the command using the system() function
    int result = system(command);

    if (result == -1) {
        perror("system");
    } else {
        printf("Command executed successfully: %s\n", cmd_str);
    }
    return NULL;
}

/**
 * Test multiple GET commands
*/
void test_multi_get() {
    // Create and run threads for each command
    for (int i = 0; i < NUM_THREADS; i++) {
        int rc = pthread_create(&threads[i], NULL, run_fget, (void *)commands[i]);
        if (rc) {
            fprintf(stderr, "Error: pthread_create returned code %d\n", rc);
            exit(1);
        }
    }
}

/**
 * Test multiple GET commands
*/
void test_multi_info(char *commands) {
    // Create and run threads for each command
    for (int i = 0; i < NUM_THREADS; i++) {
        int rc = pthread_create(&threads[i], NULL, run_fget, (void *)commands[i]);
        if (rc) {
            fprintf(stderr, "Error: pthread_create returned code %d\n", rc);
            exit(1);
        }
    }
}

/* main */
int main() {

    // print configuration information for our program
    printf("-------------Configuring the heap-------------\n");
    printf("MAX_FILE_PATH_LENGTH: %d\n", MAX_FILE_PATH_LENGTH);
    printf("MAX_FILE_COUNT: %d\n", MAX_FILE_COUNT);

    // Construct two usb_t for usb1 and usb2
    usb_t usb1 = create_USB_struct();
    usb_t usb2 = create_USB_struct();
    read_config_file("USB_config.txt", usb1.mount_path, usb2.mount_path);

    // Construct thread_args for testing
    thread_args bg_thread_args;
    for (int i = 0; i < MAX_FILE_COUNT; i++)
    {
        memset(bg_thread_args.unique_paths[i], '\0', MAX_FILE_PATH_LENGTH);
    }
    bg_thread_args.usb1 = &usb1;
    bg_thread_args.usb2 = &usb2;
    bg_thread_args.unique_path_count = &unique_path_count;


    // --------- TEST 0 ---------
    // Test 
    // printf("--------- TEST 0 ---------\n");


    // --------- TEST 1 ---------
    // Test multiple GET commands
    printf("--------- TEST 1 ---------\n");
    printf("Test multi-threading GET commands...\n");
    // Commands to test the five functionalities
    char *commands[] = {
        "GET source_path destination_path",
        "MD new_directory_path",
        "INFO file_path",
        "PUT source_path destination_path",
        "RM file_path"
    };


    return 0;
}