// int main()
// {
//     // Construct two usb_t for usb1 and usb2
//     usb_t usb1 = create_USB_struct(USB1_MOUNT_PATH);
//     usb_t usb2 = create_USB_struct(USB2_MOUNT_PATH);

//     for (int i = 0; i < MAX_FILE_COUNT; i++)
//     {
//         memset(unique_paths[i], '\0', MAX_FILE_PATH_LENGTH);
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

// printf("Read test_log.txt after removing it from usb1: %s\n", read_from_USBs("test_log.txt", &usb1, &usb2, unique_paths, &unique_path_count));

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