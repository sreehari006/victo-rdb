#include "./includes/server_proto.h"
#include "../auth/includes/crypto_proto.h"
#include "../auth/includes/user_ops_proto.h"
#include "../commons/includes/adaptor_proto.h"
#include "../commons/includes/globals_proto.h"
#include "../../utils/logs/includes/log_proto.h"
#include "../../utils/uuid/includes/uuid_proto.h"

struct ClientData {
    int client_socket;
    char* client_id;
    int user_access[MAX_ACCESS_INDEX_TYPES];
};

struct ServerData {
    int server_socket;
    struct ClientData client_connection[MAX_CLIENTS];
    int isRunning;
};

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

static struct ServerData serverData;

void nullifyClientConnection(int i) {
    serverData.client_connection[i].client_socket = 0;
    serverData.client_connection[i].client_id = NULL;
    for(int j=0; j<25; j++) {
        serverData.client_connection[i].user_access[j] = USER_ACCESS_NO_ACCESS;
    }
}
void stop_websock_server() {
    vt__log_writer(LOG_DEBUG, "server stopWebSockServer started");
    serverData.isRunning = 0;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (serverData.client_connection[i].client_socket != 0) {
            close(serverData.client_connection[i].client_socket);
            nullifyClientConnection(i);
        }
    }

    free_subscribe_trig_messag_queue();
    close(serverData.server_socket);
    vt__log_writer(LOG_DEBUG, "server stopWebSockServer completed");
}

void calculate_websocket_key(const char *client_key, char *response_key) {
    vt__log_writer(LOG_DEBUG, "server calculate_websocket_key started");

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

    vt__log_writer(LOG_DEBUG, "server calculate_websocket_key completed");
}

bool handle_client_connection(int client_socket, int client_index) {
    vt__log_writer(LOG_DEBUG, "server handle_client_connection started");

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received == 0) {
        vt__log_writer(LOG_ERROR, "Error reading from socket during client handshake. Client Might have closed the connection");
        return false;
    }

    if (bytes_received < 0) {
        vt__log_writer(LOG_ERROR, "Error reading from socket during client handshake. Unable to read data.");
        return false;
    }

    char* sec_websocket_key_copy = strdup(buffer);
    const char *search_key = "Sec-WebSocket-Key: ";
    char *key_start = strstr(sec_websocket_key_copy, search_key);
    if (key_start == NULL) {
        vt__log_writer(LOG_ERROR, "Invalid WebSocket handshake request");
        return false;
    }

    key_start += strlen(search_key);
    char *key_end = strchr(key_start, '\r');
    if (key_end == NULL) {
        vt__log_writer(LOG_ERROR, "Invalid WebSocket handshake request");
        return false;
    }

    *key_end = '\0';

    char client_key[BUFFER_SIZE];
    strncpy(client_key, key_start, BUFFER_SIZE);
    free(sec_websocket_key_copy);

    if(is_auth_enabled()) {
        char* auth_key_copy = strdup(buffer);
        const char *auth_search_key = "Authorization: ";
        char *auth_key_start = strstr(auth_key_copy, auth_search_key);
        if (auth_key_start == NULL) {
            vt__log_writer(LOG_ERROR, "Invalid WebSocket handshake request. Missing auth token.");
            return false;
        }

        auth_key_start += strlen(auth_search_key);
        char *auth_key_end = strchr(auth_key_start, '\r');
        if (auth_key_end == NULL) {
            vt__log_writer(LOG_ERROR, "Invalid WebSocket handshake request. Missing auth token.");
            return false;
        }

        *auth_key_end = '\0';

        char auth_token[BUFFER_SIZE];
        strncpy(auth_token, auth_key_start, BUFFER_SIZE);
        free(auth_key_copy);

        const char* authTokenStripped = auth_token + strlen("Basic ");
        if(authTokenStripped == NULL) {
            vt__log_writer(LOG_ERROR, "Invalid WebSocket handshake request. Invalid auth token.");
            return false;            
        }

        char *decoded;
        size_t decoded_len;

        vt__base64_decode(authTokenStripped, &decoded, &decoded_len);
        if (decoded) {
            char* user_name = strtok(decoded, ":");
            char* password = strtok(NULL, ":");
            char* passwordHash = vt__sha256(password);
            User* user = authenticate(user_name, passwordHash);
            free(decoded);
            if(user == NULL) {
                vt__log_writer(LOG_ERROR, "Invalid WebSocket handshake request. Authentication failed.");
                return false;
            }

            serverData.client_connection[client_index].client_socket = client_socket;
            serverData.client_connection[client_index].client_id = strdup(user->uuid);

            for(int i=0; i<25; i++) {
                serverData.client_connection[client_index].user_access[i] = user->user_access[i];
            }

            free(user);
        } else {
            vt__log_writer(LOG_ERROR, "Invalid WebSocket handshake request. Decoding auth token failed.");
            return false;               
        }

    } else {
            serverData.client_connection[client_index].client_socket = client_socket;
            serverData.client_connection[client_index].client_id = strdup(USER_GUEST);
            for(int i=0; i<25; i++) {
                serverData.client_connection[client_index].user_access[i] = USER_ACCESS_FULL_ACCESS;
            }  
    }


    char response_key[BUFFER_SIZE];
    calculate_websocket_key(client_key, response_key);

    const char *handshake_response_format =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s \r\n\r\n";

    char handshake_response[BUFFER_SIZE];
    snprintf(handshake_response, sizeof(handshake_response), handshake_response_format, response_key);

    vt__log_writer(LOG_INFO, "Client connection request successful");
    send(client_socket, handshake_response, strlen(handshake_response),0);

    vt__log_writer(LOG_DEBUG, "server handle_client_connection completed");

    return true;
}

void sendWebSocketFrame(int socket, const char *data, unsigned char opcode) {
    vt__log_writer(LOG_DEBUG, "server sendWebSocketFrame started");
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

    vt__log_writer(LOG_INFO, "Sending response to client");
    vt__log_writer(LOG_DEBUG, data);
    send(socket, header, (data_len <= 125) ? 2 : 4, 0);
    send(socket, data, data_len, 0);
    
    vt__log_writer(LOG_DEBUG, "server sendWebSocketFrame completed");
}

void *threadFunction(void *arg) {
    vt__log_writer(LOG_INFO, "server threadFunction started");

    char sysThreadID[25]; 
    snprintf(sysThreadID, sizeof(sysThreadID), "%p", (void *) pthread_self());
    char* threadUUID = vt__get_uuid();
    vt__set_thread_id_on_register(sysThreadID, threadUUID);

    vt__log_writer(LOG_DEBUG, "Inside threadFunciton for processing the client message");
    
    char* client_message = NULL;
    int client_message_size=0;

    char buffer[1024];
    struct ThreadData *data = (struct ThreadData *) arg;
    
    vt__log_writer(LOG_DEBUG, "Read client messages");
    while (1) {
        int bytes_received = recv(data->sd, buffer, sizeof(buffer), 0);
        if (bytes_received == 0) {
            nullifyClientConnection(data->client_index);
            vt__log_writer(LOG_WARN, "Inside thread function. Socket might be closed for reading messages.");
            return NULL;
        }

        if (bytes_received < 0) {
            nullifyClientConnection(data->client_index);
            close(data->sd);
            vt__log_writer(LOG_WARN, "Inside thread function. Error reading data.");
            return NULL;
        }

        if (client_message == NULL) {
            client_message = malloc(bytes_received + 1); 
            if (client_message == NULL) {
                vt__log_writer(LOG_CRITICAL, "Memory allocation failed while reading client messages");
                exit(EXIT_FAILURE);
            }
        } else {
            char *temp = realloc(client_message, client_message_size + bytes_received + 1); 
            if (temp == NULL) {
                vt__log_writer(LOG_CRITICAL, "Memory re-allocation failed while reading client messages");
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

    vt__log_writer(LOG_DEBUG, "Start extract mesages from websocket frame");
    struct WebSocketFrame frame;
    frame.fin = (client_message[0] & 0x80) >> 7;
    frame.opcode = client_message[0] & 0x0F;
    frame.mask = (client_message[1] & 0x80) >> 7;
    frame.payload_length = client_message[1] & 0x7F;

    if (frame.opcode == 0x0) {
        vt__log_writer(LOG_DEBUG, "OP Code - continuation frame");
        
        // Implement continuation frame

    } else if (frame.opcode == 0x1) {
        vt__log_writer(LOG_DEBUG, "OP Code - text frame");
        int payload_index = 2;
        
        vt__log_writer(LOG_DEBUG, "Payload is masked");
        if(frame.mask) {
            payload_index += 4;
        }

        if(frame.payload_length < 126) {
            vt__log_writer(LOG_DEBUG, "Payload length is less than 126 bytes");
            memcpy(&frame.masking_key,client_message + 2, 4);
        } else if (frame.payload_length == 126) {
            vt__log_writer(LOG_DEBUG, "Payload length field is 126 bytes");
            memcpy(&frame.payload_length, client_message + 2, 2);
            memcpy(&frame.masking_key,client_message + 4, 4);
            frame.payload_length = ntohs(frame.payload_length);
            payload_index += 2;
        } else if (frame.payload_length == 127) {
            vt__log_writer(LOG_DEBUG, "Payload length field is 127 bytes");
            memcpy(&frame.payload_length, client_message + 2, 8);
            memcpy(&frame.masking_key,client_message + 10, 4);
            frame.payload_length = ntohll(frame.payload_length);
            payload_index += 8;
        }

        frame.payload = (char *)malloc(frame.payload_length);

        if (frame.payload == NULL) {
            vt__log_writer(LOG_CRITICAL, "Memory allocation failed while reading payload from websocket frames");
            exit(EXIT_FAILURE);
        }

        vt__log_writer(LOG_DEBUG, "Extract payload");
        for(int j=0; j<frame.payload_length; j++) {
            if(frame.mask) {
                frame.payload[j] = client_message[j+payload_index] ^ frame.masking_key[j%4];
            } else{
                frame.payload[j] = client_message[j+payload_index];
            }
        }    

        frame.payload[frame.payload_length] = '\0';

        vt__log_writer(LOG_INFO, "Start DB Operations");
        ClientInfo clientInfo;
        clientInfo.client_id = strdup(serverData.client_connection[data->client_index].client_id);
        for(int i=0; i<25; i++) {
            clientInfo.user_access[i] = serverData.client_connection[data->client_index].user_access[i];
        }

        char* result = do_db_ops(threadUUID, frame.payload, clientInfo);
        if(result != NULL) {
            vt__log_writer(LOG_INFO, "Send query result back to client");
            sendWebSocketFrame(data->sd, result, 0x01);
            free(result);
        } else {
            vt__log_writer(LOG_WARN, "Query result is empty");
            char null_result[100] = "{\"response_id\": \"";
            strcat(null_result, threadUUID);
            strcat(null_result, "\", \"Error\":\"Unknown Error\"}");
            sendWebSocketFrame(data->sd, null_result, 0x01);
        }

        free(frame.payload);
        free(client_message);
        client_message = NULL;
        client_message_size = 0;
    } else if (frame.opcode == 0x2) {
        // Implement binary frame
    } else if (frame.opcode == 0x8) {
        nullifyClientConnection(data->client_index);
        
        unsigned char closeFrame[4];
        closeFrame[0] = 0x88;
        closeFrame[1] = 0x02;
        closeFrame[2] = (unsigned char)((1000 >> 8) & 0xFF);
        closeFrame[3] = (unsigned char)(1000 & 0xFF);

        send(data->sd, closeFrame, sizeof(closeFrame), 0);
        close(data->sd);
    } else if (frame.opcode == 0x9) {
        char pongMessage[] = {0x00};  
        send(data->sd, pongMessage, sizeof(pongMessage), 0);
        sendWebSocketFrame(data->sd, pongMessage, 0x0A);
    } else if (frame.opcode == 0xA) {
        // Implement Pong Frame
        printf("Received Pong from Client: %d\n", data->sd);
    } 

    free(client_message);
    free(data);
    vt__log_writer(LOG_INFO, "server threadFunction completed");
    free(threadUUID);
    sleep(1);
    vt__remove_thread_id_from_register(sysThreadID);

    pthread_exit(NULL);
}

void *processSubscribeThreadFucntion(void *arg) {
    SubscribeTrigMsgNode* subscribeTrigMsgNode = (SubscribeTrigMsgNode*) arg;
    SubscriptionListNode* subscriptionMessageList = query_subscription(subscribeTrigMsgNode);

    while (subscriptionMessageList != NULL) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (serverData.client_connection[i].client_socket > 0 && strcmp(serverData.client_connection[i].client_id, subscriptionMessageList->client_id) == 0) {
                while(subscriptionMessageList->message != NULL) {
                    char* result = subscription_message(strdup(subscriptionMessageList->message->vector_hash), strdup(subscriptionMessageList->message->query_hash));
                    if(result) {
                        sendWebSocketFrame(serverData.client_connection[i].client_socket, result, 0x01);
                        free(result);
                    }    
                }
                break;
            }    
        } 
        subscriptionMessageList = subscriptionMessageList->next;
    }


    pthread_exit(NULL);
}

void *subscribeThreadFunction(void *arg) {
    while (1) {

        SubscribeTrigMsgNode* subscribeTrigMsgNode = dequeue_subscribe_trig_message();
        pthread_t processSubscriptionThread;
        if (pthread_create(&processSubscriptionThread, NULL, processSubscribeThreadFucntion, subscribeTrigMsgNode) != 0) {
            vt__log_writer(LOG_CRITICAL, "Error creating a thread for processing subscription");
            exit(EXIT_FAILURE);
        } else {
            // char sysThreadID[25]; 
            // snprintf(sysThreadID, sizeof(sysThreadID), "%p", subscriptionThread);
            // vt__set_thread_id_on_register(sysThreadID, "subscriber");
            vt__log_writer(LOG_INFO, "New thread created for processing subscription: ");
        }

        /* SubscribeReplyInfo subscribeReplyInfo = querySubscription();
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (serverData.client_connection[i].client_socket > 0 && strcmp(serverData.client_connection[i].client_id, subscribeReplyInfo.client_id) == 0) {
                sendWebSocketFrame(serverData.client_connection[i].client_socket, subscribeReplyInfo.vector_hash, 0x01);
                printf("Queued Hash: %s %d %s %s\n", serverData.client_connection[i].client_id, serverData.client_connection[i].client_socket, subscribeReplyInfo.vector_hash, subscribeReplyInfo.client_id);
            } 
        } */

        if(!serverData.isRunning) {
            break;
        }

    }

    pthread_exit(NULL);
}

void startWebSockServer() {
    char sysThreadID[25]; 
    snprintf(sysThreadID, sizeof(sysThreadID), "%p", (void *) pthread_self());
    vt__set_thread_id_on_register(sysThreadID, "main");

    vt__log_writer(LOG_DEBUG, "server startWebSockServer started");
    int max_socket, activity, sd;
    struct sockaddr_in server_addr;
    fd_set master_set, readfds;
    int i, max_clients = MAX_CLIENTS;

    serverData.server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverData.server_socket < 0) {
        vt__log_writer(LOG_CRITICAL, "Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(get_websock_init_port());
    server_addr.sin_addr.s_addr = inet_addr(get_websock_init_IP());
    
    if(bind(serverData.server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))<0) {
        vt__log_writer(LOG_CRITICAL, "Couldn't bind to the port");
        exit(EXIT_FAILURE);
    }
    
    if(listen(serverData.server_socket, 1) < 0){
        vt__log_writer(LOG_CRITICAL, "Error while listening");
        exit(EXIT_FAILURE);
    }

    vt__log_writer(LOG_DEBUG, "Initiatize serverData with client sockets and client name");
    for (i = 0; i < max_clients; i++) {
        nullifyClientConnection(i);
    }

    init_subscribe_trig_queue();
    pthread_t subscriptionThread;
    if (pthread_create(&subscriptionThread, NULL, subscribeThreadFunction, NULL) != 0) {
        vt__log_writer(LOG_CRITICAL, "Error creating a thread for subscription");
        exit(EXIT_FAILURE);
    } else {
        char sysThreadID[25]; 
        snprintf(sysThreadID, sizeof(sysThreadID), "%p", subscriptionThread);
        vt__set_thread_id_on_register(sysThreadID, "subscriber");
        vt__log_writer(LOG_INFO, "New thread created for handling subscription: ");
    }

    vt__log_writer(LOG_DEBUG, "Listening for incoming connections and message");
    serverData.isRunning = 1;
    while (serverData.isRunning) {    
        FD_ZERO(&readfds);
        FD_SET(serverData.server_socket, &readfds);
        max_socket = serverData.server_socket;

        for (i = 0; i < max_clients; i++) {
            sd = serverData.client_connection[i].client_socket;
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
                
            if (sd > max_socket)
                max_socket = sd;
        } 

        vt__log_writer(LOG_DEBUG, "Look for activity on sockets");
        activity = select(max_socket + 1, &readfds, NULL, NULL, NULL);
        
        if (activity == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("Select error");
                vt__log_writer(LOG_DEBUG, "Select error while looking for activity on sockets");
                exit(EXIT_FAILURE);
            }
        }

        if (FD_ISSET(serverData.server_socket, &readfds)) {
            vt__log_writer(LOG_DEBUG, "Accept client connection");
            int new_socket = accept(serverData.server_socket, NULL, NULL);
            if (new_socket == -1) {
                if (errno == EINTR) {
                    continue;  
                } else {
                    vt__log_writer(LOG_CRITICAL, "Connection accept failed on server socket");
                    exit(EXIT_FAILURE);
                }
            }

            for (i = 0; i < max_clients; i++) {
                if (serverData.client_connection[i].client_socket == 0) {
                    if(handle_client_connection(new_socket, i)) {
                        // serverData.client_connection[i].client_socket = new_socket;
                        vt__log_writer(LOG_INFO, "New connection set");
                        vt__log_writer(LOG_INFO, "Socket file descriptor: ");
                        char new_socket_str[20];
                        sprintf(new_socket_str, "%d", new_socket);
                        vt__log_writer(LOG_INFO, new_socket_str);
                        vt__log_writer(LOG_INFO, "Server IP: ");
                        vt__log_writer(LOG_INFO, inet_ntoa(server_addr.sin_addr));
                        // vt__log_writer(LOG_INFO, "Server Port: ");
                        // vt__log_writer(LOG_INFO, ntohs(server_addr.sin_port));
                        vt__log_writer(LOG_INFO, "Connected client id:");
                        vt__log_writer(LOG_INFO, serverData.client_connection[i].client_id);

                    } else {
                        close(new_socket);
                    } 
                    
                    break;
                }
            }
        }

       for (i = 0; i < max_clients; i++) {
            sd = serverData.client_connection[i].client_socket;
            if (FD_ISSET(sd, &readfds)) {
                struct ThreadData* data = malloc(sizeof(struct ThreadData));;
                data->sd = sd;
                data->client_index = i;
                pthread_t thread;
                
                if (pthread_create(&thread, NULL, threadFunction, (void *) data) != 0) {
                    vt__log_writer(LOG_CRITICAL, "Error creating a new thread");
                    exit(EXIT_FAILURE);
                } else {
                    vt__log_writer(LOG_INFO, "New thread created for handling messages from Client: ");
                    vt__log_writer(LOG_INFO, "Client connection details:");
                    char client_index_str[20];
                    sprintf(client_index_str, "%d", i);
                    vt__log_writer(LOG_INFO, client_index_str);
                }

                pthread_join(thread, NULL);
                vt__log_writer(LOG_INFO, "Back to Main Thread While Loop for checking new incoming messages");
            }
        } 
    }

    printf("** Server stopped **\n");

}