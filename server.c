/*
 * File Name: server.c
 * Assignment Title: Practicum II - File Storage System
 *
 * CS5600 Computer Systems / Northeastern University
 * Spring 2023 / Apr 11, 2023
 * Created by Chun Sheung Ng (Derrick) & Zhenyu Wang (Sean)
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "functions.c"

int socket_desc, client_sock;
socklen_t client_size;
struct sockaddr_in server_addr, client_addr;

char unique_files[MAX_FILE_COUNT][MAX_FILE_PATH_LENGTH];
int unique_files_count;
char unique_dirs[MAX_DIR_COUNT][MAX_DIR_PATH_LENGTH];
int unique_dirs_count;

// Construct two usb_t for usb1 and usb2
usb_t usb1;
usb_t usb2;

void *connection_handler(void *);
void process_request(char client_message_copy[8196], char client_message[8196], char server_message[8196]);
void synchronize_();
void *background_thread(void *arg);
pthread_t bg_thread;

int main(void)
{
  usb1 = create_USB_struct(USB1_MOUNT_PATH);
  usb2 = create_USB_struct(USB2_MOUNT_PATH);
  client_size = sizeof(client_addr);
  int opt = 1;

  // memset unique files
  for (int i = 0; i < MAX_FILE_COUNT; i++)
  {
    memset(unique_files[i], '\0', MAX_FILE_PATH_LENGTH);
  }

  // Create socket file descriptor:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0)
  {
    printf("Error while creating socket\n");
    exit(EXIT_FAILURE);
  }
  printf("Socket created successfully\n");

  // Set port and IP:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(2000);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind to the set port and IP:
  if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("Couldn't bind to the port\n");
    return -1;
  }
  printf("Done with binding\n");

  // Start listening for incoming connections
  if (listen(socket_desc, 3) < 0)
  {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Waiting for incoming connections...\n");

  // Synchonize connected USBs
  if (pthread_create(&bg_thread, NULL, background_thread, NULL) != 0)
  {
    fprintf(stderr, "Error creating background thread\n");
    exit(EXIT_FAILURE);
  }

  // while command is not exit, keep running the server
  while (1)
  {
    // Accept incoming connection:
    client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

    if (client_sock < 0)
    {
      printf("Error while accepting the connection\n");
      return -1;
    }
    printf("Client connected at IP: %s and port: %i\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    // Create a new thread for each client:
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, connection_handler, (void *)&client_sock) < 0)
    {
      printf("Error while creating thread\n");
      return -1;
    }

    // Detach the thread:
    pthread_detach(thread_id);
  }
  // Cancel background thread
  if (pthread_cancel(bg_thread) != 0)
  {
    fprintf(stderr, "Error canceling background thread\n");
    exit(EXIT_FAILURE);
  }
  return 0;
}

void *connection_handler(void *client_sock)
{
  int sock = *(int *)client_sock;
  int read_size;
  char server_message[8196], client_message[8196], client_message_copy[8196];

  // Clear buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));

  // Send welcome message to client:
  sprintf(server_message, "Welcome to the server. Type 'esc' to quit.\n");
  if (send(sock, server_message, strlen(server_message), 0) < 0)
  {
    printf("Can't send client the message!\n");
  }

  while ((read_size = recv(sock, client_message, sizeof(client_message), 0)) > 0)
  {

    // Check if the client wants to exit:
    if (strcmp(client_message, "esc") == 0)
    {
      memset(server_message, '\0', sizeof(server_message));
      printf("Client %d requested to disconnect!\n", sock);
      sprintf(server_message, "EXIT$$Goodbye!\n");
      send(sock, server_message, strlen(server_message), 0);
      read_size = 0;
      break;
    }
    printf("Msg from client at socket %d: %s\n", sock, client_message);
    // Process the client's request:
    process_request(client_message_copy, client_message, server_message);

    // Send the result back to the client:
    // write(sock, server_message, strlen(server_message));
    send(sock, server_message, strlen(server_message), 0);

    // Clear buffers:
    memset(client_message, '\0', sizeof(client_message));
    memset(client_message_copy, '\0', sizeof(client_message_copy));
    memset(server_message, '\0', sizeof(server_message));
  }

  if (read_size == 0)
  {
    printf("Client %d disconnected\n", sock);
    fflush(stdout);
  }
  else if (read_size == -1)
  {
    printf("Error while receiving data\n");
  }
  return NULL;
}

void process_request(char client_message_copy[8196], char client_message[8196], char server_message[8196])
{
  char *content; // pointer to file content
  // --------- Parse the message ---------
  char *command;
  int count = 0;
  char **args = malloc(3 * sizeof(char *));
  strcpy(client_message_copy, client_message);
  command = strtok(client_message_copy, "$$");
  while (command != NULL)
  {
    count++;
    args = realloc(args, count * sizeof(char *));
    args[count - 1] = malloc(strlen(command) + 1);
    strcpy(args[count - 1], command);
    command = strtok(NULL, "$$");
  }

  // ------------- Switch command options -------------
  if (strcmp(args[0], "GET") == 0) // ASK: If the remote file or path is omitted, use the values for the first argument.
  {
    // read file by passing in server path into content
    content = read_from_USBs(args[1], &usb1, &usb2);

    if (content)
    {
      // Respond to client:
      sprintf(server_message, "SUCCESS$$SAVE$$%s$$%s", args[2], content); // how to deal with content
      printf("GET command -> %s\n", server_message);                      // print server message
    }
    else
    {
      sprintf(server_message, "ERROR$$FILE_NOT_FOUND when doing GET Command.");
    }
    content = NULL;
  }
  else if (strcmp(args[0], "INFO") == 0) // INFO command
  {
    // read file by passing in server path into content
    content = get_info_from_USBs(args[1], &usb1, &usb2);

    if (content != NULL)
    {
      // Respond to client
      sprintf(server_message, "SUCCESS$$INFO$$%s", content); // how to deal with content
      printf("File info captured: %s\n", server_message);    // print server message
    }
    else
    {
      sprintf(server_message, "ERROR$$FILE_NOT_FOUND when doing INFO Command.");
    }
    content = NULL;
  }
  else if (strcmp(args[0], "MD") == 0) // MD command
  {
    if (create_dir_in_USBs(args[1], &usb1, &usb2) == 0)
    {
      // Respond to client
      sprintf(server_message, "SUCCESS$$MD$$%s", args[1]); // how to deal with content
    }
    else
    {
      sprintf(server_message, "ERROR$$CREATE_FOLDER_FAILURE when doing MD Command.");
    }
  }
  else if (strcmp(args[0], "PUT") == 0) // PUT command
  {
    if (write_to_USBs(&usb1, &usb2, args[1], args[2]) == 0)
    {
      printf("Success: Received the content from client and wrote to file %s.\n", args[1]);
      sprintf(server_message, "SUCCESS$$PUT$$%s", args[1]);
    }
    else
    {
      sprintf(server_message, "ERROR$$PUT_FILE_FAILURE when doing PUT Command.");
      perror("Error writing file\n");
    }
  }
  else if (strcmp(args[0], "RM") == 0) // RM command
  {
    if (remove_file_from_USBs(&usb1, &usb2, args[1]) == 0)
    {
      // Respond to client
      sprintf(server_message, "SUCCESS$$RM$$%s", args[1]); // how to deal with content
    }
    else
    {
      sprintf(server_message, "ERROR$$REMOVE_FILE_FAILURE when doing RM Command.");
    }
  }
  // free args memory
  free(args);
}

// Synchronize function
void synchronize_()
{
  printf("Synchronizing USB devices ... ");
  // Do some synchronization work here
}

// Background thread function
void *background_thread(void *arg)
{
  while (1)
  {
    synchronize_();
    sleep(5);
  }
  return NULL;
}