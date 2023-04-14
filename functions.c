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

int write_string_to_file(const char *filename, const char *str)
{
    FILE *fp;
    char filename_copy[256];
    char *directory = dirname((char *)filename);

    char *dir = strtok(directory, "/");
    char curr_dir[256] = "";
    while (dir != NULL)
    {
        strcat(curr_dir, dir);
        mkdir(curr_dir, 0777);
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

int create_directory(const char *path)
{

    // handle error of building dir already existed

    int status = mkdir(path, 0755); // 0755 sets the read, write, and execute permissions for the owner, and read and execute permissions for the group and others.

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
        perror("Error removing file");
        return 1;
    }
}

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
            return NULL;
        }
        memcpy(output + output_size, buffer, len + 1);
        output_size += len;
    }

    pclose(fp);
    return output;
}
