#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/un.h>

int main(void){

    int socket_desc;
    struct sockaddr_un server_addr;
    char server_message[2000], client_message[2000];

    // Clean buffers:
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_UNIX, SOCK_STREAM, 0);

    if(socket_desc == -1 && errno == EINTR){
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");

    server_addr.sun_family = AF_UNIX;

    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");

    // Get input from the user:
    printf("Enter message: ");
    gets(client_message);

    // Send the message to server:
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }

    // Receive the server's response:
    if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }

    printf("Server's response: %s\n",server_message);

    // Close the socket:
    close(socket_desc);

    return 0;
}