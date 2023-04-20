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

#define SERVER_IP "127.0.0.1" // You can obtain the IP address of the server by running the ifconfig or ip addr command on the server's terminal.

int socket_desc;
struct sockaddr_in server_addr;
char server_message[8196], server_message_copy[8196], client_message[8196], client_message_copy[8196];

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        printf("Not enough arguments. Please enter a valid command.\n");
        exit(-1);
    }

    char *command = NULL;
    char **args = NULL;
    int count = 0;

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket file descriptor:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0)
    {
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");
    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5100);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Send connection request to server:
    if (connect(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");

    // Receive the server's welcome message:
    if (recv(socket_desc, server_message, sizeof(server_message), 0) < 0)
    {
        printf("Error while receiving server's msg\n");
        // Closing the socket:
        close(socket_desc);
        return -1;
    }
    printf("Server's response: %s", server_message);
    memset(client_message, '\0', sizeof(client_message));
    memset(server_message, '\0', sizeof(server_message));
    memset(server_message_copy, '\0', sizeof(server_message_copy));
    memset(client_message_copy, '\0', sizeof(client_message_copy));
    // --------- Parse command line input ---------
    command = argv[1];
    for (int i = 2; i < argc; i++)
    {
        count++;
        args = realloc(args, count * sizeof(char *));
        args[count - 1] = malloc(strlen(argv[i]) + 1);
        strcpy(args[count - 1], argv[i]);
    }

    // ------------- Switch command options of client input -------------
    if (strcmp(command, "PUT") == 0)
    {
        // read file by passing in client local path into content
        char *content = read_file_to_string(args[0]);

        if (content)
        {
            if (count == 1)
            {
                sprintf(client_message, "PUT$$%s$$%s", args[0], content);
            }
            else if (count == 2)
            {
                sprintf(client_message, "PUT$$%s$$%s", args[1], content);
            }
        }
        else
        {
            printf("Local file does not exist, please enter a valid command.\n");
            return 0;
        }
    }
    else if (strcmp(command, "GET") == 0)
    {
        if (count == 1)
        {
            sprintf(client_message, "GET$$%s$$%s", args[0], args[0]);
        }
        else if (count == 2)
        {
            sprintf(client_message, "GET$$%s$$%s", args[0], args[1]);
        }
    }
    else if (strcmp(command, "MD") == 0)
    {
        sprintf(client_message, "MD$$%s", args[0]);
    }
    else if (strcmp(command, "INFO") == 0)
    {
        sprintf(client_message, "INFO$$%s", args[0]);
    }
    else if (strcmp(command, "RM") == 0)
    {
        sprintf(client_message, "RM$$%s", args[0]);
    }
    else
    {
        printf("Invalid command. Please enter a valid one.\n");
        exit(-1);
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

    // --------- Parse the SERVER message ---------
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
        printf("Command error, please enter a new message.\n"); // print to be optimized
    }
    else if (strcmp(receivedArgs[0], "EXIT") == 0)
    {
        printf("%s\n", receivedArgs[1]); // print server Goobye
        // Closing the socket:
        close(socket_desc);
        return 0;
    }
    else
    {
        if (strcmp(receivedArgs[1], "SAVE") == 0)
        {
            if (write_string_to_file(NULL, receivedArgs[2], receivedArgs[3]) == 0)
            {
                printf("Success: Received content from server and wrote to file:\n%s\n", receivedArgs[3]);
            }
        }
        else if (strcmp(receivedArgs[1], "MD") == 0)
        {
            printf("Success: Created Directory to server %s\n", receivedArgs[2]);
        }
        else if (strcmp(receivedArgs[1], "INFO") == 0)
        {
            printf("Success: file information:\n%s\nFile Content:\n%s\n", receivedArgs[2], receivedArgs[3]);
        }
        else if (strcmp(receivedArgs[1], "PUT") == 0)
        {
            printf("Success: Put file to server %s\n", receivedArgs[2]);
        }
    }

    return 0;
}
