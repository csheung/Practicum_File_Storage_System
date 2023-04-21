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
    char *cmd_str = (char*)arg;

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
    
    // Commands to test the five functionalities
    // commands[] = {
    //     "GET source_path destination_path",
    //     "MD new_directory_path",
    //     "INFO file_path",
    //     "PUT source_path destination_path",
    //     "RM file_path"
    // };

    // declare threads for subsequent tests
    int i;
    int thread_count = 5;
    pthread_t threads[thread_count];

    // declare testing string commands
    int cmd_size = 256;
    char c0[cmd_size], c1[cmd_size], c2[cmd_size], c3[cmd_size], c4[cmd_size];
    char *commands[] = {c0, c1, c2, c3, c4}; // put defined commands into an array
    int result; // pthread_create return value


    // Test 3 MD -- MD command for all threads
    printf("\n--------- TEST 1 ---------\n");
    printf("Test multi-threading MD commands...\n");

    // define all MD commands for testing
    strcpy(c0, "MD folder1/folder7");
    strcpy(c1, "MD folder1/folder8");
    strcpy(c2, "MD folder1/folder9");
    strcpy(c3, "MD folder2/folder9/folder10");
    strcpy(c4, "MD folder2/folder11");

    // call pthread_create on MD commands
    pthread_create(&threads[0], NULL, run_fget, (void *)c0);
    pthread_create(&threads[1], NULL, run_fget, (void *)c1);
    pthread_create(&threads[2], NULL, run_fget, (void *)c2);
    pthread_create(&threads[3], NULL, run_fget, (void *)c3);
    pthread_create(&threads[4], NULL, run_fget, (void *)c4);

    for (i = 0; i < thread_count; i++) {
        result = pthread_join(threads[i], NULL);
        if (result)
        {
            printf("Error joining thread %d\n", i);
            exit(-1);
        }
    }


    // Test 2 PUT -- PUT command for all threads
    printf("--------- TEST 2 ---------\n");
    printf("Test multi-threading PUT commands...\n");

    // define all PUT commands for testing
    strcpy(c0, "PUT log.txt folder1/temp.txt");
    strcpy(c1, "PUT log.txt folder2/folder3/test_log.txt");
    strcpy(c2, "PUT log.txt tmpa.txt");
    strcpy(c3, "PUT log.txt tmpb.txt");
    strcpy(c4, "PUT log.txt tmpc.txt");

    // call pthread_create on PUT commands
    pthread_create(&threads[0], NULL, run_fget, (void *)c0);
    pthread_create(&threads[1], NULL, run_fget, (void *)c1);
    pthread_create(&threads[2], NULL, run_fget, (void *)c2);
    pthread_create(&threads[3], NULL, run_fget, (void *)c3);
    pthread_create(&threads[4], NULL, run_fget, (void *)c4);

    for (i = 0; i < thread_count; i++) {
        result = pthread_join(threads[i], NULL);
        if (result)
        {
            printf("Error joining thread %d\n", i);
            exit(-1);
        }
    }

    printf("\n--------------------------------------------------\n");

    // Test 3 multiple GET commands
    printf("--------- TEST 3 ---------\n");
    printf("Test multi-threading GET commands...\n");

    // define all GET commands for testing
    strcpy(c0, "GET folder1/test_log.txt local/root1.txt");
    strcpy(c1, "GET folder1/test_log.txt local/root2.txt");
    strcpy(c2, "GET folder2/folder3/test_log.txt local/root3.txt");
    strcpy(c3, "GET folder2/folder3/test_log.txt local/root4.txt");
    strcpy(c4, "GET folder2/folder3/test_log.txt local/root5.txt");

    // call pthread_create on GET commands
    pthread_create(&threads[0], NULL, run_fget, (void *)c0);
    pthread_create(&threads[1], NULL, run_fget, (void *)c1);
    pthread_create(&threads[2], NULL, run_fget, (void *)c2);
    pthread_create(&threads[3], NULL, run_fget, (void *)c3);
    pthread_create(&threads[4], NULL, run_fget, (void *)c4);

    for (i = 0; i < thread_count; i++) {
        result = pthread_join(threads[i], NULL);
        if (result)
        {
            printf("Error joining thread %d\n", i);
            exit(-1);
        }
    }

    printf("\n--------------------------------------------------\n");

    // Test 4 INFO -- INFO command for all threads
    printf("--------- TEST 4 ---------\n");
    printf("Test multi-threading INFO commands...\n");

    // define all INFO commands for testing
    strcpy(c0, "INFO folder1/test_log.txt");
    strcpy(c1, "INFO folder2/folder3/test_log.txt");
    strcpy(c2, "INFO folder1/test_log.txt");
    strcpy(c3, "INFO folder2/folder3/test_log.txt");
    strcpy(c4, "INFO folder1/test_log.txt");

    // call pthread_create on INFO commands
    pthread_create(&threads[0], NULL, run_fget, (void *)c0);
    pthread_create(&threads[1], NULL, run_fget, (void *)c1);
    pthread_create(&threads[2], NULL, run_fget, (void *)c2);
    pthread_create(&threads[3], NULL, run_fget, (void *)c3);
    pthread_create(&threads[4], NULL, run_fget, (void *)c4);

    for (i = 0; i < thread_count; i++) {
        result = pthread_join(threads[i], NULL);
        if (result)
        {
            printf("Error joining thread %d\n", i);
            exit(-1);
        }
    }

    printf("\n--------------------------------------------------\n");

    // Test 5 RM -- RM command for all threads
    printf("--------- TEST 5 ---------\n");
    printf("Test multi-threading RM commands...\n");

    // define all RM commands for testing
    strcpy(c0, "RM folder1/tmp.txt");
    strcpy(c1, "RM tmp.txt");
    strcpy(c2, "RM tmpa.txt");
    strcpy(c3, "RM tmpb.txt");
    strcpy(c4, "RM tmpc.txt");

    // call pthread_create on RM commands
    pthread_create(&threads[0], NULL, run_fget, (void *)c0);
    pthread_create(&threads[1], NULL, run_fget, (void *)c1);
    pthread_create(&threads[2], NULL, run_fget, (void *)c2);
    pthread_create(&threads[3], NULL, run_fget, (void *)c3);
    pthread_create(&threads[4], NULL, run_fget, (void *)c4);

    for (i = 0; i < thread_count; i++) {
        result = pthread_join(threads[i], NULL);
        if (result)
        {
            printf("Error joining thread %d\n", i);
            exit(-1);
        }
    }

    printf("\n--------------------------------------------------\n");


    // Test 6 - Question 8 Parallel Read
    printf("--------- TEST 6 ---------\n");
    printf("Test Parallel Read...\n");

    // define all GET commands with read_file_from_USBs for testing
    strcpy(c0, "GET folder1/test_log.txt local/root.txt");
    strcpy(c1, "GET folder1/test_log.txt local/root.txt");
    strcpy(c2, "GET folder2/folder3/test_log.txt local/root.txt");
    strcpy(c3, "GET folder2/folder3/test_log.txt local/root.txt");
    strcpy(c4, "GET folder2/folder3/test_log.txt local/root.txt");

    // call pthread_create on GET commands
    pthread_create(&threads[0], NULL, run_fget, (void *)c0);
    pthread_create(&threads[1], NULL, run_fget, (void *)c1);
    pthread_create(&threads[2], NULL, run_fget, (void *)c2);
    pthread_create(&threads[3], NULL, run_fget, (void *)c3);
    pthread_create(&threads[4], NULL, run_fget, (void *)c4);

    for (i = 0; i < thread_count; i++) {
        result = pthread_join(threads[i], NULL);
        if (result)
        {
            printf("Error joining thread %d\n", i);
            exit(-1);
        }
    }

    printf("\n--------------------------------------------------\n");

    return 0;
}