#include "./interface/server.h"
#include "../commons/interface/adaptor.h"

struct ClientData {
    int client_socket;
    char* client_name;
};

struct ServerData {
    int server_socket;
    struct ClientData client_connection[MAX_CLIENTS];
    //int client_socket[MAX_CLIENTS];
    int isRunning;
};

struct ServerData serverData;

struct WebSocketFrame {
    uint8_t fin;
    uint8_t opcode;
    uint8_t mask;
    uint64_t payload_length;
    uint8_t masking_key[4];
    char* payload;
};

struct ThreadData {
    int sd;
    int client_index;
};

void stopWebSockServer() {

    serverData.isRunning = 0;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (serverData.client_connection[i].client_socket != 0) {
            close(serverData.client_connection[i].client_socket);
            serverData.client_connection[i].client_socket = 0;
            serverData.client_connection[i].client_name = NULL;
        }
    }

    close(serverData.server_socket);
}

void calculate_websocket_key(const char *client_key, char *response_key) {
    const char websocket_magic[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char concatenated_key[BUFFER_SIZE];
    unsigned char sha1_hash[SHA_DIGEST_LENGTH];

    snprintf(concatenated_key, sizeof(concatenated_key), "%s%s", client_key, websocket_magic);
    SHA1((const unsigned char *)concatenated_key, strlen(concatenated_key), sha1_hash);

    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    bio = BIO_new(BIO_s_mem());
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_write(bio, sha1_hash, sizeof(sha1_hash));
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bufferPtr);

    strcpy(response_key, bufferPtr->data);
    BIO_free_all(bio);
}

void handle_client_connection(int client_socket, int client_index) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        perror("Error reading from socket");
        return;
    }

    const char *search_key = "Sec-WebSocket-Key: ";
    char *key_start = strstr(buffer, search_key);
    if (key_start == NULL) {
        fprintf(stderr, "Invalid WebSocket handshake request\n");
        return;
    }

    key_start += strlen(search_key);
    char *key_end = strchr(key_start, '\r');
    if (key_end == NULL) {
        fprintf(stderr, "Invalid WebSocket handshake request\n");
        return;
    }

    *key_end = '\0';
    char client_key[BUFFER_SIZE];
    strncpy(client_key, key_start, BUFFER_SIZE);

    char response_key[BUFFER_SIZE];
    calculate_websocket_key(client_key, response_key);

    const char *handshake_response_format =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s \r\n\r\n";

    char handshake_response[BUFFER_SIZE];
    snprintf(handshake_response, sizeof(handshake_response), handshake_response_format, response_key);

    char client_name[20];
    sprintf(client_name, "client_%d", client_index);

    serverData.client_connection[client_index].client_name = client_name;
    send(client_socket, handshake_response, strlen(handshake_response),0);

}

void sendWebSocketFrame(int socket, const char *data, unsigned char opcode) {
    int data_len = strlen(data);
    unsigned char header[10]; 

    header[0] = 0x80 | opcode;

    if (data_len <= 125) {
        header[1] = data_len;
    } else if (data_len <= 0xFFFF) {
        header[1] = 126;
        header[2] = (data_len >> 8) & 0xFF;
        header[3] = data_len & 0xFF;
    } else {
        header[1] = 127;

        for (int i = 0; i < 8; ++i) {
            header[2 + i] = (data_len >> ((7 - i) * 8)) & 0xFF;
        }
    }

    send(socket, header, (data_len <= 125) ? 2 : 4, 0);
    send(socket, data, data_len, 0);
}

void *threadFunction(void *arg) {
    char* client_message = NULL;
    int client_message_size=0;

    char buffer[1024];
    struct ThreadData *data = (struct ThreadData *) arg;
    

    while (1) {
        int bytes_received = recv(data->sd, buffer, sizeof(buffer), 0);
        // printf("**** %d ", bytes_received);

        if (client_message == NULL) {
            client_message = malloc(bytes_received + 1); 
            if (client_message == NULL) {
                perror("Memory allocation error");
                exit(EXIT_FAILURE);
            }
        } else {
            char *temp = realloc(client_message, client_message_size + bytes_received + 1); 
            if (temp == NULL) {
                perror("Memory allocation error");
                exit(EXIT_FAILURE);
            }
            client_message = temp;
        }

        memcpy(client_message + client_message_size, buffer, bytes_received);
        client_message_size += bytes_received;

        client_message[client_message_size] = '\0';


        if(bytes_received < 1024) {
            break;
        }
    }

    // printf("-- %d --", strlen(client_message));

    struct WebSocketFrame frame;
    // memset(frame.payload, '\0', sizeof(frame.payload));

    frame.fin = (client_message[0] & 0x80) >> 7;
    frame.opcode = client_message[0] & 0x0F;
    frame.mask = (client_message[1] & 0x80) >> 7;
    frame.payload_length = client_message[1] & 0x7F;

    // printf("Fin: %u \n", frame.fin);
    // printf("OpCode: %u \n", frame.opcode);
    // printf("Mask: %u \n", frame.mask);
    // printf("Payload Length: %llu \n", frame.payload_length);

    if (frame.opcode == 0x0) {
        // Implement continuation frame
    } else if (frame.opcode == 0x1) {
        int payload_index = 2;
        
        if(frame.mask) {
            payload_index += 4;
        }

        if(frame.payload_length < 126) {
            memcpy(&frame.masking_key,client_message + 2, 4);
        } else if (frame.payload_length == 126) {
            memcpy(&frame.payload_length, client_message + 2, 2);
            memcpy(&frame.masking_key,client_message + 4, 4);
            frame.payload_length = ntohs(frame.payload_length);
            payload_index += 2;
        } else if (frame.payload_length == 127) {
            memcpy(&frame.payload_length, client_message + 2, 8);
            memcpy(&frame.masking_key,client_message + 10, 4);
            frame.payload_length = ntohll(frame.payload_length);
            payload_index += 8;
        }

        frame.payload = (char *)malloc(frame.payload_length);

        if (frame.payload == NULL) {
            // Check if memory allocation is successful
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }

        for(int j=0; j<frame.payload_length; j++) {
            if(frame.mask) {
                frame.payload[j] = client_message[j+payload_index] ^ frame.masking_key[j%4];
            } else{
                frame.payload[j] = client_message[j+payload_index];
            }
        }    
        frame.payload[frame.payload_length] = '\0';
        // printf("Fin: %u \n", frame.fin);
        // printf("OpCode: %u \n", frame.opcode);
        // printf("Mask: %u \n", frame.mask);
        // printf("Payload Length: %llu \n", frame.payload_length);
        // printf("Payload: %s\n", frame.payload);

        char* result = do_db_ops(frame.payload);
        if(result != NULL) {
            sendWebSocketFrame(data->sd, result, 0x01);
            free(result);
        } else {
            sendWebSocketFrame(data->sd, "{\"Error\":\"Unknown Error\"}", 0x01);
        }

        free(frame.payload);
    } else if (frame.opcode == 0x2) {
        // Implement binary frame
    } else if (frame.opcode == 0x8) {
        close(data->sd);
        serverData.client_connection[data->client_index].client_socket = 0;
        serverData.client_connection[data->client_index].client_socket = '\0';
    } else if (frame.opcode == 0x9) {
        char pongMessage[] = {0x00};  
        send(data->sd, pongMessage, sizeof(pongMessage), 0);
        sendWebSocketFrame(data->sd, pongMessage, 0x0A);
    } else if (frame.opcode == 0xA) {
        // Implement Pong Frame
        printf("Received Pong from Client: %d\n", data->sd);
    } 

    free(client_message);

    pthread_exit(NULL);
}

void startWebSockServer() {
    int max_socket, activity, sd;
    struct sockaddr_in server_addr;
    fd_set master_set, readfds;
    int i, max_clients = MAX_CLIENTS;
    // char server_message[2000], client_message[2000];

    serverData.server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverData.server_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(WEBSOCK_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(bind(serverData.server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Couldn't bind to the port\n");
        exit(1);
    }
    
    if(listen(serverData.server_socket, 1) < 0){
        printf("Error while listening\n");
        exit(1);
    }

    printf("\nListening for incoming connections.....\n");


    for (i = 0; i < max_clients; i++) {
        serverData.client_connection[i].client_socket = 0;
        serverData.client_connection[i].client_name = NULL;
    }

    serverData.isRunning = 1;
    // signal(SIGINT, sigint_handler);
    while (serverData.isRunning) {    
        FD_ZERO(&readfds);
        FD_SET(serverData.server_socket, &readfds);
        max_socket = serverData.server_socket;

        for (i = 0; i < max_clients; i++) {
            sd = serverData.client_connection[i].client_socket;
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_socket)
                max_socket = sd;
        } 

        activity = select(max_socket + 1, &readfds, NULL, NULL, NULL);

        if (activity == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("Select error");
                exit(EXIT_FAILURE);
            }
        }

        if (FD_ISSET(serverData.server_socket, &readfds)) {
            int new_socket = accept(serverData.server_socket, NULL, NULL);
            if (new_socket == -1) {
                if (errno == EINTR) {
                    continue;  
                } else {
                    perror("Accept failed");
                    exit(EXIT_FAILURE);
                }
            }

            for (i = 0; i < max_clients; i++) {
                if (serverData.client_connection[i].client_socket == 0) {
                    serverData.client_connection[i].client_socket = new_socket;
                    handle_client_connection(serverData.client_connection[i].client_socket, i);
                    printf("\nNew connection, socket fd is %d, ip is: %s, port: %d client: %s \n",
                           new_socket, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port), serverData.client_connection[i].client_name);
                    break;
                }
            }
        }

       for (i = 0; i < max_clients; i++) {
            sd = serverData.client_connection[i].client_socket;
            if (FD_ISSET(sd, &readfds)) {

                struct ThreadData data;
                data.sd = sd;
                data.client_index = i;
                pthread_t thread;
                
                if (pthread_create(&thread, NULL, threadFunction, (void *) &data) != 0) {
                    fprintf(stderr, "Error creating thread\n");
                    exit(1);
                } else {
                    printf("New Thread created for client of index: %d\n", i);
                }

                pthread_join(thread, NULL);
                printf("Back to Main Thread While Loop \n");
            }
        } 
    }

    printf("## Shutting down the server ## \n");

}