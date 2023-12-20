#include "./interface/server.h"

void startTCPServer() {

    int server_socket, max_socket, activity, client_socket[MAX_CLIENTS], sd;
    struct sockaddr_in server_addr;
    fd_set master_set, readfds;
    int i, max_clients = MAX_CLIENTS;
    char server_message[2000], client_message[2000];

    // Create and set up the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Bind to the set port and IP:
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Couldn't bind to the port\n");
        exit(1);
        // return -1;
    }
    printf("Done with binding\n");
    
    // Listen for clients:
    if(listen(server_socket, 1) < 0){
        printf("Error while listening\n");
        exit(1);
        // return -1;
    }
    printf("\nListening for incoming connections.....\n");


    for (i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }

    while (1) {    
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_socket = server_socket;
        // printf("\n Max Socket before while initalizing a while loop: %d", max_socket);

        // Add child sockets to set
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_socket)
                max_socket = sd;

            // printf("--------- %d--%d", sd, max_clients);
        } 

        // printf("Max Socket: %d", max_socket);
        // Wait for an activity on any of the sockets using select
        activity = select(max_socket + 1, &readfds, NULL, NULL, NULL);
        // printf("\n ***************** Activity: %d", activity);

        if (activity < 0) {
            perror("Select error");
        }

        // If something happened on the server socket, then it's an incoming connection
        if (FD_ISSET(server_socket, &readfds)) {
            int new_socket = accept(server_socket, NULL, NULL);
            if (new_socket < 0) {
                perror("Accept failed");
                exit(1);
            }

            // Add the new socket to the array of client sockets
            for (i = 0; i < max_clients; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    printf("\n New connection, socket fd is %d, ip is: %s, port: %d\n",
                           new_socket, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
                    break;
                }
            }
        }

        // Handle client connections
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                memset(server_message, '\0', sizeof(server_message));
                memset(client_message, '\0', sizeof(client_message));

                // Handle data from the client socket
                // ... (Receive and send data as needed)
                // Receive client's message:
                int bytes = recv(sd, client_message, sizeof(client_message), 0);
                if (bytes <= 0){
                    // printf("Couldn't receive\n");
                    // return -1;
                } else {
                    printf("\n Msg from client: %s\n", client_message);
                
                    // Respond to client:
                    strcpy(server_message, "This is the server's message.");
                
                    if (send(sd, server_message, strlen(server_message), 0) < 0){
                        printf("Can't send\n");
                        // return -1;
                    }
                }
            }
        }
    }

    // Closing the socket:
    close(server_socket);
    for (i = 0; i < max_clients; i++) {
        if (client_socket[i] != 0) {
            close(client_socket[i]);
        }
    }
}