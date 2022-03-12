// Author: Azlan Amjad
// Assignment: A1
// Date: September 27th, 2021

#include <iostream>
#include <cstdio>
#include <string.h>
#include <vector>
#include <ctype.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <fcntl.h>

using namespace std;

#define PROXYPORT 8080
#define HTTP_PORT 80
#define MAX_MESSAGE 2048
#define MAX_HTMLPAGE 20000
#define MAX_STRING 30
#define MAX_BLOCKS 3

void parseHTTPrequest(char* request, char* url, char* host, char* path);

int main() {
    // sockaddr structures for server and client
    struct sockaddr_in client, proxy, server;
    int server_parentsockfd, server_childsockfd, client_sockfd;
    int config_parentsockfd, config_childsockfd;
    int addrlen = sizeof(struct sockaddr_in);
    int clientbytes, serverbytes, requestbytes;
    int pid;

    // block requests
    char block_request[MAX_MESSAGE], request[MAX_MESSAGE];
    vector<string> blocked_words_array;
    int blocked_words = 0;

    // HTTP error request
    char http_error_request[] = "GET http://pages.cpsc.ucalgary.ca/~carey/CPSC441/ass1/error.html HTTP/1.1\r\nHOST: pages.cpsc.ucalgary.ca\r\n\r\n";

    // HTTP request received from client
    char client_http_request[MAX_MESSAGE];
    char host[MAX_MESSAGE], path[MAX_MESSAGE], url[MAX_MESSAGE];

    // HTTP request sent to server
    char request_to_server[MAX_MESSAGE];

    // HTTP response received from server
    char server_http_response[MAX_MESSAGE+MAX_HTMLPAGE];

    //char response_header[MAX_MESSAGE], response_body[MAX_HTMLPAGE];
    //char content_length[MAX_MESSAGE], content_type[MAX_MESSAGE];

    // HTTP response sent to client
    //char response_to_client[MAX_MESSAGE+MAX_HTMLPAGE];

    //char header[MAX_MESSAGE];
    //char temp[MAX_MESSAGE];



// ============================================== AS SERVER =====================================================
    cout << "==================================== AS SERVER =================================" << endl << endl;
    // PREPARE CONFIG SOCKET
    memset(&proxy, 0, sizeof(proxy));
    proxy.sin_family = AF_INET;
    proxy.sin_port = htons(PROXYPORT+1);
    proxy.sin_addr.s_addr = htonl(INADDR_ANY);

    config_parentsockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (config_parentsockfd == -1) {
        cout << "as server (config): socket() system call failed" << endl;
        exit(1);
    }

    if (bind(config_parentsockfd, (struct sockaddr *)&proxy, sizeof(struct sockaddr_in)) == -1) {
        cout << "as server (config): bind() system call failed" << endl;
        exit(1);
    }

    if (listen(config_parentsockfd, 1) == -1) {
        cout << "as server (config): listen() system call failed" << endl;
        exit(1);
    }

    cout << "PLEASE CONNECT TO PROXY ON PORT: " << PROXYPORT+1 << ", TO DYNAMICALLY CONFIGURE" << endl;
    cout << "PROXY WILL NOT PROCEED UNTIL YOU CONNECT" << endl << endl;
    config_childsockfd = accept(config_parentsockfd, 0, 0);
    if (config_childsockfd == -1) {
        cout << "as server (config): accept() system call failed" << endl;
        exit(1);
    }

    // set config socket as non blocking
    fcntl(config_childsockfd, F_SETFL, O_NONBLOCK);
    
    

    // ADDRESS INITIALIZATION - as server
    memset(&proxy, 0, sizeof(proxy));
    proxy.sin_family = AF_INET;
    proxy.sin_port = htons(PROXYPORT);
    proxy.sin_addr.s_addr = htonl(INADDR_ANY);
    cout << "PROXY ADDRESS INITIALIZED" << endl << endl;

    // SOCKET CREATION - as server
    server_parentsockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_parentsockfd == -1) {
        cout << "as server: socket() system call failed" << endl;
        exit(1);
    }
    cout << "WEB -> PROXY SOCKET CREATED" << endl << endl;

    // BINDING - as server
    if (bind(server_parentsockfd, (struct sockaddr *)&proxy, sizeof(struct sockaddr_in)) == -1) {
        cout << "as server: bind() system call failed" << endl;
        exit(1);
    }
    cout << "DONE BINDING SOCKET TO PROXY ADDRESS" << endl << endl;

    // LISTENING - as server
    if (listen(server_parentsockfd, 5) == -1) {
        cout << "as server: listen() system call failed" << endl;
        exit(1);
    }
    cout << "LISTENING FOR CLIENTS" << endl << endl;



    // Server/Client loops forever
    // will continue listening for requests forever
    cout << "================================== STARTING LOOP ===============================" << endl << endl;
    while(1) {
        cout << "TOP OF LOOP" << endl << endl;

        // CONNECTION ACCEPTANCE - as server
        server_childsockfd = accept(server_parentsockfd, (struct sockaddr *)&client, (socklen_t *)&addrlen);
        if (server_childsockfd == -1) {
            cout << "as server: accept() system call failed" << endl;
            exit(1);
        }
        cout << "ACCEPTED A CLIENT" << endl << endl;

        // check if there is a dynamic config request
        bzero(block_request, MAX_MESSAGE);
        if ((requestbytes = recv(config_childsockfd, block_request, MAX_MESSAGE, 0)) > 0) {
            // read each line of block request
            for (const char* line = strtok(block_request, "\r\n"); line != NULL; line = strtok(NULL, "\n")) {
                cout << "CONFIG REQUEST: " << line << endl;
                
                // copy out line
                bzero(request, MAX_MESSAGE);
                strcpy(request, line);

                // check for BLOCK and FREE request
                char* block_option = strstr(request, "BLOCK");
                char* free_option = strstr(request, "FREE");
                
                // if it is a BLOCK
                if (block_option == request) {
                    cout << "THIS IS A BLOCK REQUEST" << endl;
                    // cant block
                    if (blocked_words == MAX_BLOCKS) {
                        cout << "SORRY CANT BLOCK ANYMORE WORDS" << endl;
                    }
                    // block
                    else {
                        cout << "BLOCKING..." << endl;
                        char* word = request + 6;
                        string block(word);
                        blocked_words_array.push_back(word);
                        blocked_words++;
                    }
                }
                // if it is a FREE
                // NEED TO IMPLEMENT
                else if (free_option == request) {
                    cout << "THIS IS A FREE REQUEST" << endl;
                    char* word = request + 5;
                    string free(word);
                    vector<string>::iterator it = find(blocked_words_array.begin(), blocked_words_array.end(), free);
                    if (it != blocked_words_array.end()) {
                        blocked_words_array.erase(it);
                        blocked_words--;
                    }
                }
            }
            cout << "END CONFIG IF" << endl;
        }
        cout << "BLOCKED WORDS: " << endl;
        for (vector<string>::iterator it = blocked_words_array.begin(); it != blocked_words_array.end(); it++) {
            cout << *it << endl;
        }

        pid = fork();
        if (pid < 0) {
            cout << "as server: fork() system call failed" << endl;
            exit(1);
        }
        else if (pid > 0) {
            // PARENT PROCESS
            // parent process continues to listen
            // created child process for that request, close child socket
            //cout << "closing child socket in parent process" << endl << endl;
            close(server_childsockfd);
        }
        else {
            // CHILD PROCESS
            // close inherited parent socket
            //cout << "closing parent socket in child process" << endl << endl;
            close(server_parentsockfd);
            // close the config child sock as well, we dont need it here
            close(config_childsockfd);
// ====================================================== AS SERVER =====================================================
            cout << "==================================== AS SERVER =================================" << endl << endl;
            // receive the HTTP request and parse it to extract the needed information
            // RECEIVE HTTP REQUEST FROM CLIENT - as server
            bzero(client_http_request, MAX_MESSAGE);
            clientbytes = recv(server_childsockfd, client_http_request, MAX_MESSAGE, 0);
            if (clientbytes == -1) {
                cout << "as server receiving HTTP request: recv() system call failed" << endl;
                exit(1);
            }
            // copy out the http request before we start playing with it
            bzero(request_to_server, MAX_MESSAGE);
            strcpy(request_to_server, client_http_request);

            cout << "Read first " << clientbytes << " bytes from client HTTP request" << endl;
            
            // fill in host, path and url
            bzero(url, MAX_MESSAGE);
            bzero(host, MAX_MESSAGE);
            bzero(path, MAX_MESSAGE);
            parseHTTPrequest(client_http_request, url, host, path);
            cout << "URL: " << url << endl;
            cout << "HOST: " << host << endl;
            cout << "PATH: " << path << endl << endl;

            // see if we need to block this url
            char temp_url[MAX_MESSAGE];
            bzero(temp_url, MAX_MESSAGE);
            strcpy(temp_url, url);
            bool block = false;
            cout << blocked_words << endl;
            for (vector<string>::iterator it = blocked_words_array.begin(); it != blocked_words_array.end(); it++) {
                cout << *it << endl;
                const char* word = (*it).c_str();
                const char* blocked_words = strstr(temp_url, word);
                if (blocked_words != NULL) {
                    block = true;
                    cout << "THIS URL NEEDS TO BE BLOCKED!" << endl;
                }
            }
            
// ====================================================== AS CLIENT ======================================================
            cout << "==================================== AS CLIENT =================================" << endl << endl;
            // prepare a new socket for connecting to the web server
            // ADDRESS INITIALIZATION - as client
            struct hostent *server_host;
            server_host = gethostbyname(host);

            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_port = htons(HTTP_PORT);
            bcopy((char*)server_host->h_addr, (char*)&server.sin_addr.s_addr, server_host->h_length);
            cout << "SERVER ADDRESS INITIALIZED" << endl << endl;

            // SOCKET CREATION - as client
            client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (client_sockfd == -1) {
                cout << "as client: socket() system call failed" << endl;
                exit(1);
            }
            cout << "PROXY -> SERVER SOCKET CREATED" << endl << endl;

            // CONNECTION REQUEST - as client
            if (connect(client_sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
                cout << "as client: connect() system call failed" << endl;
                exit(1);
            }
            cout << "CONNECTED TO SERVER" << endl << endl;

            // SEND HTTP REQUEST TO SERVER - as client
            if (block) {
                if (send(client_sockfd, http_error_request, strlen(http_error_request), 0) == -1) {
                    cout << "as client sending HTTP request: send() system call failed" << endl;
                    exit(1);
                }
                cout << "HTTP REQUEST SENT TO SERVER" << endl << endl;
            }
            else {
                if (send(client_sockfd, request_to_server, strlen(request_to_server), 0) == -1) {
                    cout << "as client sending HTTP request: send() system call failed" << endl;
                    exit(1);
                }
                cout << "HTTP REQUEST SENT TO SERVER" << endl << endl;
            }

            // RECEIVE HTTP RESPONSE FROM SERVER - as client
            bzero(server_http_response, MAX_MESSAGE+MAX_HTMLPAGE);
            while ((serverbytes = recv(client_sockfd, server_http_response, MAX_MESSAGE+MAX_HTMLPAGE, 0)) > 0) {
                cout << "Read first " << serverbytes << " bytes from server HTTP response" << endl;
                cout << "HTTP RESPONSE RECEIVED FROM SERVER" << endl << endl;

// ====================================================== AS SERVER ======================================================
                // send the HTTP response to the client
                // in loop because we can send large data in chunks
                if (send(server_childsockfd, server_http_response, serverbytes, 0) == -1) {
                    cout << "as server sending HTTP response to client: send() system call failed" << endl;
                }
                cout << "HTTP RESPONSE SENT TO CLIENT" << endl << endl;
            }
            if (serverbytes == -1) {
                cout << "as client receiving HTTP response: recv() system call failed" << endl;
                exit(1);
            }

            // close the socket connected to server
            close(client_sockfd);
            // at the end close the server child socket
            close(server_childsockfd);
            // exit the child process
            // we dont want to try and accept clients from the child
            exit(0);
        }
    }
}

void parseHTTPrequest(char* request, char* url, char* host, char* path) {
    // request line
    char* line = strtok(request, "\r\n");
    cout << line << endl;

    // get to second token
    char* token = strtok(line, " ");
    token = strtok(NULL, " ");

    // copy part after "http://" into url
    sscanf(token, "http://%s", url);

    // copy host from the url
    int i;
    for (i = 0; i < strlen(url); i++) {
        if (url[i] == '/') {
            break;
        }
    }
    strcpy(host, url);
    host[i] = '\0';

    // copy path name from url
    const char *temp = url + i;
    strcpy(path, temp);
}
