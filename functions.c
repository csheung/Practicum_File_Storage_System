#include <stdio.h>
#include <stdlib.h>

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
