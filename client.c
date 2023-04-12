/*
 * client.c -- TCP Socket Client
 *
 * adapted from:
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "functions.c"

int socket_desc;
struct sockaddr_in server_addr;
char server_message[8196], server_message_copy[8196], client_message[8196];

// char **args[];
// int argCount = 0;

int main(void)
{
  // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));

  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0)
  {
    printf("Unable to create socket\n");
    return -1;
  }

  printf("Socket created successfully\n");

  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(2000);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Send connection request to server:
  if (connect(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("Unable to connect\n");
    return -1;
  }
  printf("Connected with server successfully\n");

  // while command is not exit, keep running the client
  while (1)
  {
    // Get input from the user:
    printf("Enter message: ");
    gets(client_message);

    if (strcmp(client_message, "exit") == 0)
    {
      // Close the socket:
      close(socket_desc);
      return 0;
    }

    // Send the message to server:
    if (send(socket_desc, client_message, strlen(client_message), 0) < 0)
    {
      printf("Unable to send message\n");
      return -1;
    }

    // Receive the server's response:
    if (recv(socket_desc, server_message, sizeof(server_message), 0) < 0)
    {
      printf("Error while receiving server's msg\n");
      // Closing the socket:
      close(socket_desc);
      return -1;
    }
    printf("Server's response: %s\n", server_message);

    // --------- Parse the message ---------
    char *command;
    int count = 0;
    char **receivedArgs = malloc(4 * sizeof(char *));

    strcpy(server_message_copy, server_message);
    command = strtok(server_message_copy, "$");
    while (command != NULL)
    {
      count++;
      receivedArgs = realloc(receivedArgs, count * sizeof(char *));
      receivedArgs[count - 1] = malloc(strlen(command) + 1);
      strcpy(receivedArgs[count - 1], command);
      command = strtok(NULL, "$");
    }

    //-------------- Switch the command options -------------
    if (strcmp(receivedArgs[0], "ERROR") == 0) // ASK: If the remote file or path is omitted, use the values for the first argument.
    {
      printf("%s ", server_message); // print to be optimized
    }
    else
    {
      if (strcmp(receivedArgs[1], "SAVE") == 0)
      {
        if (write_string_to_file(receivedArgs[2], receivedArgs[3]) == 0)
        {
          printf("Success: Received the content from server and wrote to file %s.\n", receivedArgs[3]);
        }
      }
    }
  }
}
