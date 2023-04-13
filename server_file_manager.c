#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "functions.c"

#define MAX_FILE_PATH_LENGTH 256
#define MAX_FILE_COUNT 100
#define USB1_MOUNT_PATH "/Volumes/Sandisk/Practicum2/"
#define USB2_MOUNT_PATH "/Volumes/usb/Practicum2/"

typedef struct
{
    char mount_path[256];
    char file_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
    int file_count;
} usb_t;

// Flags showing whether the USB drive is connected to the machine
int usb1_exist = -1;
int usb2_exist = -1;
char unique_files[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
int unique_files_count;

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

        char *usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);
        printf("write_to_USBs: %s", usb1_file_path);
        if (write_string_to_file(usb1_file_path, file_content) == 0)
        {
            strcpy(usb1->file_paths[usb1->file_count++], file_path);
            printf("Success: Received the content from client and wrote to file %s.\n", file_path);
            // sprintf(server_message, "SUCCESS$$PUT$$%s\n", args[1]);
        }
        else
        {
            // sprintf(server_message, "ERROR$$PUT_FILE_FAILURE when doing PUT Command.\n");
            perror("Error writing file\n");
        }
    }
    if (usb2_exist == 0)
    {

        char *usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        if (write_string_to_file(file_path, file_content) == 0)
        {
            strcpy(usb2->file_paths[usb2->file_count++], file_path);
            printf("Success: Received the content from client and wrote to file %s.\n", file_path);
            // sprintf(server_message, "SUCCESS$$PUT$$%s\n", args[1]);
        }
        else
        {
            // sprintf(server_message, "ERROR$$PUT_FILE_FAILURE when doing PUT Command.\n");
            perror("Error writing file\n");
        }
    }

    return 0;
}

void get_unique_files(usb_t *usb1, usb_t *usb2, char **unique_files, int *unique_files_count)
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

synchronize(usb_t *usb1, usb_t *usb2, char **unique_files, int *unique_files_count)
{
    char *file_path;
    char *file_content;
    char *usb1_file_path;
    char *usb2_file_path;

    get_unique_files(usb1, usb2, unique_files, unique_files_count);
    for (int i = 0; i < *unique_files_count; i++)
    {
        strcpy(file_path, unique_files[i]);
        // if file in usb1.file_paths, write to usb2
        for (int j = 0; j < usb1->file_count; i++)
        {
            if (strcmp(usb1->file_paths[j], file_path) == 0)
            {
                char *usb1_file_path[MAX_FILE_PATH_LENGTH];
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
                char *usb2_file_path[MAX_FILE_PATH_LENGTH];
                sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);

                file_content = read_file_to_string(usb2_file_path);
                write_to_USBs(usb1, usb2, file_path, file_content);
            }
        }
    }
}

int readFromUsbs(const char *file_path, usb_t *usb1, usb_t *usb2, char **unique_files, int *unique_files_count)
{
    check_USB_connections(usb1, usb2);
    if (usb1_exist == 0 && usb2_exist == 0)
    {
        synchronize(usb1, usb2, unique_files, unique_files_count);
    }
    else if (usb1_exist == 0)
    {
        char *usb1_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb1_file_path, "%s%s", USB1_MOUNT_PATH, file_path);
        char *file_content = read_file_to_string(usb1_file_path);
    }
    else if (usb2_exist == 0)
    {
        char *usb2_file_path[MAX_FILE_PATH_LENGTH];
        sprintf(usb2_file_path, "%s%s", USB2_MOUNT_PATH, file_path);
        char *file_content = read_file_to_string(usb2_file_path);
    }
    else
    {
        perror("no usb connected.");
    }
}

int main()
{
    // 1. Construct two usb_t for usb1 and usb2
    usb_t usb1;
    usb1.file_count = 0;
    strcpy(usb1.mount_path, USB1_MOUNT_PATH);
    usb_t usb2;
    usb2.file_count = 0;
    strcpy(usb2.mount_path, USB2_MOUNT_PATH);

    write_to_USBs(&usb1, &usb2, "abc.txt", read_file_to_string("a.txt"));

    return 0;
}

// funciton: readFromUsbs(check_USB_connection(); if usb1&usb2 then synchronize
//    else read from either usb1 or usb2
// -> return content)
