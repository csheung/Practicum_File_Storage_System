/*
* File Name: client_test.c
* Assignment Title: Practicum II - File Storage System

* CS5600 Computer Systems / Northeastern University
* Spring 2023 / Apr 11, 2023
* Created by Chun Sheung Ng (Derrick) & Zhenyu Wang (Sean)
*
*/

#include "functions.c"
#include <string.h>

// create variables for testing purposes
char unique_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
int unique_path_count = 0;

int main()
{
    // Construct two usb_t for usb1 and usb2
    usb_t usb1 = create_USB_struct();
    usb_t usb2 = create_USB_struct();

    if (use_physical_device)
    {
        read_config_file("physical_USB_config.txt", usb1.mount_path, usb2.mount_path);
    }
    else
    {
        read_config_file("USB_config.txt", usb1.mount_path, usb2.mount_path);
    }

    // Construct thread_args for testing
    thread_args bg_thread_args;
    for (int i = 0; i < MAX_FILE_COUNT; i++)
    {
        memset(bg_thread_args.unique_paths[i], '\0', MAX_FILE_PATH_LENGTH);
    }
    bg_thread_args.usb1 = &usb1;
    bg_thread_args.usb2 = &usb2;
    bg_thread_args.unique_path_count = &unique_path_count;

    // --------- TEST 0 ---------
    // Test read_file_to_string and write_to_USBs
    printf("--------- TEST 0 ---------\n");
    printf("Some functions will let synchronize() align all files in USB(s),\n");
    printf("synchronize() function will be called after every test to get desired results...\n");
    // synchronize() function is called every three seconds in real client-server operation

    // --------- TEST 1 ---------
    // Test read_file_to_string and write_to_USBs
    // a.txt should already exist in USB(s)
    printf("--------- TEST 1 ---------\n");
    printf("Test write and read file from USB(s)...\n");
    char *temp_str;
    if ((temp_str = read_file_to_string("test_log.txt")) != NULL)
    {
        printf("Successfully read content from read_file_to_string function.\n");
        printf("File content: %s\n", temp_str);
    }
    else
    {
        printf("CANNOT read the file content. Check your read_file_to_string function.\n");
    }
    // sychronize both USBs
    synchronize(&usb1, &usb2, bg_thread_args.unique_paths, bg_thread_args.unique_path_count);

    // --------- TEST 2 ---------
    // Test read_file_to_string and write_to_USBs
    printf("--------- TEST 2 ---------\n");
    printf("Read file from USB(s) and Write to new file at the same space...\n");
    write_to_USBs(&usb1, &usb2, "abc.txt", read_file_to_string("test_log.txt"));
    printf("Written file abc.txt\n");
    synchronize(&usb1, &usb2, bg_thread_args.unique_paths, bg_thread_args.unique_path_count);
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
    printf("Expect: file abc.txt added in test 1 will be removed from both USBs\n");
    // sychronize both USBs
    synchronize(&usb1, &usb2, bg_thread_args.unique_paths, bg_thread_args.unique_path_count);

    // file count will be handled by scan_usb, synchronize and list_files
    // printf("%d %d\n", usb1.file_count, usb2.file_count);

    // path_exists
    // --------- TEST 4 ---------
    printf("\n--------- TEST 4 ---------\n");
    printf("Test if path_exists works...\n");
    if (path_exists("a.txt") == 1)
    {
        printf("a.txt is found. Successfully called path_exists function.\n");
    }
    else
    {
        printf("Path is NOT found. Check your path_exists function.\n");
    }

    // --------- TEST 5a ---------
    printf("\n--------- TEST 5a ---------\n");
    printf("Add a new file to new folder(s)...\n");
    printf("Writing file folder1/test_log.txt\n");
    write_to_USBs(&usb1, &usb2, "folder1/test_log.txt", read_file_to_string("log.txt"));
    printf("Expect: new file of path -> test_log.txt\n"); // Expected test result

    printf("Writing file folder2/folder3/test_log.txt\n");
    write_to_USBs(&usb1, &usb2, "folder2/folder3/test_log.txt", read_file_to_string("log.txt"));
    printf("Expect: new file of path -> folder2/folder3/test_log.txt\n"); // Expected test result

    // sychronize both USBs
    synchronize(&usb1, &usb2, bg_thread_args.unique_paths, bg_thread_args.unique_path_count);

    // --------- TEST 5b ---------
    // test create_directory
    printf("\n--------- TEST 5b ---------\n");
    // check if log.txt exists
    if (read_file_to_string("log.txt") != NULL)
    {
        printf("Successfully read content from read_file_to_string function.\n");
    }
    else
    {
        FILE *file = fopen("log.txt", "w");
        if (file == NULL)
        {
            perror("Error opening log.txt file for writing");
            return 1;
        }
        int write_status = fputs("This is a log file for testing...", file);
        if (write_status == EOF)
        {
            perror("Error writing content to file log.txt");
            fclose(file);
        }
        fclose(file);
        printf("Created log.txt file for test...\n");
    }

    printf("Writing file test_log.txt with content read from log.txt...\n");
    write_to_USBs(&usb1, &usb2, "test_log.txt", read_file_to_string("log.txt"));
    printf("Expect: test_log.txt is created and written with log.txt content\n");

    // sychronize both USBs
    synchronize(&usb1, &usb2, bg_thread_args.unique_paths, bg_thread_args.unique_path_count);

    // --------- TEST 6 ---------
    // test remove_file
    printf("\n--------- TEST 6 ---------\n");
    printf("Test remove_file_from_USBs()\n");
    printf("Remove file test_log.txt added in Test 5...\n");
    remove_file_from_USBs(&usb1, &usb2, "test_log.txt");
    printf("Expectation for the following case: Path not exists, so an error message should be printed.\n");
    printf("Read test_log.txt after removing it from usb1 \n", read_from_USBs("test_log.txt", &usb1, &usb2));

    // sychronize both USBs
    synchronize(&usb1, &usb2, bg_thread_args.unique_paths, bg_thread_args.unique_path_count);

    // --------- TEST 7a ---------
    // Test concat_info_content() for subsequent get_unique_paths()
    printf("\n--------- TEST 7a ---------\n");
    if (concat_info_content("Hello", "World!") != NULL)
    {
        printf("Test concat_info_content() and expect to get 'Hello$$World'!: %s...\n", concat_info_content("Hello", "World!"));
    }
    else
    {
        printf("Error: concat_info_content() returns NULL.\n");
    }

    // --------- TEST 7b ---------
    // Test get_unique_paths()
    printf("\n--------- TEST 7b ---------\n");
    write_to_USBs(&usb1, &usb2, "b.txt", "test 7b content");
    printf("Test get_unique_paths() to find difference between USB1 and USB2...\n");
    // char unique_paths[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
    // int unique_path_count = 0;
    get_unique_paths(&usb1, &usb2, bg_thread_args.unique_paths, bg_thread_args.unique_path_count);
    printf("Find below the current elements in unique paths...\n");
    print_unique_paths(unique_paths, unique_path_count);

    // sychronize both USBs
    synchronize(&usb1, &usb2, bg_thread_args.unique_paths, bg_thread_args.unique_path_count);

    // --------- TEST 8 & 9 ---------
    // test get_info && get_info_from_USBs
    printf("\n--------- TEST 8 ---------\n");
    printf("Get background info from one USB\n");

    // get_info from usb1 -> only background info
    char usb_file_path[MAX_FILE_PATH_LENGTH];
    sprintf(usb_file_path, "%s%s", usb1.mount_path, "folder1/test_log.txt");
    printf("Testing get_info in usb1 folder1/test_log.txt...\n");
    printf("%s\n", get_info(usb_file_path));
    printf("Expect: only the info printed for the path folder1/test_log.txt\n");

    // get_info from usb2 -> only background info
    sprintf(usb_file_path, "%s%s", usb2.mount_path, "folder2/folder3/test_log.txt");
    printf("Testing get_info in usb2 folder2/folder3/test_log.txt...\n");
    printf("%s\n", get_info(usb_file_path));
    printf("Expect: only the info printed for the path folder2/folder3/test_log.txt\n");

    // sychronize both USBs
    synchronize(&usb1, &usb2, bg_thread_args.unique_paths, bg_thread_args.unique_path_count);

    // test get_info_from_USBs
    printf("\n--------- TEST 9 ---------\n");
    printf("Get file info from USBs\n");
    // get_info_from_USBs -> background info plus content
    printf("Test get_info_from_USBs for path folder1/test_log.txt...\n");
    // get_info_from_USBs(&usb1, &usb2, "folder1/test_log.txt");
    if (get_info_from_USBs("folder1/test_log.txt", &usb1, &usb2) == NULL)
    {
        printf("Error: Check get_info_from_USBs returns NULL.\n");
    };
    printf("Expect: INFO + Content for the path folder1/test_log.txt below\n");

    // get_info_from_USBs -> background info plus content
    printf("Test get_info_from_USBs for path folder2/folder3/test_log.txt...\n");
    // get_info_from_USBs("folder2/folder3/test_log.txt", &usb1, &usb2);
    if (get_info_from_USBs("folder2/folder3/test_log.txt", &usb1, &usb2) == NULL)
    {
        printf("Error: Check get_info_from_USBs returns NULL.\n");
    };
    printf("Expect: INFO + Content for the path folder2/folder3/test_log.txt below\n");

    // sychronize both USBs
    synchronize(&usb1, &usb2, bg_thread_args.unique_paths, bg_thread_args.unique_path_count);

    // test file count in usb_t struct
    printf("Reviewing file count for USBs updated by synthronize()...\n");
    printf("usb1.file_count: %d, usb2.file_count: %d\n", usb1.file_count, usb2.file_count);

    // --------- TEST 10 ---------
    // test create_dir_in_USBs
    printf("\n--------- TEST 10 ---------\n");
    printf("Test create_dir_in_USBs\n");
    if (create_dir_in_USBs("folder4/folder5/folder6", &usb1, &usb2) == 0)
    {
        printf("Successfully create directories in USB\n");
    }
    else
    {
        printf("Failed to create directories in USB\n");
    }
    printf("Expect: directories folder4, folder5, folder6 are constructed\n");

    // sychronize both USBs
    synchronize(&usb1, &usb2, bg_thread_args.unique_paths, bg_thread_args.unique_path_count);

    // test file count in usb_t struct
    printf("Reviewing file count for USBs updated by synthronize()...\n");
    printf("usb1.file_count: %d, usb2.file_count: %d\n", usb1.file_count, usb2.file_count);

    return 0;
}