/*
 * File Name: functions.c
 * Assignment Title: Practicum II - File Storage System
 *
 * CS5600 Computer Systems / Northeastern University
 * Spring 2023 / Apr 11, 2023
 * Created by Chun Sheung Ng (Derrick) & Zhenyu Wang (Sean)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <libgen.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "functions.h"

// Declare a monitor to synchronize thread operations on the shared pm_heap,  page_allocation_list, etc.
static pthread_mutex_t pm_mutex_usb1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pm_cond_usb1 = PTHREAD_COND_INITIALIZER;
static int access_available_usb1 = 1; // Initially, a boolean predicate, 1 indicates that the shared data is available to access

// Declare a monitor to synchronize thread operations on the shared pm_heap,  page_allocation_list, etc.
static pthread_mutex_t pm_mutex_usb2 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pm_cond_usb2 = PTHREAD_COND_INITIALIZER;
static int access_available_usb2 = 1; // Initially, a boolean predicate, 1 indicates that the shared data is available to access

// Flags showing whether the USB drive is connected to the machine
int usb1_exist = -1;
int usb2_exist = -1;

/**
 * Returns a substring of the given string
 */
char *substr(const char *str, int start, int length)
{
    char *substr = malloc(length + 1);
    if (substr == NULL)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    strncpy(substr, str + start, length);
    substr[length] = '\0';
    return substr;
}

/**
 * Reads the contents of a file into a string
 */
char *read_file_to_string(const char *filename)
{

    FILE *fp;
    char *str;
    long size;

    fp = fopen(filename, "r"); // open the file in binary mode
    if (!fp)
    {
        printf("Failed to open the file.\n");
        return NULL;
    }

    fseek(fp, 0, SEEK_END); // move the file pointer to the end of the file
    size = ftell(fp);       // get the size of the file
    fseek(fp, 0, SEEK_SET); // move the file pointer back to the beginning of the file

    str = (char *)malloc(size + 1); // allocate memory for the string
    if (!str)
    {
        printf("Failed to allocate memory.\n");
        fclose(fp);
        return NULL;
    }

    fread(str, 1, size, fp); // read the file into the string
    str[size] = '\0';        // null terminate the string

    fclose(fp); // close the file

    return str;
}

/**
 * Writes a string to a file
 */
int write_string_to_file(usb_t *usb, const char *filename, const char *str)
{
    FILE *fp;
    // char filename_copy[256];
    char *directory = dirname((char *)filename);
    char *dir = strtok(directory, "/");
    char curr_dir[256] = "";
    while (dir != NULL)
    {
        strcat(curr_dir, dir);
        if (access(curr_dir, F_OK) == -1)
        {
            mkdir(curr_dir, 0777);
        }
        strcat(curr_dir, "/");
        dir = strtok(NULL, "/");
    }
    // printf("file name %s \n", filename);
    // printf("file name copy %s \n", filename_copy);
    fp = fopen(filename, "w"); // open the file in write mode
    if (!fp)
    {
        printf("Failed to open the file.\n");
        return -1;
    }

    fprintf(fp, "%s", str); // write the string to the file

    fclose(fp); // close the file

    return 0;
}

/**
 * helper function to check if directory exists
 */
int path_exists(const char *path)
{
    if (access(path, F_OK) == 0)
    {
        // Path exists
        return 1;
    }
    else
    {
        // Path doesn't exist
        return 0;
    }
}

/**
 * Creates a directory at the given path
 */
int create_directory(usb_t *usb, const char *path)
{
    // handle error for dir already existed
    if (path_exists(path))
    {
        // printf("Path already exists: %s\n", path);
        return -1;
    }
    char path_copy[MAX_FILE_PATH_LENGTH];
    strcpy(path_copy, path);
    // strtok to handle multiple directories
    char *dir = strtok((char *)path_copy, "/");
    int status;
    char curr_dir[256] = "";
    while (dir != NULL)
    {
        strcat(curr_dir, dir);
        // 0777 sets the read, write, and execute permissions for the owner, and read and execute permissions for the group and others.
        if (access(curr_dir, F_OK) == -1)
        {
            status = mkdir(curr_dir, 0777);
        }
        strcat(curr_dir, "/");
        dir = strtok(NULL, "/");
    }

    if (status == 0)
    {
        printf("Directory created successfully: %s\n", path);
        return 0;
    }
    else
    {
        perror("Error creating directory");
        return -1;
    }
}

/**
 * Removes a file at the given path
 */
int remove_file(const char *path)
{
    int result = remove(path);

    if (result == 0)
    {
        printf("File removed successfully: %s\n", path);
        return 0;
    }
    else
    {
        printf("Error removing file: No such file or directory: %s\n", path);
        return -1;
    }
}

/**
 * Retrieves file information as a formatted string
 */
char *get_info(const char *filename)
{
    char command[1024];
    snprintf(command, sizeof(command), "(echo 'Permissions  Links  Owner  Group  Size  Modified_Time  File_Name' && ls -l %s) | column -t", filename);

    FILE *fp = popen(command, "r");
    if (!fp)
    {
        fprintf(stderr, "Error executing command: %s\n", command);
        return NULL;
    }

    char *output = NULL;
    size_t output_size = 0;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), fp))
    {
        size_t len = strlen(buffer);
        output = realloc(output, output_size + len + 1);
        if (!output)
        {
            fprintf(stderr, "Error allocating memory for output\n");
            pclose(fp);
            return NULL;
        }
        memcpy(output + output_size, buffer, len + 1);
        output_size += len;
    }

    int status = pclose(fp);
    if (status == -1)
    {
        printf("Error occurred calling pclose()\n");
        free(output);
        return NULL;
    }

    return output;
}

/**
 * Manage the accessibility of the monitor
 */
int enter_available_monitor()
{
    // Seed the random number generator with the current time
    srand(time(NULL));
    // Generate a random number between 0 and 1 using the rand() function
    int random_usb = rand() % 2;
    // Check if the monitor for USB1 is available
    if (usb1_exist == 0 && access_available_usb1)
    {
        // Lock the mutex for the USB1 monitor
        pthread_mutex_lock(&pm_mutex_usb1);
        // Wait until the USB1 monitor is available before acquiring the lock
        while (!access_available_usb1)
        {
            pthread_cond_wait(&pm_cond_usb1, &pm_mutex_usb1);
        }
        // Set the access_available flag to indicate that the USB1 monitor is currently being used
        access_available_usb1 = 0;
        return 0;
    }
    // Check if the monitor for USB2 is available
    else if (usb2_exist == 0 && access_available_usb2)
    {
        // Lock the mutex for the USB2 monitor
        pthread_mutex_lock(&pm_mutex_usb2);
        // Wait until the USB2 monitor is available before acquiring the lock
        while (!access_available_usb2)
        {
            pthread_cond_wait(&pm_cond_usb2, &pm_mutex_usb2);
        }
        // Set the access_available flag to indicate that the USB2 monitor is currently being used
        access_available_usb2 = 0;
        return 1;
    }
    else
    {
        if (random_usb == 0)
        {
            // Lock the mutex for the USB1 monitor
            pthread_mutex_lock(&pm_mutex_usb1);
            // Wait until the USB1 monitor is available before acquiring the lock
            while (!access_available_usb1)
            {
                pthread_cond_wait(&pm_cond_usb1, &pm_mutex_usb1);
            }
            // Set the access_available flag to indicate that the USB1 monitor is currently being used
            access_available_usb1 = 0;
            return 0;
        }
        else
        {
            // Lock the mutex for the USB2 monitor
            pthread_mutex_lock(&pm_mutex_usb2);
            // Wait until the USB2 monitor is available before acquiring the lock
            while (!access_available_usb2)
            {
                pthread_cond_wait(&pm_cond_usb2, &pm_mutex_usb2);
            }
            // Set the access_available flag to indicate that the USB2 monitor is currently being used
            access_available_usb2 = 0;
            return 1;
        }
    }
}

/**
 *
 * Release monitor, 0 - release monitor on USB1, 1 - release monitor on USB2
 */
void release_monitor(int monitor)
{
    if (monitor == 0)
    {
        access_available_usb1 = 1;
        pthread_cond_signal(&pm_cond_usb1);
        pthread_mutex_unlock(&pm_mutex_usb1);
    }
    else
    {

        access_available_usb2 = 1;
        pthread_cond_signal(&pm_cond_usb2);
        pthread_mutex_unlock(&pm_mutex_usb2);
    }
}

/**
 * Create a struct for the new USB device
 */
usb_t create_USB_struct()
{
    usb_t usb;
    usb.file_count = 0;
    memset(usb.mount_path, '\0', MAX_FILE_PATH_LENGTH);

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

/**
 * Function to write data to connected USB devices
 */
int write_to_USBs(usb_t *usb1, usb_t *usb2, const char *file_path, const char *file_content)
{
    int monitor_acquired;
    int result = -1;
    // Check the connection status of both USB devices
    check_USB_connections(usb1, usb2);
    // If neither USB device is connected, return an error
    if (usb1_exist == -1 && usb2_exist == -1)
    {
        perror("No USB connected");
        return -1;
    }
    if (usb1 != NULL && usb2 != NULL)
    {
        monitor_acquired = enter_available_monitor();
        if (monitor_acquired == 0)
        {
            char usb1_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb1_file_path, "%s%s", usb1->mount_path, file_path);
            result = write_string_to_file(usb1, usb1_file_path, file_content);
        }
        else
        {
            char usb2_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb2_file_path, "%s%s", usb2->mount_path, file_path);
            result = write_string_to_file(usb2, usb2_file_path, file_content);
        }
    }
    else
    {
        // Monitor acquired already by background thread - synchronize()
        if (usb1_exist == 0 && usb1) // USB1 is connected and instructed to write
        {
            char usb1_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb1_file_path, "%s%s", usb1->mount_path, file_path);

            result = write_string_to_file(usb1, usb1_file_path, file_content);
        }

        if (usb2_exist == 0 && usb2) // USB2 is connected and instructed to write
        {
            char usb2_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb2_file_path, "%s%s", usb2->mount_path, file_path);
            result = write_string_to_file(usb2, usb2_file_path, file_content);
        }
    }
    if (result == -1)
    {
        perror("write_to_USBs error");
    }
    if (usb1 != NULL && usb2 != NULL)
    {
        release_monitor(monitor_acquired);
    }
    return result;
}

/**
 * Function to add a file path to a USB device's file_paths array
 */
int add_filepath_to_usb(usb_t *usb, const char *file_path)
{
    // Copy the given file path to the next available index in the file_paths array
    strcpy(usb->file_paths[usb->file_count++], file_path);
    return 0;
}

// Function to remove a file from both USB devices
// Remove is special. Our current approach to this is that it may remove one copy from a USB, but if synchronizing process is taking off, this removed copy may be recovered.
int remove_file_from_USBs(usb_t *usb1, usb_t *usb2, const char *file_path)
{
    int overall_res = 0;
    // Check if the file path is not NULL
    if (file_path == NULL)
    {
        printf("file path not exists\n");
        return -1;
    }

    // Connect to USB devices
    check_USB_connections(usb1, usb2);

    if (usb1_exist == 0)
    {
        // Remove the path from USB device
        char usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", usb1->mount_path, file_path);
        overall_res += remove_file(usb1_file_path);
    }
    if (usb2_exist == 0)
    {
        // Remove the file from the USB2 device
        char usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", usb2->mount_path, file_path);
        overall_res += remove_file(usb2_file_path);
    }
    // If both remove_file calls are not successful, return failure
    if (overall_res == -2)
    {
        return -1;
    }
    return 0;
}

/**
 * Function to list files in a directory on a USB device
 */
void list_files(usb_t *usb, char *path)
{
    DIR *dir;
    struct dirent *entry;
    char *path_copy;

    // Open the specified directory
    dir = opendir(path);
    if (dir == NULL)
    {
        return;
    }

    // Read the directory entries
    while ((entry = readdir(dir)) != NULL)
    {
        // If the entry is a directory
        if (entry->d_type == DT_DIR)
        {
            // Skip the current directory (.) and parent directory (..)
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            // Construct the subdirectory path
            char sub_path[MAX_FILE_PATH_LENGTH];
            snprintf(sub_path, sizeof(sub_path), "%s/%s", path, entry->d_name);
            // Check if the subdirectory path is already in the file_paths array
            int is_duplicate = 0;
            for (int i = 0; i < usb->file_count; i++)
            {
                if (strcmp(usb->file_paths[i], sub_path) == 0)
                {
                    is_duplicate = 1;
                    break;
                }
            }
            // If the subdirectory path is not a duplicate, add it to the file_paths array
            if (!is_duplicate)
            {
                path_copy = substr(sub_path, MOUNT_PATH_LENGTH, strlen(sub_path) - MOUNT_PATH_LENGTH);
                add_filepath_to_usb(usb, path_copy);

                free(path_copy);
            }
            // Recursively list files in the subdirectory
            list_files(usb, sub_path);
        }
        // If the entry is a file
        else
        {
            // Construct the file path
            char file_path[MAX_FILE_PATH_LENGTH];

            snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);
            // Check if the file path is already in the file_paths array
            int is_duplicate = 0;
            for (int i = 0; i < usb->file_count; i++)
            {
                if (strcmp(usb->file_paths[i], file_path) == 0)
                {
                    is_duplicate = 1;
                    break;
                }
            }
            // If the file path is not a duplicate, add it to the file_paths array
            if (!is_duplicate)
            {
                path_copy = substr(file_path, MOUNT_PATH_LENGTH, strlen(file_path) - MOUNT_PATH_LENGTH);
                add_filepath_to_usb(usb, path_copy);

                free(path_copy);
            }
        }
    }
    closedir(dir);
}

// Scans the given USB device and lists all files within it
void scan_usb(usb_t *usb)
{
    char *path = substr(usb->mount_path, 0, strlen(usb->mount_path) - 1);
    list_files(usb, path);
    free(path);
}

// Compares file paths in two USBs and finds unique files and directories
void get_unique_paths(usb_t *usb1, usb_t *usb2, char unique_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH], int *unique_path_count)
{
    *unique_path_count = 0;

    // Find unique files from usb1 compared to usb2
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
            strcpy(unique_paths[*unique_path_count], usb1->file_paths[i]);
            (*unique_path_count)++;
        }
    }

    // Find unique files from usb2 compared to usb1
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
            strcpy(unique_paths[*unique_path_count], usb2->file_paths[i]);
            (*unique_path_count)++;
        }
    }
}

void clear_paths(usb_t *usb1, usb_t *usb2, char unique_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH], int *unique_path_count)
{
    usb1->file_count = 0;
    usb2->file_count = 0;
    // initiate every file path as an EOF
    for (int i = 0; i < MAX_FILE_COUNT; i++)
    {
        memset(usb1->file_paths[i], '\0', MAX_FILE_PATH_LENGTH);
        memset(usb2->file_paths[i], '\0', MAX_FILE_PATH_LENGTH);
        memset(unique_paths[i], '\0', MAX_FILE_PATH_LENGTH);
        *unique_path_count = 0;
    }
}

/**
 * Sychronize the USBs
 */
int synchronize(usb_t *usb1, usb_t *usb2, char unique_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH], int *unique_path_count)
{
    check_USB_connections(usb1, usb2);
    // Check if both USB devices are connected and available.
    if (usb1_exist == 0 && usb2_exist == 0)
    {
        // Lock the mutex for the USB1 and USB2 monitors.
        pthread_mutex_lock(&pm_mutex_usb1);
        // Wait until the monitors are available before acquiring the lock.
        while (!access_available_usb1)
        {
            pthread_cond_wait(&pm_cond_usb1, &pm_mutex_usb1);
        }
        // Set the access_available flag to 0 for USB1.
        access_available_usb1 = 0;
        // Lock the mutex for the USB2 monitor.
        pthread_mutex_lock(&pm_mutex_usb2);
        // Wait until the monitor is available before acquiring the lock.
        while (!access_available_usb2)
        {
            pthread_cond_wait(&pm_cond_usb2, &pm_mutex_usb2);
        }
        // Set the access_available flag to 0 for USB2.
        access_available_usb2 = 0;
    }
    else
    {
        printf("No two USBs connected, synchronization cancels.\n");
        return -1;
    }
    // Reset stored paths in usb structs from the last scan.
    clear_paths(usb1, usb2, unique_paths, unique_path_count);
    // Scan the contents of USB1 and USB2.
    scan_usb(usb1);
    scan_usb(usb2);

    // Declare variables for file and directory synchronization.
    char file_path[MAX_FILE_PATH_LENGTH];
    char *file_content;
    int synchronized = 0;

    // DEBUG -- Print all scanned files
    // for (int i = 0; i < usb1->file_count; i++)
    // {
    //     printf("usb1 path: %s\n", usb1->file_paths[i]);
    // }
    // for (int i = 0; i < usb2->file_count; i++)
    // {
    //     printf("usb2 path: %s\n", usb2->file_paths[i]);
    // }

    // Get the unique paths from USB1 and USB2 that need to be synchronized.
    get_unique_paths(usb1, usb2, unique_paths, unique_path_count);
    printf("%d unique paths to be synchronized\n", *unique_path_count);

    // Iterate through the unique files and synchronize them.
    for (int i = 0; i < *unique_path_count; i++)
    {
        synchronized = 0; // mark the current file or directory synchronized
        memset(file_path, '\0', MAX_FILE_PATH_LENGTH);
        strcpy(file_path, unique_paths[i]);

        // Synchronize files from USB1 to USB2.
        for (int j = 0; j < usb1->file_count; j++)
        {
            if (strcmp(usb1->file_paths[j], file_path) == 0)
            {
                printf("Synchronizing %s to %s\n", file_path, usb2->mount_path);
                // If the file path is a file, read its content and write it to USB2.
                if (strchr(file_path, '.') != NULL)
                {
                    file_content = read_from_USBs(file_path, usb1, NULL);
                    write_to_USBs(NULL, usb2, file_path, file_content);
                    free(file_content);
                }
                // If the file path is a directory, create the directory in USB2.
                else
                {
                    create_dir_in_USBs(file_path, NULL, usb2);
                }
                synchronized = 1;
                break;
            }
        }
        // If the file path is found in usb1, continue to the next unique file
        if (synchronized == 1)
        {
            continue;
        }
        // Synchronize files from USB2 to USB1.
        for (int j = 0; j < usb2->file_count; j++)
        {
            if (strcmp(usb2->file_paths[j], file_path) == 0)
            {
                printf("Synchronizing %s to %s\n", file_path, usb1->mount_path);

                // If the file path is a file, read its content and write it to USB1.
                if (strchr(file_path, '.') != NULL)
                {
                    file_content = read_from_USBs(file_path, NULL, usb2);
                    write_to_USBs(usb1, NULL, file_path, file_content);
                    free(file_content);
                }
                // If the file path is a directory, create the directory in USB1.
                else
                {
                    create_dir_in_USBs(file_path, usb1, NULL);
                }
                synchronized = 1;
                break;
            }
        }
    }

    // Set the mutexes for USB1 and USB2 as available and signal waiting threads.
    access_available_usb1 = 1;
    access_available_usb2 = 1;
    pthread_cond_signal(&pm_cond_usb1);
    pthread_cond_signal(&pm_cond_usb2);
    // Release the mutex locks after accessing shared data structures.
    pthread_mutex_unlock(&pm_mutex_usb1);
    pthread_mutex_unlock(&pm_mutex_usb2);

    return 0;
}

// Adapted for Q8
/**
 * Read content from USB(s)
 */
char *read_from_USBs(const char *file_path, usb_t *usb1, usb_t *usb2)
{
    int monitor_acquired;
    char *file_content = NULL;
    check_USB_connections(usb1, usb2);
    if (usb1_exist == -1 && usb2_exist == -1)
    {
        perror("No USB connected");
        return NULL;
    }
    // TODO - add monitor
    if (usb1 != NULL && usb2 != NULL)
    {
        monitor_acquired = enter_available_monitor();
        if (monitor_acquired == 0)
        {
            char usb1_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb1_file_path, "%s%s", usb1->mount_path, file_path);
            file_content = read_file_to_string(usb1_file_path);
        }
        else
        {
            char usb2_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb2_file_path, "%s%s", usb2->mount_path, file_path);
            file_content = read_file_to_string(usb2_file_path);
        }
    }
    else
    {
        // Monitor acquired already by background thread - synchronize()
        if (usb1_exist == 0 && usb1)
        {
            char usb1_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb1_file_path, "%s%s", usb1->mount_path, file_path);
            file_content = read_file_to_string(usb1_file_path);
        }
        else if (usb2_exist == 0 && usb2)
        {
            char usb2_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb2_file_path, "%s%s", usb2->mount_path, file_path);
            file_content = read_file_to_string(usb2_file_path);
        }
    }

    if (file_content == NULL)
    {
        perror("read_from_USBs error");
    }
    if (usb1 != NULL && usb2 != NULL)
    {
        release_monitor(monitor_acquired);
    }

    return file_content;
}

/**
 * Get info (ls -l) from USB(s) connected
 */
char *get_info_from_USBs(const char *file_path, usb_t *usb1, usb_t *usb2)
{
    char *file_info = NULL;
    char *file_content = NULL;
    int monitor_acquired;
    check_USB_connections(usb1, usb2);
    if (usb1_exist == -1 && usb2_exist == -1)
    {
        perror("No USB connected");
        return -1;
    }
    // TODO - add monitor
    if (usb1 != NULL && usb2 != NULL)
    {
        monitor_acquired = enter_available_monitor();
        if (monitor_acquired == 0)
        {
            char usb1_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb1_file_path, "%s%s", usb1->mount_path, file_path);
            file_info = get_info(usb1_file_path);
            file_content = read_file_to_string(usb1_file_path);
        }
        else
        {
            char usb2_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb2_file_path, "%s%s", usb2->mount_path, file_path);
            file_info = get_info(usb2_file_path);
            file_content = read_file_to_string(usb2_file_path);
        }
    }
    else
    {
        // Monitor acquired already by background thread - synchronize()
        if (usb1_exist == 0 && usb1)
        {
            char usb1_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb1_file_path, "%s%s", usb1->mount_path, file_path);
            file_info = get_info(usb1_file_path);
            file_content = read_file_to_string(usb1_file_path);
        }
        else if (usb2_exist == 0 && usb2)
        {
            char usb2_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb2_file_path, "%s%s", usb2->mount_path, file_path);
            file_content = read_file_to_string(usb2_file_path);
            file_info = get_info(usb2_file_path);
        }
    }
    if (file_info == NULL)
    {
        perror("get_info_from_USBs error");
    }
    if (usb1 != NULL && usb2 != NULL)
    {
        release_monitor(monitor_acquired);
    }
    if (file_info == NULL)
    {
        perror("get_info_from_USBs error");
        return NULL;
    }
    else
    {
        char *result = concat_info_content(file_info, file_content);
        free(file_info);
        free(file_content);
        return result;
    }
}

/**
 * Create identical directories for USB devices, 0 success, -1 failure
 */
int create_dir_in_USBs(const char *file_path, usb_t *usb1, usb_t *usb2)
{
    int monitor_acquired;
    int result = -1;
    check_USB_connections(usb1, usb2);
    if (usb1_exist == -1 && usb2_exist == -1)
    {
        perror("No USB connected");
        return -1;
    }

    // TODO - enter just one monitor for client request
    if (usb1 != NULL && usb2 != NULL)
    {
        monitor_acquired = enter_available_monitor();
        if (monitor_acquired == 0)
        {
            char usb1_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb1_file_path, "%s%s", usb1->mount_path, file_path);
            result = create_directory(usb1, usb1_file_path);
        }
        else
        {
            char usb2_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb2_file_path, "%s%s", usb2->mount_path, file_path);
            result = create_directory(usb2, usb2_file_path);
        }
    }
    else
    { // Monitor acquired already by background thread - synchronize()
        if (usb1_exist == 0 && usb1)
        {
            char usb1_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb1_file_path, "%s%s", usb1->mount_path, file_path);
            result = create_directory(usb1, usb1_file_path);
        }
        if (usb2_exist == 0 && usb2)
        {
            char usb2_file_path[MAX_FILE_PATH_LENGTH];
            sprintf(usb2_file_path, "%s%s", usb2->mount_path, file_path);
            result = create_directory(usb2, usb2_file_path);
        }
    }

    if (result == -1)
    {
        perror("create_dir_in_USBs error");
    }
    if (usb1 != NULL && usb2 != NULL)
    {
        release_monitor(monitor_acquired);
    }
    return result;
}

void read_config_file(const char *filename, char usb1_mount_path[MAX_FILE_PATH_LENGTH], char usb2_mount_path[MAX_FILE_PATH_LENGTH])
{
    FILE *file = fopen(filename, "r");

    if (!file)
    {
        fprintf(stderr, "Failed to open file %s\n", filename);
        exit(1);
    }

    char buffer[256];
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1)
    {
        strncpy(buffer, line, sizeof(buffer));

        char key[256];
        char value[256];

        if (sscanf(buffer, "%[^=]=%s", key, value) == 2)
        {
            if (strcmp(key, "USB1_MOUNT_PATH") == 0)
            {
                strcpy(usb1_mount_path, value);
            }
            else if (strcmp(key, "USB2_MOUNT_PATH") == 0)
            {
                strcpy(usb2_mount_path, value);
            }
            else
            {
                fprintf(stderr, "Unknown key: %s\n", key);
            }
        }
    }

    fclose(file);
    if (line)
    {
        free(line);
    }
}

char *concat_info_content(char *str1, char *str2)
{
    // Get the length of the concatenated string
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    size_t len3 = len1 + len2 + 2; // Add space for "$$"

    // Allocate memory for the concatenated string
    char *result = malloc(len3);

    // Copy the first string to the result buffer
    strcpy(result, str1);

    // Append the second string with "$$" to the result buffer
    strcat(result, "$$");
    strcat(result, str2);

    return result;
}