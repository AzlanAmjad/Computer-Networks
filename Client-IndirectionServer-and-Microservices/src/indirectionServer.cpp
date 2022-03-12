/*
    Author: Azlan Amjad
    Assignment: A2, CPSC 441
    Date Created: October 19th, 2021
*/

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sys/time.h>
#include <string.h>

using namespace std;

#define SERVER_PORT 8080
#define MAX_MESSAGE 2048

void get_server_addr(uint16_t &port, char *IP);

int main()
{
    // variables
    int server_parentsockfd, server_childsockfd, service_sockfd;
    struct sockaddr_in indirection_server, client, translator, converter, voting;
    int addr_len = sizeof(struct sockaddr_in);
    int pid;
    int readbytes;

    char temp[MAX_MESSAGE];

    char service_choice[MAX_MESSAGE];

    // translation
    char word_to_translate[MAX_MESSAGE];
    char translated_word[MAX_MESSAGE];
    char translation_response[MAX_MESSAGE];

    // conversion
    char conversion_request[MAX_MESSAGE];
    char converted_currency[MAX_MESSAGE];
    char conversion_response[MAX_MESSAGE];

    // voting
    char init_choice[MAX_MESSAGE];
    char result[MAX_MESSAGE];
    char voting_response[MAX_MESSAGE];
    char key[MAX_MESSAGE];
    char vote[MAX_MESSAGE];

    uint16_t translator_port, converter_port, voting_port;
    char translator_IP[MAX_MESSAGE], converter_IP[MAX_MESSAGE], voting_IP[MAX_MESSAGE];

    cout << "\nin the following steps please let the server know the address of each micro service, thank you." << endl;

    // initialize address of translator service
    cout << "\nplease enter the server address (port number and IP address) of the translator service below" << endl;
    get_server_addr(translator_port, translator_IP);

    memset(&translator, 0, sizeof(translator));
    translator.sin_family = AF_INET;
    translator.sin_port = htons(translator_port);
    translator.sin_addr.s_addr = inet_addr(translator_IP);

    // initialize address of currency converter service
    cout << "\nplease enter the server address (port number and IP address) of the currency converter service below" << endl;
    get_server_addr(converter_port, converter_IP);

    memset(&converter, 0, sizeof(converter));
    converter.sin_family = AF_INET;
    converter.sin_port = htons(converter_port);
    converter.sin_addr.s_addr = inet_addr(converter_IP);

    // initialize address of voting service
    cout << "\nplease enter the server address (port number and IP address) of the voting service below" << endl;
    get_server_addr(voting_port, voting_IP);

    memset(&voting, 0, sizeof(voting));
    voting.sin_family = AF_INET;
    voting.sin_port = htons(voting_port);
    voting.sin_addr.s_addr = inet_addr(voting_IP);

    // UDP socket creation for service
    service_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (service_sockfd == -1)
    {
        cout << "as indirection server creating service UDP socket: socket() system call failed" << endl;
        exit(1);
    }

    // set 1 second time out on UDP socket
    struct timeval tv;
    tv.tv_sec = 1;
    if (setsockopt(service_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
    {
        cout << "error in setting timeout for UDP service socket" << endl;
        exit(1);
    }

    // server address initialization
    memset(&indirection_server, 0, sizeof(indirection_server));
    indirection_server.sin_family = AF_INET;
    indirection_server.sin_port = htons(SERVER_PORT);
    indirection_server.sin_addr.s_addr = htonl(INADDR_ANY); // indirection server is running on local machine

    // TCP socket creation
    server_parentsockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_parentsockfd == -1)
    {
        cout << "as indirection server: socket() system call failed" << endl;
        exit(1);
    }

    // bind IP address and port to end point (server_parentsockfd)
    if (bind(server_parentsockfd, (struct sockaddr *)&indirection_server, addr_len) == -1)
    {
        cout << "as indirection server: bind() system call failed" << endl;
        exit(1);
    }

    // listen for incoming connections
    if (listen(server_parentsockfd, 1) == -1)
    {
        cout << "as indirection server: listen() system call failed" << endl;
        exit(1);
    }
    cout << "\nindirection server is waiting for connection..." << endl;

    // while loop to interact with clients and micro service servers
    while (1)
    {
        // accept a client connection
        server_childsockfd = accept(server_parentsockfd, (struct sockaddr *)&client, (socklen_t *)&addr_len);
        if (server_childsockfd == -1)
        {
            cout << "as indirection server: accept() system call failed" << endl;
            exit(1);
        }
        cout << "\nconnected to client..." << endl;

        // forking allows the server to continue running even if client decides to quit
        pid = fork();
        if (pid < 0)
        {
            cout << "as indirection server: fork() system call failed" << endl;
            exit(1);
        }
        else if (pid > 0)
        {
            // PARENT PROCESS
            // parent process continues to listen
            // created child process for that request, close child socket
            // cout << "closing child socket in parent process" << endl << endl;
            close(server_childsockfd);
        }
        else
        {
            // CHILD PROCESS
            close(server_parentsockfd);
            // while loop for client to interact with server
            while (1)
            {
                /*
                    receive client command in a specified format
                    parse the command
                    send the parsed inputs to the desired micro service via UDP socket
                    recieve response from micro service
                    send response back to client
                */

                bzero(service_choice, MAX_MESSAGE);

                // receive client service choice
                readbytes = recv(server_childsockfd, service_choice, MAX_MESSAGE, 0);
                if (readbytes == -1)
                {
                    cout << "as indirection server receiving: recv() system call failed" << endl;
                    exit(1);
                }
                // client closed socket
                else if (readbytes == 0)
                {
                    cout << "client has decided to end session..." << endl;
                    cout << "ending client session..." << endl;
                    exit(0);
                }

                cout << "micro service choice: " << service_choice << endl;

                // if client chooses translator
                // ask client for required information
                // receive required information
                // send micro service required information
                // get response from microservice
                // send microservice response to client
                if (strcmp(service_choice, "1") == 0)
                {
                    cout << "client chose translator" << endl;

                    // while loop for translation Service
                    while (1)
                    {
                        bzero(word_to_translate, MAX_MESSAGE);
                        bzero(translated_word, MAX_MESSAGE);
                        bzero(translation_response, MAX_MESSAGE);
                        bzero(temp, MAX_MESSAGE);

                        // receive word to translate from client
                        if (recv(server_childsockfd, word_to_translate, MAX_MESSAGE, 0) == -1)
                        {
                            cout << "as indirection server receiving: recv() system call failed" << endl;
                            exit(1);
                        }

                        cout << "word to translate: " << word_to_translate << endl;

                        // send translator service word to translate
                        if (sendto(service_sockfd, word_to_translate, strlen(word_to_translate), 0, (struct sockaddr *)&translator, addr_len) == -1)
                        {
                            cout << "as indirection server sending: sendto() system call failed" << endl;
                            exit(1);
                        }

                        // get translated word from translator
                        readbytes = recvfrom(service_sockfd, translated_word, MAX_MESSAGE, 0, (struct sockaddr *)&translator, (socklen_t *)&addr_len);
                        // socket timed out
                        if (readbytes == EAGAIN || readbytes == EWOULDBLOCK)
                        {
                            strcpy(translated_word, "UDP socket timed out");
                        }
                        else if (readbytes == -1)
                        {
                            cout << "as indirection server receiving: recvfrom() system call failed" << endl;
                            exit(1);
                        }

                        cout << "translated word: " << translated_word << endl;

                        // send response to client
                        strcpy(translation_response, "\nfrench translation: ");
                        strcat(translation_response, translated_word);
                        strcat(translation_response, "\n\n");
                        strcat(translation_response, "(type 'exit' to exit or any letter to continue): ");
                        if (send(server_childsockfd, translation_response, strlen(translation_response), 0) == -1)
                        {
                            cout << "as indirection server sending translated word: send() system call failed" << endl;
                            exit(1);
                        }

                        // wait for client to continue
                        if (recv(server_childsockfd, temp, MAX_MESSAGE, 0) == -1)
                        {
                            cout << "as indirection server receiving continue: recv() system call failed" << endl;
                            exit(1);
                        }
                        // see if user wants to exit
                        if (strcmp(temp, "exit") == 0)
                        {
                            break;
                        }
                    }
                }

                // if client chooses currency converter
                // ask client for required information
                // receive required information
                // send micro service required information
                // get response from microservice
                // send microservice response to client
                else if (strcmp(service_choice, "2") == 0)
                {
                    cout << "client chose currency converter" << endl;

                    // while loop for Converting Service
                    while (1)
                    {
                        bzero(conversion_request, MAX_MESSAGE);
                        bzero(converted_currency, MAX_MESSAGE);
                        bzero(conversion_response, MAX_MESSAGE);
                        bzero(temp, MAX_MESSAGE);

                        // receive conversion request from client
                        if (recv(server_childsockfd, conversion_request, MAX_MESSAGE, 0) == -1)
                        {
                            cout << "as indirection server receiving: recv() system call failed" << endl;
                            exit(1);
                        }

                        cout << "conversion request: " << conversion_request << endl;

                        // send currency converter service conversion request
                        if (sendto(service_sockfd, conversion_request, strlen(conversion_request), 0, (struct sockaddr *)&converter, addr_len) == -1)
                        {
                            cout << "as indirection server sending: sendto() system call failed" << endl;
                            exit(1);
                        }

                        // get converted currency from Converter
                        readbytes = recvfrom(service_sockfd, converted_currency, MAX_MESSAGE, 0, (struct sockaddr *)&converter, (socklen_t *)&addr_len);
                        // socket timed out
                        if (readbytes == EAGAIN || readbytes == EWOULDBLOCK)
                        {
                            strcpy(converted_currency, "UDP socket timed out");
                        }
                        else if (readbytes == -1)
                        {
                            cout << "as indirection server receiving: recvfrom() system call failed" << endl;
                            exit(1);
                        }

                        cout << "converted currency: " << converted_currency << endl;

                        // send response to client
                        strcpy(conversion_response, "\nconverted currency: ");
                        strcat(conversion_response, converted_currency);
                        strcat(conversion_response, "\n\n");
                        strcat(conversion_response, "(type 'exit' to exit or any letter to continue): ");
                        if (send(server_childsockfd, conversion_response, strlen(conversion_response), 0) == -1)
                        {
                            cout << "as indirection server sending: send() system call failed" << endl;
                            exit(1);
                        }

                        // wait for client to continue
                        if (recv(server_childsockfd, temp, MAX_MESSAGE, 0) == -1)
                        {
                            cout << "as indirection server receiving continue: recv() system call failed" << endl;
                            exit(1);
                        }
                        // see if user wants to exit
                        if (strcmp(temp, "exit") == 0)
                        {
                            break;
                        }
                    }
                }

                // if client chooses voting
                // ask client for required information
                // receive required information
                // send micro service required information
                // get response from microservice
                // send microservice response to client
                else if (strcmp(service_choice, "3") == 0)
                {
                    cout << "client chose voting" << endl;

                    // while loop for voting service
                    while (1)
                    {
                        bzero(init_choice, MAX_MESSAGE);
                        bzero(result, MAX_MESSAGE);
                        bzero(voting_response, MAX_MESSAGE);
                        bzero(temp, MAX_MESSAGE);

                        bzero(key, MAX_MESSAGE);
                        bzero(vote, MAX_MESSAGE);

                        // receive voting service choice from client
                        if (recv(server_childsockfd, init_choice, MAX_MESSAGE, 0) == -1)
                        {
                            cout << "as indirection server receiving: recv() system call failed" << endl;
                            exit(1);
                        }
                        cout << "voting service choice: " << init_choice << endl;

                        // send this choice to the voting server
                        if (sendto(service_sockfd, init_choice, strlen(init_choice), 0, (struct sockaddr *)&voting, addr_len) == -1)
                        {
                            cout << "as indirection server sending: sendto() system call failed" << endl;
                            exit(1);
                        }

                        // show candidates
                        if (strcmp(init_choice, "1") == 0)
                        {
                            // receive results from voting server
                            readbytes = recvfrom(service_sockfd, result, MAX_MESSAGE, 0, (struct sockaddr *)&voting, (socklen_t *)&addr_len);
                            // socket timed out
                            if (readbytes == EAGAIN || readbytes == EWOULDBLOCK)
                            {
                                strcpy(result, "UDP socket timed out\n");
                            }
                            else if (readbytes == -1)
                            {
                                cout << "as indirection server receiving: recvfrom() system call failed" << endl;
                                exit(1);
                            }

                            cout << "candidates result: " << result;

                            // send results back to the client
                            strcpy(voting_response, "\nresult:\n");
                            strcat(voting_response, result);
                            strcat(voting_response, "\n");
                            strcat(voting_response, "(type 'exit' to exit or any letter to continue): ");
                            if (send(server_childsockfd, voting_response, strlen(voting_response), 0) == -1)
                            {
                                cout << "as indirection server sending: send() system call failed" << endl;
                                exit(1);
                            }
                        }
                        // secure voting
                        else if (strcmp(init_choice, "2") == 0)
                        {
                            // receive message with encryption key
                            readbytes = recvfrom(service_sockfd, key, MAX_MESSAGE, 0, (struct sockaddr *)&voting, (socklen_t *)&addr_len);
                            // socket timed out
                            if (readbytes == EAGAIN || readbytes == EWOULDBLOCK)
                            {
                                // key will be returned as 0 if socket timed out
                                strcpy(key, "0");
                            }
                            else if (readbytes == -1)
                            {
                                cout << "as indirection server receiving: recvfrom() system call failed" << endl;
                                exit(1);
                            }

                            // send encryption key
                            if (send(server_childsockfd, key, strlen(key), 0) == -1)
                            {
                                cout << "as indirection server sending: send() system call failed" << endl;
                                exit(1);
                            }

                            // receive vote from client
                            if (recv(server_childsockfd, vote, MAX_MESSAGE, 0) == -1)
                            {
                                cout << "as indirection server receiving: recv() system call failed" << endl;
                                exit(1);
                            }

                            cout << "client has voted for: " << vote << endl;

                            // send encrypted vote to voting service
                            if (sendto(service_sockfd, vote, strlen(vote), 0, (struct sockaddr *)&voting, addr_len) == -1)
                            {
                                cout << "as indirection server sending: sendto() system call failed" << endl;
                                exit(1);
                            }

                            // receive vote confirmation
                            readbytes = recvfrom(service_sockfd, result, MAX_MESSAGE, 0, (struct sockaddr *)&voting, (socklen_t *)&addr_len);
                            // socket timed out
                            if (readbytes == EAGAIN || readbytes == EWOULDBLOCK)
                            {
                                strcpy(result, "UDP socket timed out");
                            }
                            if (readbytes == -1)
                            {
                                cout << "as indirection server receiving: recvfrom() system call failed" << endl;
                                exit(1);
                            }
                            
                            cout << "result of voting: " << result << endl;

                            // send vote confirmation back to client
                            strcpy(voting_response, "\nresult: ");
                            strcat(voting_response, result);
                            strcat(voting_response, "\n");
                            strcat(voting_response, "(type 'exit' to exit or any letter to continue): ");
                            if (send(server_childsockfd, voting_response, strlen(voting_response), 0) == -1)
                            {
                                cout << "as indirection server sending: send() system call failed" << endl;
                                exit(1);
                            }
                        }
                        // voting summary
                        else if (strcmp(init_choice, "3") == 0)
                        {
                            // receive results from voting server
                            readbytes = recvfrom(service_sockfd, result, MAX_MESSAGE, 0, (struct sockaddr *)&voting, (socklen_t *)&addr_len);
                            // socket timed out
                            if (readbytes == EAGAIN || readbytes == EWOULDBLOCK)
                            {
                                strcpy(result, "UDP socket timed out\n");
                            }
                            if (readbytes == -1)
                            {
                                cout << "as indirection server receiving: recvfrom() system call failed" << endl;
                                exit(1);
                            }

                            cout << "voting result: " << result;

                            // send results back to the client
                            strcpy(voting_response, "\nresult:\n");
                            strcat(voting_response, result);
                            strcat(voting_response, "\n");
                            strcat(voting_response, "(type 'exit' to exit or any letter to continue): ");
                            if (send(server_childsockfd, voting_response, strlen(voting_response), 0) == -1)
                            {
                                cout << "as indirection server sending: send() system call failed" << endl;
                                exit(1);
                            }
                        }
                        else
                        {
                            strcpy(voting_response, "\nInvalid choice.");
                            strcat(voting_response, "\n\n");
                            strcat(voting_response, "(type 'exit' to exit or any letter to continue): ");
                            if (send(server_childsockfd, voting_response, strlen(voting_response), 0) == -1)
                            {
                                cout << "as indirection server sending: send() system call failed" << endl;
                                exit(1);
                            }
                        }

                        // wait for client to continue
                        if (recv(server_childsockfd, temp, MAX_MESSAGE, 0) == -1)
                        {
                            cout << "as indirection server receiving continue: recv() system call failed" << endl;
                            exit(1);
                        }
                        // see if user wants to exit
                        if (strcmp(temp, "exit") == 0)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

void get_server_addr(uint16_t &port, char *IP)
{
    cout << "please enter the port number of the service (number between 8000 to 9000): ";
    cin >> port;
    cout << "please enter the IP address of the service (XXX.XXX.XXX.XXX) (127.0.0.1 if server is on local host): ";
    cin >> IP;
}