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
// char unique_files[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
// int unique_files_count;


/**
 * Create a struct for the new USB device
 */
usb_t create_USB_struct(const char *usb_mount_path)
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

    if (usb1_exist == 0 && usb1) // USB1 is connected and instructed to write
    {

        char usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);
        // printf("write_to_USBs: %s", usb1_file_path);
        if (write_string_to_file(usb1_file_path, file_content) == 0)
        {
            strcpy(usb1->file_paths[usb1->file_count++], file_path);
        }
        else
        {
            perror("Error writing file\n");
        }
    }

    if (usb2_exist == 0 && usb2) // USB2 is connected and instructed to write
    {
        char usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        if (write_string_to_file(usb2_file_path, file_content) == 0)
        {
            strcpy(usb2->file_paths[usb2->file_count++], file_path);
        }
        else
        {
            perror("Error writing file\n");
        }
    }
    return 0;
}

int remove_filepath_from_usb(usb_t *usb, const char *file_path)
{

    int i, j;
    for (i = 0; i < usb->file_count; i++)
    {
        if (strcmp(usb->file_paths[i], file_path) == 0)
        {
            for (j = i; j < (usb->file_count - 1); j++)
            {
                strcpy(usb->file_paths[j], usb->file_paths[j + 1]);
            }
            memset(usb->file_paths[usb->file_count - 1], '\0', MAX_FILE_PATH_LENGTH);
            // update file count
            usb->file_count--;
            break;
        }
    }
    return 0;
}

// remove files from both USBs
int remove_file_from_USBs(usb_t *usb1, usb_t *usb2, const char *file_path)
{
    if (file_path == NULL)
    {
        printf("file path not exists\n");
        return -1;
    }

    // connect to USB devices
    check_USB_connections(usb1, usb2);

    if (usb1_exist == 0)
    {
        // remove the path from USB device
        char usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);
        remove(usb1_file_path);
        // remove from array records
        remove_filepath_from_usb(usb1, file_path);
    }
    if (usb2_exist == 0)
    {
        // remove the path from USB device
        char usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        remove(usb2_file_path);

        // remove from array records
        remove_filepath_from_usb(usb2, file_path);
    }
    return 0;
}

void get_unique_files(usb_t *usb1, usb_t *usb2, char unique_files[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH], int *unique_files_count)
{
    *unique_files_count = 0;

    for (int i = 0; i < usb1->file_count; i++)
    {
        bool exists = false;
        for (int j = 0; j < usb2->file_count; j++)
        {
            if (strcmp(usb1->file_paths[i], usb2->file_paths[j]) == 0)
            {

                exists = true;
                break;
            }
        }
        if (!exists)
        {

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

                exists = true;
                break;
            }
        }
        if (!exists)
        {

            strcpy(unique_files[*unique_files_count], usb2->file_paths[i]);

            (*unique_files_count)++;
        }
    }
}

int synchronize(usb_t *usb1, usb_t *usb2, char unique_files[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH], int *unique_files_count)
{
    char file_path[MAX_FILE_PATH_LENGTH];
    char *file_content;
    char *usb1_file_path;
    char *usb2_file_path;
    get_unique_files(usb1, usb2, unique_files, unique_files_count);

    for (int i = 0; i < *unique_files_count; i++)
    {
        strcpy(file_path, unique_files[i]);

        // if file in usb1.file_paths, write to usb2
        for (int j = 0; j < usb1->file_count; j++)
        {

            if (strcmp(usb1->file_paths[j], file_path) == 0)
            {
                char usb1_file_path[MAX_FILE_PATH_LENGTH];
                sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);

                file_content = read_file_to_string(usb1_file_path);
                write_to_USBs(NULL, usb2, file_path, file_content);
            }
        }
        // if file in usb2.file_paths, write to usb1
        for (int j = 0; j < usb2->file_count; j++)
        {

            if (strcmp(usb2->file_paths[j], file_path) == 0)
            {
                char usb2_file_path[MAX_FILE_PATH_LENGTH];
                sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);

                file_content = read_file_to_string(usb2_file_path);
                write_to_USBs(usb1, NULL, file_path, file_content);
            }
        }
    }
    return 0;
}

char *read_from_USBs(const char *file_path, usb_t *usb1, usb_t *usb2, char unique_files[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH], int *unique_files_count)
{
    check_USB_connections(usb1, usb2);
    char *file_content = NULL;
    if (usb1_exist == 0 && usb2_exist == 0)
    {
        synchronize(usb1, usb2, unique_files, unique_files_count);
    }

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

/*
int main()
{
    // Construct two usb_t for usb1 and usb2
    usb_t usb1 = create_USB_struct(USB1_MOUNT_PATH);
    usb_t usb2 = create_USB_struct(USB2_MOUNT_PATH);

    for (int i = 0; i < MAX_FILE_COUNT; i++)
    {
        memset(unique_files[i], '\0', MAX_FILE_PATH_LENGTH);
    }

    // /* Write to USBs */
    /* --------- TEST 1 ---------*/
    // printf("Test 1\n");
    // write_to_USBs(&usb1, &usb2, "abc.txt", read_file_to_string("a.txt"));
    // printf("written file abc.txt\n");
    // printf("%d %d\n", usb1.file_count, usb2.file_count);

    /* --------- TEST 2 ---------*/
    // printf("next step to remove the file added in Test 1\n");
    // remove_file_from_USBs(&usb1, &usb2, "abc.txt");
    // printf("%d %d\n", usb1.file_count, usb2.file_count);
    // printf("after removing file\n");

    /* --------- TEST XXX ---------*/
    // printf("Test XXX Test get_unique_files()\n");

    /* --------- TEST 3 ---------*/
    // printf("Test 3\n");
    // write_to_USBs(&usb1, &usb2, "test_log.txt", read_file_to_string("log.txt"));
    // printf("Writing file test_log.txt\n");
    // printf("usb1.file_count, usb2.file_count: %d %d\n", usb1.file_count, usb2.file_count);

    // write_to_USBs(&usb1, &usb2, "test_log1.txt", read_file_to_string("log.txt"));
    // printf("Writing file test_log1.txt\n");
    // printf("usb1.file_count, usb2.file_count: %d %d\n", usb1.file_count, usb2.file_count);

    // remove("t1/test_log.txt");
    // remove_filepath_from_usb(&usb1, "test_log.txt");

    // printf("Read test_log.txt after removing it from usb1: %s\n", read_from_USBs("test_log.txt", &usb1, &usb2, unique_files, &unique_files_count));

    /* --------- TEST 4 ---------*/
//     printf("Test 4 Add a new file to a new folder\n");
//     write_to_USBs(&usb1, &usb2, "folder1/test_log.txt", read_file_to_string("log.txt"));
//     printf("Writing file folder1/test_log.txt\n");
//     printf("usb1.file_count, usb2.file_count: %d %d\n", usb1.file_count, usb2.file_count);

//     write_to_USBs(&usb1, &usb2, "folder2/folder3/test_log.txt", read_file_to_string("log.txt"));
//     printf("Writing file folder2/folder3/test_log.txt\n");
//     printf("usb1.file_count, usb2.file_count: %d %d\n", usb1.file_count, usb2.file_count);

//     return 0;
// }
