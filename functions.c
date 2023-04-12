#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

char *read_file_to_string(const char *filename)
{

    FILE *fp;
    char *str;
    long size;

    fp = fopen(filename, "rb"); // open the file in binary mode
    if (!fp)
    {
        printf("Failed to open the file.\n");
        return NULL;
    }

    fseek(fp, 0, SEEK_END); // move the file pointer to the end of the file
    size = ftell(fp);       // get the size of the file
    fseek(fp, 0, SEEK_SET); // move the file pointer back to the beginning of the file

    str = (char*)malloc(size + 1); // allocate memory for the string
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

int create_directory(const char *path) {

    // handle error of building dir already existed

    int status = mkdir(path, 0755); // 0755 sets the read, write, and execute permissions for the owner, and read and execute permissions for the group and others.

    if (status == 0) {
        printf("Directory created successfully: %s\n", path);
        return 0;
    } else {
        perror("Error creating directory");
        return 1;
    }
}

int remove_file(const char *path) {
    int result = remove(path);

    if (result == 0) {
        printf("File removed successfully: %s\n", path);
        return 0;
    } else {
        perror("Error removing file");
        return 1;
    }
}
