usb1_exist = 0;
usb2_exist = 0;

typedef struct
{
    list<file_path>;
} usb_t;

// function: writeToUsbs(check_USB_connection(); if usb1  -> wirte ; if usb2  -> write in usb2) // overwrite if file exists
// funciton: readFromUsbs(check_USB_connection(); if usb1&usb2 then synchronize
//    else read from either usb1 or usb2
// -> return content)

// function: findDifference() -> list<file>{
//  for i in usb2.list:
//        if i in usb1.list:
//       list.append(i)
// for j in usb1.list:
//        if j in usb2.list:
//       list.append(j)
//}

// function: synchronize( findDifference() ->
// for file_name in list<file>:
//     if file_name in usb1.file_list: file_device_path = /usb1/file_name;
//      else: file_device_path = /usb2/file_name;
//    file_content = read_string_from_file(file_device_path)
//    writeToUsbs(file_content)

// function: check_USB_connection( update usb_exist flags)
