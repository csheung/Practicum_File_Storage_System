/*
 * File Name: functions.h
 * Assignment Title: Practicum II - File Storage System
 *
 * CS5600 Computer Systems / Northeastern University
 * Spring 2023 / Apr 11, 2023
 * Created by Chun Sheung Ng (Derrick) & Zhenyu Wang (Sean)
 *
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <libgen.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define MAX_FILE_PATH_LENGTH 256
#define MAX_FILE_COUNT 100
#define MAX_DIR_PATH_LENGTH 256
#define MAX_DIR_COUNT 100
#define use_physical_device 1

typedef struct
{
    int file_count;
    char mount_path[256];
    char file_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
} usb_t;

typedef struct
{
    int *unique_path_count;
    usb_t *usb1;
    usb_t *usb2;
    char unique_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
} thread_args;

// Read the content of a file into a string
char *read_file_to_string(const char *filename);

// Write a string to a file on the USB
int write_string_to_file(usb_t *usb, char filename[MAX_FILE_PATH_LENGTH], const char *str);

// Check if a path exists
int path_exists(const char *path);

// Create a directory on the USB
int create_directory(usb_t *usb, char path[MAX_FILE_PATH_LENGTH]);

// Remove a file from the system
int remove_file(const char *path);

// Get information about a file
char *get_info(const char *filename);

// Create a USB struct with the given mount path
usb_t create_USB_struct();

// Write a file to both USBs
int write_to_USBs(usb_t *usb1, usb_t *usb2, char file_path[MAX_FILE_PATH_LENGTH], const char *file_content);

// Remove a file path from the USB struct
int remove_filepath_from_usb(usb_t *usb, const char *file_path);

// Add a file path to the USB struct
int add_filepath_to_usb(usb_t *usb, const char *file_path);

// Add a directory path to the USB struct
int add_dirpath_to_usb(usb_t *usb, const char *dir_path);

// Remove a directory path from the USB struct
int remove_dirpath_from_usb(usb_t *usb, const char *dir_path);

// Remove a file from both USBs
int remove_file_from_USBs(usb_t *usb1, usb_t *usb2, const char *file_path);

// Get unique paths from both USBs
void get_unique_paths(usb_t *usb1, usb_t *usb2, char unique_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH], int *unique_path_count);

// Synchronize the content of both USBs
int synchronize(usb_t *usb1, usb_t *usb2, char unique_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH], int *unique_path_count);

// Read a file from both USBs
char *read_from_USBs(const char *file_path, usb_t *usb1, usb_t *usb2);

// Get information about a file from both USBs
char *get_info_from_USBs(const char *file_path, usb_t *usb1, usb_t *usb2);

// Create a directory in both USBs
int create_dir_in_USBs(char file_path[MAX_FILE_PATH_LENGTH], usb_t *usb1, usb_t *usb2);

// Concatenate Info and Content of a file
char *concat_info_content(char *str1, char *str2);

void print_unique_paths(char unique_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH], int unique_path_count);

#endif // FUNCTIONS_H