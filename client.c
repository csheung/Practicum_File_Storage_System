/*
 * File Name: client.c
 * Assignment Title: Practicum II - File Storage System
 *
 * CS5600 Computer Systems / Northeastern University
 * Spring 2023 / Apr 11, 2023
 * Created by Chun Sheung Ng (Derrick) & Zhenyu Wang (Sean)
 *
 */

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
char server_message[8196], server_message_copy[8196], client_message[8196], client_message_copy[8196];

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

  // while receivedCommand is not exit, keep running the client
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

    // --------- Parse client input ---------
    char *command;
    int count = 0;
    char **args = malloc(3 * sizeof(char *));

    strcpy(client_message_copy, client_message);
    command = strtok(client_message_copy, " ");
    while (command != NULL)
    {
      count++;
      args = realloc(args, count * sizeof(char *));
      args[count - 1] = malloc(strlen(command) + 1);
      strcpy(args[count - 1], command);
      command = strtok(NULL, " ");
    }
    // print out the commands
    for (int i = 0; i < count; i++)
    {
      printf("%s ", args[i]);
    }
    printf("\n");

    // ------------- Switch command options of client input -------------
    if (strcmp(args[0], "PUT") == 0)
    {
      // read file by passing in client local path into content
      char *content = read_file_to_string(args[1]);
      // printf("\nPUT command -> Content read from Client File: %s\n", content); // fot testing
      
      if (content)
      {
        // Respond to client:
        sprintf(client_message, "PUT$$%s$$%s", args[2], content); // how to deal with content
        printf("PUT command data -> %s\n", client_message); // print server message
      }
    } 
    else if (strcmp(args[0], "GET") == 0)
    {
      sprintf(client_message, "GET$$%s$$%s", args[1], args[2]);
    }
    else if (strcmp(args[0], "MD") == 0)
    {
      sprintf(client_message, "MD$$%s", args[1]);
    }
    else if (strcmp(args[0], "INFO") == 0)
    {
      sprintf(client_message, "INFO$$%s", args[1]);
    }
    else if (strcmp(args[0], "RM") == 0)
    {
      sprintf(client_message, "RM$$%s", args[1]);
    }
    else
    {
      // error message
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
    char *receivedCommand;
    int recvCount = 0;
    char **receivedArgs = malloc(4 * sizeof(char *));

    strcpy(server_message_copy, server_message);
    receivedCommand = strtok(server_message_copy, "$$");
    while (receivedCommand != NULL)
    {
      recvCount++;
      receivedArgs = realloc(receivedArgs, recvCount * sizeof(char *));
      receivedArgs[recvCount - 1] = malloc(strlen(receivedCommand) + 1);
      strcpy(receivedArgs[recvCount - 1], receivedCommand);
      receivedCommand = strtok(NULL, "$$");
    }

    //-------------- Switch the receivedCommand options -------------
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
      else if (strcmp(receivedArgs[1], "MD") == 0)
      {
        printf("Success: Created Directory to server %s.\n", receivedArgs[2]);
      }
      else if (strcmp(receivedArgs[1], "MD") == 0)
      {
        printf("Success: Put file to server %s.\n", receivedArgs[2]);
      }
    }
  }
}
