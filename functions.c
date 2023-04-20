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
#include <stdio.h>
#include <stdlib.h>
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

// char unique_files[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
// int unique_files_count;

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
    int status;
    char *dir = strtok(directory, "/");
    char curr_dir[256] = "";
    while (dir != NULL)
    {
        strcat(curr_dir, dir);
        if (access(curr_dir, F_OK) == -1)
        {
            status = mkdir(curr_dir, 0777);
            if (usb != NULL)
            {
                add_dirpath_to_usb(usb, curr_dir);
            }
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
        return 1;
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
        return 1;
    }

    // strtok to handle multiple directories
    char *dir = strtok((char *)path, "/");
    int status;
    char curr_dir[256] = "";
    while (dir != NULL)
    {
        strcat(curr_dir, dir);
        // 0777 sets the read, write, and execute permissions for the owner, and read and execute permissions for the group and others.
        if (access(curr_dir, F_OK) == -1)
        {
            status = mkdir(curr_dir, 0777);
            add_dirpath_to_usb(usb, curr_dir);
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
        return 1;
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

    // Check if the file was found
    int status = pclose(fp);
    if (status == -1)
    {
        printf("Error occurred calling pclose()\n");
        return NULL;
    }
    else if (WIFEXITED(status))
    {
        // Command exited normally
        int exit_status = WEXITSTATUS(status);
        if (exit_status != 0)
        {
            printf("File not found\n");
            return NULL;
        }
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
            return NULL;
        }
        memcpy(output + output_size, buffer, len + 1);
        output_size += len;
    }
    return output;
}

/**
 * Manage the accessibility of the monitor
*/
void enter_available_monitor()
{
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
        return;
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
        return;
    }
    else
    {
        // Seed the random number generator with the current time
        srand(time(NULL));
        // Generate a random number between 0 and 1 using the rand() function
        int random_usb = rand() % 2;
    }
}

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

/**
 * Function to write data to connected USB devices
*/
int write_to_USBs(usb_t *usb1, usb_t *usb2, const char *file_path, const char *file_content)
{
    // Check if both usbs are connected and available
    if (usb1_exist == 0 && usb2_exist == 0)
    {
        // Lock the mutex for the USB1 and USB2 monitor
        pthread_mutex_lock(&pm_mutex_usb1);
        // Wait until the monitors are available before acquiring the lock
        while (!access_available_usb1)
        {
            pthread_cond_wait(&pm_cond_usb1, &pm_mutex_usb1);
        }
        access_available_usb1 = 0;

        pthread_mutex_lock(&pm_mutex_usb2);
        while (!access_available_usb2)
        {
            pthread_cond_wait(&pm_cond_usb2, &pm_mutex_usb2);
        }
        // Set the access_available flag to 0 for both USBs
        access_available_usb2 = 0;
    }

    // Check the connection status of both USB devices
    check_USB_connections(usb1, usb2);
    // If neither USB device is connected, return an error
    if (usb1_exist == -1 && usb2_exist == -1)
    {
        perror("No USB connected.\n");
        return -1;
    }

    if (usb1_exist == 0 && usb1) // USB1 is connected and instructed to write
    {
        char usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);

        if (write_string_to_file(usb1, usb1_file_path, file_content) == 0)
        {
            add_filepath_to_usb(usb1, file_path);
        }
        else
        {
            perror("Error writing file\n");
            return -1;
        }
    }

    if (usb2_exist == 0 && usb2) // USB2 is connected and instructed to write
    {
        char usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        if (write_string_to_file(usb2, usb2_file_path, file_content) == 0)
        {
            add_filepath_to_usb(usb2, file_path);
        }
        else
        {
            perror("Error writing file\n");
            return -1;
        }
    }
    // release the mutex and signal the condition for both USBs
    access_available_usb1 = 1;
    access_available_usb2 = 1;
    pthread_cond_signal(&pm_cond_usb1);
    pthread_cond_signal(&pm_cond_usb2);
    pthread_mutex_unlock(&pm_mutex_usb1);
    pthread_mutex_unlock(&pm_mutex_usb2);
    return 0;
}

/**
 * Remove a filepath from the USB
*/
int remove_filepath_from_usb(usb_t *usb, const char *file_path)
{
    // Iterate through the file_paths array
    int i, j;
    for (i = 0; i < usb->file_count; i++)
    {
        // If the file path is found
        if (strcmp(usb->file_paths[i], file_path) == 0)
        {
            // Shift the remaining file paths one position to the left
            for (j = i; j < (usb->file_count - 1); j++)
            {
                strcpy(usb->file_paths[j], usb->file_paths[j + 1]);
            }
            // Clear the last file path in the array
            memset(usb->file_paths[usb->file_count - 1], '\0', MAX_FILE_PATH_LENGTH);
            // update file count
            usb->file_count--;
            break;
        }
    }
    return 0;
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

/**
 * Function to add a directory path to a USB device's dir_paths array
*/
int add_dirpath_to_usb(usb_t *usb, const char *dir_path)
{
    // Copy the given directory path to the next available index in the dir_paths array
    strcpy(usb->dir_paths[usb->dir_count++], dir_path);
    return 0;
}

/**
 * Function to remove a directory path from a USB device's dir_paths array
*/
int remove_dirpath_from_usb(usb_t *usb, const char *dir_path)
{
    // Iterate through the dir_paths array
    int i, j;
    for (i = 0; i < usb->dir_count; i++)
    {
        // If the directory path is found
        if (strcmp(usb->dir_paths[i], dir_path) == 0)
        {
            // Shift the remaining directory paths one position to the left
            for (j = i; j < (usb->dir_count - 1); j++)
            {
                strcpy(usb->dir_paths[j], usb->dir_paths[j + 1]);
            }
            // Clear the last directory path in the array
            memset(usb->dir_paths[usb->dir_count - 1], '\0', MAX_FILE_PATH_LENGTH);
            // Update the directory count
            usb->dir_count--;
            break;
        }
    }
    return 0;
}

// Function to remove a file from both USB devices
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
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);
        overall_res += remove_file(usb1_file_path);
        // Remove the file path from USB1's file_paths array
        remove_filepath_from_usb(usb1, file_path);
    }
    if (usb2_exist == 0)
    {
        // Remove the file from the USB2 device
        char usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        overall_res += remove_file(usb2_file_path);

        // Remove the file path from USB2's file_paths array
        remove_filepath_from_usb(usb2, file_path);
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

    // Open the specified directory
    dir = opendir(path);
    if (dir == NULL)
    {
        printf("%s is not connected.", usb->mount_path);
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

                strncpy(usb->file_paths[usb->file_count], substr(sub_path, MOUNT_PATH_LENGTH, strlen(sub_path) - MOUNT_PATH_LENGTH), MAX_FILE_PATH_LENGTH);
                usb->file_count++;
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

                strncpy(usb->file_paths[usb->file_count], substr(file_path, MOUNT_PATH_LENGTH, strlen(file_path) - MOUNT_PATH_LENGTH), MAX_FILE_PATH_LENGTH);
                usb->file_count++;
            }
        }
    }
    closedir(dir);
}

// Scans the given USB device and lists all files within it
void scan_usb(usb_t *usb)
{
    char *path = substr(usb->mount_path, 0, strlen(usb->mount_path) - 1); // directory path to list files from
    list_files(usb, path);
}

// Compares file paths in two USBs and finds unique files and directories
void get_unique_paths(usb_t *usb1, usb_t *usb2, char unique_files[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH], int *unique_files_count, char unique_dirs[MAX_DIR_COUNT][MAX_DIR_PATH_LENGTH], int *unique_dirs_count)
{
    *unique_files_count = 0;

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
            strcpy(unique_files[*unique_files_count], usb1->file_paths[i]);
            (*unique_files_count)++;
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
            strcpy(unique_files[*unique_files_count], usb2->file_paths[i]);
            (*unique_files_count)++;
        }
    }
}

/**
 * Sychronize the USBs
*/
int synchronize(usb_t *usb1, usb_t *usb2, char unique_files[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH], int *unique_files_count, char unique_dirs[MAX_DIR_COUNT][MAX_DIR_PATH_LENGTH], int *unique_dirs_count)
{
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

    // Scan the contents of USB1 and USB2.
    scan_usb(usb1);
    scan_usb(usb2);

    // Declare variables for file and directory synchronization.
    char file_path[MAX_FILE_PATH_LENGTH];
    char *file_content;
    int usb1_file_count_before_sync = usb1->file_count;
    int usb2_file_count_before_sync = usb2->file_count;
    int synchronized = 0;

    // Get the unique paths from USB1 and USB2 that need to be synchronized.
    get_unique_paths(usb1, usb2, unique_files, unique_files_count, unique_dirs, unique_dirs_count);
    printf("%d unique paths to be synchronized\n", *unique_files_count);

    // Iterate through the unique files and synchronize them.
    for (int i = 0; i < *unique_files_count; i++)
    {
        synchronized = 0; // mark the file or directory synchronized
        memset(file_path, MAX_FILE_PATH_LENGTH, '\0');
        strcpy(file_path, unique_files[i]);
        printf("unique file path %s\n", unique_files[i]);

        // Synchronize files from USB1 to USB2.
        for (int j = 0; j < usb1_file_count_before_sync; j++)
        {
            if (strcmp(usb1->file_paths[j], file_path) == 0)
            {
                printf("Synchronizing %s to %s\n", file_path, usb2->mount_path);
                char usb1_file_path[MAX_FILE_PATH_LENGTH];
                sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);

                // If the file path is a file, read its content and write it to USB2.
                if (strchr(file_path, '.') != NULL)
                {
                    file_content = read_file_to_string(usb1_file_path);
                    write_to_USBs(NULL, usb2, file_path, file_content);
                }
                // If the file path is a directory, create the directory in USB2.
                else
                {
                    create_dir_in_USBs(file_path, NULL, usb2);
                }
            }
        }

        // Synchronize files from USB2 to USB1.
        for (int j = 0; j < usb2_file_count_before_sync; j++)
        {
            if (strcmp(usb2->file_paths[j], file_path) == 0)
            {
                printf("Synchronizing %s to %s\n", file_path, usb1->mount_path);
                char usb2_file_path[MAX_FILE_PATH_LENGTH];
                sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);

                // If the file path is a file, read its content and write it to USB1.
                if (strchr(file_path, '.') != NULL)
                {
                    file_content = read_file_to_string(usb2_file_path);
                    write_to_USBs(usb1, NULL, file_path, file_content);
                }
                // If the file path is a directory, create the directory in USB1.
                else
                {
                    create_dir_in_USBs(file_path, usb1, NULL);
                }
            }
        }
    }   

    // Clear unique files list after synchronization.
    for (int i = 0; i < MAX_FILE_COUNT; i++)
    {
        memset(unique_files[i], '\0', MAX_FILE_PATH_LENGTH);
    }
    *unique_files_count = 0;

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
    check_USB_connections(usb1, usb2);
    char *file_content = NULL;
    // TODO - add monitor
    if (usb1_exist == 0)
    {
        // Lock the mutex for the USB1 monitor
        pthread_mutex_lock(&pm_mutex_usb1);
        // Wait until the monitors are available before acquiring the lock
        while (!access_available_usb1)
        {
            pthread_cond_wait(&pm_cond_usb1, &pm_mutex_usb1);
        }
        access_available_usb1 = 0;
        
        char usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);
        file_content = read_file_to_string(usb1_file_path);

        // release the mutex
        access_available_usb1 = 1;
        pthread_cond_signal(&pm_cond_usb1);
        pthread_mutex_unlock(&pm_mutex_usb1);
    }
    else if (usb2_exist == 0)
    {
        // Lock the mutex for the USB2 monitor
        pthread_mutex_lock(&pm_mutex_usb2);
        while (!access_available_usb2)
        {
            pthread_cond_wait(&pm_cond_usb2, &pm_mutex_usb2);
        }
        // Set the access_available flag to 0 for both
        access_available_usb2 = 0;

        char usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        file_content = read_file_to_string(usb2_file_path);

        // release the mutex
        access_available_usb2 = 1;
        pthread_cond_signal(&pm_cond_usb2);
        pthread_mutex_unlock(&pm_mutex_usb2);
    }
    else
    {
        perror("read_from_USBs -> No USB connected");
    }
    return file_content;
}

/**
 * Get info (ls -l) from USB(s) connected
 */
char *get_info_from_USBs(const char *file_path, usb_t *usb1, usb_t *usb2)
{
    check_USB_connections(usb1, usb2);
    char *file_info = NULL;

    // TODO - add monitor
    if (usb1_exist == 0)
    {
        // Lock the mutex for the USB1 monitor
        pthread_mutex_lock(&pm_mutex_usb1);
        // Wait until the monitors are available before acquiring the lock
        while (!access_available_usb1)
        {
            pthread_cond_wait(&pm_cond_usb1, &pm_mutex_usb1);
        }
        access_available_usb1 = 0;

        char usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);
        file_info = get_info(usb1_file_path);

        // release the mutex
        access_available_usb1 = 1;
        pthread_cond_signal(&pm_cond_usb1);
        pthread_mutex_unlock(&pm_mutex_usb1);
    }
    else if (usb2_exist == 0)
    {
        // Lock the mutex for the USB2 monitor
        pthread_mutex_lock(&pm_mutex_usb2);
        while (!access_available_usb2)
        {
            pthread_cond_wait(&pm_cond_usb2, &pm_mutex_usb2);
        }
        // Set the access_available_usb2 flag to 0
        access_available_usb2 = 0;

        char usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        file_info = get_info(usb2_file_path);

        // release the mutex
        access_available_usb2 = 1;
        pthread_cond_signal(&pm_cond_usb2);
        pthread_mutex_unlock(&pm_mutex_usb2);
    }
    else
    {
        perror("get_info_from_USBs -> No USB connected");
    }
    return file_info;
}

/**
 * Create identical directories for USB devices
 */
int create_dir_in_USBs(const char *file_path, usb_t *usb1, usb_t *usb2)
{
    check_USB_connections(usb1, usb2);
    if (usb1_exist == -1 && usb2_exist == -1)
    {
        perror("No USB connected.\n");
        return -1;
    }
    // TODO - monitor
    int usb1_result, usb2_result; // identifiers
    if (usb1_exist == 0 && usb1)
    {
        // Lock the mutex for the USB1 monitor
        pthread_mutex_lock(&pm_mutex_usb1);
        // Wait until the monitors are available before acquiring the lock
        while (!access_available_usb1)
        {
            pthread_cond_wait(&pm_cond_usb1, &pm_mutex_usb1);
        }
        access_available_usb1 = 0;

        char usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);
        usb1_result = create_directory(usb1, usb1_file_path);

        // release the mutex
        access_available_usb1 = 1;
        pthread_cond_signal(&pm_cond_usb1);
        pthread_mutex_unlock(&pm_mutex_usb1);
    }

    if (usb2_exist == 0 && usb2)
    {
        // Lock the mutex for the USB2 monitor
        pthread_mutex_lock(&pm_mutex_usb2);
        while (!access_available_usb2)
        {
            pthread_cond_wait(&pm_cond_usb2, &pm_mutex_usb2);
        }
        // Set the access_available flag to 0
        access_available_usb2 = 0;

        char usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        usb2_result = create_directory(usb2, usb2_file_path);

        // release the mutex
        access_available_usb2 = 1;
        pthread_cond_signal(&pm_cond_usb2);
        pthread_mutex_unlock(&pm_mutex_usb2);
    }

    if (usb1_result != 0 && usb2_result != 0)
    {
        perror("create_dir_in_USBs -> failed dir creation");
        return 1;
    }
    return 0;
}


// int main()
// {
//     // Construct two usb_t for usb1 and usb2
//     usb_t usb1 = create_USB_struct(USB1_MOUNT_PATH);
//     usb_t usb2 = create_USB_struct(USB2_MOUNT_PATH);

//     for (int i = 0; i < MAX_FILE_COUNT; i++)
//     {
//         memset(unique_files[i], '\0', MAX_FILE_PATH_LENGTH);
//     }

// Write to USBs
//--------- TEST 1 ---------
// printf("Test 1\n");
// write_to_USBs(&usb1, &usb2, "abc.txt", read_file_to_string("a.txt"));
// printf("written file abc.txt\n");
// printf("%d %d\n", usb1.file_count, usb2.file_count);

//--------- TEST 2 ---------
// printf("next step to remove the file added in Test 1\n");
// remove_file_from_USBs(&usb1, &usb2, "abc.txt");
// printf("%d %d\n", usb1.file_count, usb2.file_count);
// printf("after removing file\n");

// --------- TEST XXX ---------
// printf("Test XXX Test get_unique_paths()\n");

// --------- TEST 3 ---------
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

// --------- TEST 4 ---------
//     printf("Test 4 Add a new file to a new folder\n");
//     write_to_USBs(&usb1, &usb2, "folder1/test_log.txt", read_file_to_string("log.txt"));
//     printf("Writing file folder1/test_log.txt\n");
//     printf("usb1.file_count, usb2.file_count: %d %d\n", usb1.file_count, usb2.file_count);

//     write_to_USBs(&usb1, &usb2, "folder2/folder3/test_log.txt", read_file_to_string("log.txt"));
//     printf("Writing file folder2/folder3/test_log.txt\n");
//     printf("usb1.file_count, usb2.file_count: %d %d\n", usb1.file_count, usb2.file_count);

//     return 0;
// }
