/*
 * File Name: server_file_manager.c
 * Assignment Title: Practicum II - File Storage System
 *
 * CS5600 Computer Systems / Northeastern University
 * Spring 2023 / Apr 11, 2023
 * Created by Chun Sheung Ng (Derrick) & Zhenyu Wang (Sean)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "functions.c"

#define MAX_FILE_PATH_LENGTH 256
#define MAX_FILE_COUNT 100
// #define USB1_MOUNT_PATH "/Volumes/Sandisk/Practicum2/"
// #define USB2_MOUNT_PATH "/Volumes/usb/Practicum2/"

// for derrick test without USBs
#define USB1_MOUNT_PATH "t1/"
#define USB2_MOUNT_PATH "t2/"

typedef struct
{
    int file_count;
    char mount_path[256];
    char file_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
} usb_t;

// Flags showing whether the USB drive is connected to the machine
int usb1_exist = -1;
int usb2_exist = -1;
char unique_files[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
int unique_files_count;

/**
 * Create a struct for the new USB device
*/
usb_t create_USB_struct(const char* usb_mount_path)
{
    usb_t usb;
    usb.file_count = 0;
    strcpy(usb.mount_path, usb_mount_path);

    // initiate every file path as an EOF
    for (int i = 0; i < MAX_FILE_COUNT; i++) 
    {
        memset(usb.file_paths[i], '\0', MAX_FILE_PATH_LENGTH);
    }
    return usb;
}

// function to update the usb_exist flags
// -1 means not found, 0 means accessible
void check_USB_connections(usb_t *usb1, usb_t *usb2)
{
    usb1_exist = access(usb1->mount_path, F_OK);
    usb2_exist = access(usb2->mount_path, F_OK);
}

int write_to_USBs(usb_t *usb1, usb_t *usb2, const char *file_path, const char *file_content)
{

    check_USB_connections(usb1, usb2);
    if (usb1_exist == -1 && usb2_exist == -1)
    {
        perror("No USB connected.\n");
        return -1;
    }

    if (usb1_exist == 0)
    {

        char usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);
        // printf("write_to_USBs: %s", usb1_file_path);
        if (write_string_to_file(usb1_file_path, file_content) == 0)
        {
            strcpy(usb1->file_paths[usb1->file_count++], file_path);
            printf("Success: Received the content from client and wrote to file %s.\n", usb1_file_path);
        }
        else
        {
            perror("Error writing file\n");
        }
    }

    if (usb2_exist == 0)
    {
        char usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        if (write_string_to_file(usb2_file_path, file_content) == 0)
        {
            strcpy(usb2->file_paths[usb2->file_count++], file_path);
            printf("Success: Received the content from client and wrote to file %s.\n", usb2_file_path);
        }
        else
        {
            perror("Error writing file\n");
        }
    }
    return 0;
}

int remove_file_from_array_helper(usb_t* usb, const char* file_path)
{
    
    int i, j;
    for (i = 0; i < usb->file_count; i++)
    {
        printf("outer loop\n");
        if (strcmp(usb->file_paths[i], file_path) == 0)
        {
            printf("successfully found path\n");
            for (j = i; j < (usb->file_count-1); j++)
            {
                strcpy(usb->file_paths[j], usb->file_paths[j+1]);
            }
            printf("before updating final cell\n");
            memset(usb->file_paths[usb->file_count-1], '\0', MAX_FILE_PATH_LENGTH);
            printf("after updating final cell\n");
            // update file count
            usb->file_count--;
            break;
        }
    }
    return 0;
}

// remove files from both USBs
int remove_files_from_USBs(usb_t *usb1, usb_t *usb2, const char *file_path) 
{
    if (file_path == NULL) {
        printf("file path not exists\n");
        return -1;
    }

    // connect to USB devices
    check_USB_connections(usb1, usb2);

    if (usb1_exist == 0)
    {
        // remove the path from USB device
        printf("before usb1 remove_files_from real device\n");
        char usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);
        remove(usb1_file_path);
        printf("after usb1 removing real device\n");
        // remove from array records
        remove_file_from_array_helper(usb1, file_path);
        printf("after usb1 removing array records\n");
    }
    if (usb2_exist == 0)
    {
        // remove the path from USB device
        printf("before usb2 remove_files_from real device\n");
        char usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        remove(usb2_file_path);
        printf("after usb2 removing real device\n");
        // remove from array records
        remove_file_from_array_helper(usb2, file_path);
        printf("after usb2 removing array records\n");
    }
    return 0;
}

void get_unique_files(usb_t *usb1, usb_t *usb2, char **unique_files, int *unique_files_count)
{
    *unique_files_count = 0;
    printf("inside function of unique file count: %d\n", *unique_files_count);
    for (int i = 0; i < usb1->file_count; i++)
    {
        bool exists = false;
        for (int j = 0; j < usb2->file_count; j++)
        {
            if (strcmp(usb1->file_paths[i], usb2->file_paths[j]) == 0)
            {
                printf("curr usb1[i] path %s\n", usb1->file_paths[i]);
                printf("curr usb2[j] path %s\n", usb2->file_paths[j]);
                exists = true;
                break;
            }
        }
        if (!exists)
        {
            printf("usb1 file path not exists in usb2: %s\n", usb1->file_paths[i]);
            strcpy(unique_files[*unique_files_count], usb1->file_paths[i]);
            (*unique_files_count)++;
        }
    }

    for (int i = 0; i < usb2->file_count; i++)
    {
        bool exists = false;
        for (int j = 0; j < usb1->file_count; j++)
        {
            if (strcmp(usb2->file_paths[i], usb1->file_paths[j]) == 0)
            {
                printf("curr usb2[i] path %s\n", usb2->file_paths[i]);
                printf("curr usb1[j] path %s\n", usb1->file_paths[j]);
                exists = true;
                break;
            }
        }
        if (!exists)
        {
            printf("usb2 file path not exists in usb1: %s\n", usb2->file_paths[i]);
            // printf("unique_files[*unique_files_count] is: %s\n", unique_files[*unique_files_count]);
            printf("unique_files[*unique_files_count] size: %lu \n", sizeof(unique_files[*unique_files_count]));

            memset(unique_files[*unique_files_count], '\0', MAX_FILE_PATH_LENGTH);
            // printf("after memset\n");
            // sprintf(unique_files[*unique_files_count], "%s", usb2->file_paths[i]);
            // printf("unique_files[*unique_files_count] is: %s\n", unique_files[*unique_files_count]);

            strcpy(unique_files[*unique_files_count], "Hello");
            printf("unique_files assigned\n");
            (*unique_files_count)++;
        }
    }
    printf("end of function -> unique file count: %d\n", *unique_files_count);
}

int synchronize(usb_t *usb1, usb_t *usb2, char **unique_files, int *unique_files_count)
{
    char *file_path;
    char *file_content;
    char *usb1_file_path;
    char *usb2_file_path;
    printf("before get unique file\n");
    get_unique_files(usb1, usb2, unique_files, unique_files_count);
    printf("after get unique file\n");
    printf("*****unique file count: %d\n", *unique_files_count);

    for (int i = 0; i < *unique_files_count; i++)
    {
        strcpy(file_path, unique_files[i]);
        // if file in usb1.file_paths, write to usb2
        for (int j = 0; j < usb1->file_count; i++)
        {
            if (strcmp(usb1->file_paths[j], file_path) == 0)
            {
                char usb1_file_path[MAX_FILE_PATH_LENGTH];
                sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);

                file_content = read_file_to_string(usb1_file_path);
                write_to_USBs(usb1, usb2, file_path, file_content);
            }
        }
        // if file in usb2.file_paths, write to usb1
        for (int j = 0; j < usb2->file_count; i++)
        {
            if (strcmp(usb2->file_paths[j], file_path) == 0)
            {
                char usb2_file_path[MAX_FILE_PATH_LENGTH];
                sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);

                file_content = read_file_to_string(usb2_file_path);
                write_to_USBs(usb1, usb2, file_path, file_content);
            }
        }
    }
    return 0;
}

char *read_from_USBs(const char *file_path, usb_t *usb1, usb_t *usb2, char **unique_files, int *unique_files_count)
{
    check_USB_connections(usb1, usb2);
    char *file_content;
    if (usb1_exist == 0 && usb2_exist == 0)
    {
        printf("before synchronize\n");
        synchronize(usb1, usb2, unique_files, unique_files_count);
        printf("after synchronize\n");
    }

    // read 
    if (usb1_exist == 0)
    {
        // need identifiers if using by another thread

        char usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);
        file_content = read_file_to_string(usb1_file_path);
    }
    else if (usb2_exist == 0)
    {
        // need identifiers if using by another thread
        
        char usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        file_content = read_file_to_string(usb2_file_path);
    }
    else
    {
        perror("No usb connected.");
    }
    return file_content;
}

int main()
{
    // Construct two usb_t for usb1 and usb2
    usb_t usb1 = create_USB_struct(USB1_MOUNT_PATH);
    usb_t usb2 = create_USB_struct(USB2_MOUNT_PATH);

    /* Write to USBs */
    /* --------- TEST 1 ---------*/
    // printf("Test 1\n");
    // write_to_USBs(&usb1, &usb2, "abc.txt", read_file_to_string("a.txt"));
    // printf("written file abc.txt\n");
    // printf("%d %d\n", usb1.file_count, usb2.file_count);

    // printf("next step to remove file\n");
    // remove_files_from_USBs(&usb1, &usb2, "abc.txt");
    // printf("%d %d\n", usb1.file_count, usb2.file_count);
    // printf("after removing file\n");

    /* --------- TEST 2 ---------*/
    printf("Test 2\n");
    write_to_USBs(&usb1, &usb2, "test_log.txt", read_file_to_string("log.txt"));
    printf("written file test_log.txt\n");
    printf("%d %d\n", usb1.file_count, usb2.file_count);

    write_to_USBs(&usb1, &usb2, "test_log1.txt", read_file_to_string("log.txt"));
    printf("written file test_log1.txt\n");
    printf("%d %d\n", usb1.file_count, usb2.file_count);

    printf("before removing\n");
    remove("t1/test_log.txt");
    remove_file_from_array_helper(&usb1, "test_log.txt");
    printf("after removing\n");

    printf("%s\n", read_from_USBs("t1/test_log.txt", &usb1, &usb2, unique_files, &unique_files_count));


// char unique_files[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
// int unique_files_count;
    /* --------- TEST 3 ---------*/
    // printf("Test 2\n");
    // write_to_USBs(&usb1, &usb2, "test_log.txt", read_file_to_string("log.txt"));
    // printf("written file test_log.txt\n");
    // printf("%d %d\n", usb1.file_count, usb2.file_count);



    return 0;
}

// funciton: readFromUsbs(check_USB_connection(); 
//    if usb1&usb2 then synchronize
//    else read from either usb1 or usb2
// -> return content)
