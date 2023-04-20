/*
* File Name: client_test.c
* Assignment Title: Practicum II - File Storage System

* CS5600 Computer Systems / Northeastern University
* Spring 2023 / Apr 11, 2023
* Created by Chun Sheung Ng (Derrick) & Zhenyu Wang (Sean)
*
*/

#include "functions.c"

int main()
{
    // Construct two usb_t for usb1 and usb2
    usb_t usb1 = create_USB_struct();
    usb_t usb2 = create_USB_struct();

    for (int i = 0; i < MAX_FILE_COUNT; i++)
    {
        memset(usb1.file_paths[i], '\0', MAX_FILE_PATH_LENGTH);
        memset(usb2.file_paths[i], '\0', MAX_FILE_PATH_LENGTH);
    }
    read_config_file("USB_config.txt", usb1.mount_path, usb2.mount_path);


    // --------- TEST 1 ---------
    // Test read_file_to_string and write_to_USBs
    printf("--------- TEST 1 ---------\n");
    printf("Test read file from USB(s)...\n");
    char *temp_str;
    if ((temp_str = read_file_to_string("a.txt")) != NULL) {
        printf("Successfully read content from read_file_to_string function.\n");
        printf("File content: %s\n", temp_str);
    } else {
        printf("CANNOT read the file content. Check your read_file_to_string function.\n");
    }


    // --------- TEST 2 ---------
    // Test read_file_to_string and write_to_USBs
    printf("--------- TEST 2 ---------\n");
    printf("Read file from USB(s) and Write to new file at the same space...\n");
    write_to_USBs(&usb1, &usb2, "abc.txt", read_file_to_string("a.txt"));
    printf("Written file abc.txt\n");
    // Expect: new file "abc.txt" with the same content as "a.txt" is written to both USBs

    // file count will be handled by scan_usb, synchronize and list_files
    // printf("%d %d\n", usb1.file_count, usb2.file_count); 


    // --------- TEST 3 ---------
    // Test remove_file_from_USBs
    printf("\n--------- TEST 3 ---------\n");
    printf("Next step is to remove the file added in Test 2...\n");
    remove_file_from_USBs(&usb1, &usb2, "abc.txt");
    printf("After removing file abc.txt from the USBs\n");
    // Expect: file "abc.txt" added in test 1 will be removed from both USBs

    // file count will be handled by scan_usb, synchronize and list_files
    // printf("%d %d\n", usb1.file_count, usb2.file_count);     


    // path_exists
    // --------- TEST 4 ---------
    printf("\n--------- TEST 4 ---------\n");
    printf("Test if path_exists works...\n");
    if (path_exists("t1/a.txt") == 1) {
        printf("t1/a.txt is found. Successfully called path_exists function.\n");
    } else {
        printf("Path is NOT found. Check your path_exists function.\n");
    }


    // --------- TEST 5 ---------
    // test create_directory
    printf("--------- TEST 5 ---------\n");
    // check if log.txt exists
    if (read_file_to_string("log.txt") != NULL) {
        printf("Successfully read content from read_file_to_string function.\n");
    } else {
        FILE *file = fopen("log.txt", "w");
        if (file == NULL) {
            perror("Error opening log.txt file for writing");
            return 1;
        }
        int write_status = fputs("This is a log file for testing...", file);
        if (write_status == EOF) {
            perror("Error writing content to file log.txt");
            fclose(file);
        }
        fclose(file);
        printf("Created log.txt file for test...\n");
    }

    printf("Writing file test_log.txt with content read from log.txt...\n");
    write_to_USBs(&usb1, &usb2, "test_log.txt", read_file_to_string("log.txt"));
    printf("Expect: test_log.txt is created and written with log.txt content\n");
    // printf("usb1.file_count, usb2.file_count: %d %d\n", usb1.file_count, usb2.file_count);

    printf("Writing file test_log1.txt with content read from test_log.txt\n");
    write_to_USBs(&usb1, &usb2, "test_log1.txt", read_file_to_string("test_log.txt"));
    printf("Writing file test_log1.txt\n");
    printf("Expect: test_log1.txt is created and written with test_log.txt content\n");
    // printf("usb1.file_count, usb2.file_count: %d %d\n", usb1.file_count, usb2.file_count);


    // --------- TEST 6 ---------
    // test remove_file
    printf("\n--------- TEST 6 ---------\n");
    printf("Test remove_file_from_USBs()\n");
    printf("Remove file test_log1.txt added in Test 5...\n");
    remove_file_from_USBs(&usb1, &usb2, "test_log1.txt");
    printf("Read test_log1.txt after removing it from usb1: %s\n", read_from_USBs("test_log1.txt", &usb1, &usb2));
    printf("Expect: Path not exists, so an error message should be printed.\n");


    // --------- TEST 7 ---------
    // Test get_unique_paths()
    printf("\n--------- TEST 7 ---------\n");
    printf("Next step is to remove the file added in Test 1...\n");
    printf("Test XXX Test get_unique_paths()\n");


    // // --------- TEST 4 ---------
    // printf("Test 4 Add a new file to a new folder\n");
    // write_to_USBs(&usb1, &usb2, "folder1/test_log.txt", read_file_to_string("log.txt"));
    // printf("Writing file folder1/test_log.txt\n");
    // printf("usb1.file_count, usb2.file_count: %d %d\n", usb1.file_count, usb2.file_count);

    // write_to_USBs(&usb1, &usb2, "folder2/folder3/test_log.txt", read_file_to_string("log.txt"));
    // printf("Writing file folder2/folder3/test_log.txt\n");
    // printf("usb1.file_count, usb2.file_count: %d %d\n", usb1.file_count, usb2.file_count);



 return 0;
}