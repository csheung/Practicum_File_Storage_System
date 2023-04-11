/*
 * server.c -- TCP Socket Server
 *
 * adapted from:
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "functions.c"

int main(void)
{
  int socket_desc, client_sock;
  socklen_t client_size;
  struct sockaddr_in server_addr, client_addr;
  char server_message[8196], client_message[8196], client_message_copy[8196];

  // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));

  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0)
  {
    printf("Error while creating socket\n");
    return -1;
  }
  printf("Socket created successfully\n");

  // Set port and IP:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(2000);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Bind to the set port and IP:
  if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("Couldn't bind to the port\n");
    return -1;
  }
  printf("Done with binding\n");

  // Listen for clients:
  if (listen(socket_desc, 1) < 0)
  {
    printf("Error while listening\n");
    return -1;
  }
  printf("\nListening for incoming connections.....\n");

  // Accept an incoming connection:
  client_size = sizeof(client_addr);
  client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

  if (client_sock < 0)
  {
    printf("Can't accept\n");
    return -1;
  }
  printf("Client connected at IP: %s and port: %i\n",
         inet_ntoa(client_addr.sin_addr),
         ntohs(client_addr.sin_port));

  // while command is not exit, keep running the server
  while (1)
  {

    // Receive client's message:
    int status = recv(client_sock, client_message, sizeof(client_message), 0);
    if (status < 0)
    {
      printf("Couldn't receive\n");
      // Closing the socket:
      close(client_sock);
      close(socket_desc);
      return -1;
    }

    if (strcmp(client_message, "exit") == 0)
    {
      // Closing the socket:
      close(client_sock);
      close(socket_desc);

      return 0;
    }
    printf("Msg from client: %s\n", client_message);

    // --------- Parse the message ---------
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

    //-------------- Switch the command options -------------
    if (strcmp(args[0], "GET") == 0) // ASK: If the remote file or path is omitted, use the values for the first argument.
    {
      char *content = read_file_to_string(args[1]);
      if (content)
      {
        // Respond to client:
        sprintf(server_message, "SUCCESS SAVE %s %s", args[2], content); // how to deal with content
        printf("%s ", server_message);
        // free(content);
      }
      else
      {
        sprintf(server_message, "ERROR FILE_NOT_FOUND");
      }
    }
    else if (strcmp(args[0], "GET2") == 0)
    {
      sprintf(server_message, "ERROR FILE_NOT_FOUND");
    }
    else
    {
      sprintf(server_message, "ERROR FILE_NOT_FOUND");
    }

    if (send(client_sock, server_message, strlen(server_message), 0) < 0)
    {
      printf("Can't send\n");
      return -1;
    }
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));
  }
}
