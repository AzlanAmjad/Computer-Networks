/*
    Author: Azlan Amjad
    Assignment: A2, CPSC 441
    Date Created: October 19th, 2021
*/

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <regex>
#include <string.h>

using namespace std;

#define MAX_MESSAGE 2048

void get_server_addr(uint16_t &port, char *IP);

int encrypt(int ID, int key);

int main()
{
    // variables
    int client_sockfd;
    struct sockaddr_in server;
    uint16_t server_port;
    char server_IP[MAX_MESSAGE];

    int readBytes;
    char service_menu[] = "\nchoose a service (1) translator, (2) currency converter, (3) voting, (1, 2, 3, or exit): ";
    char service_choice[MAX_MESSAGE];

    char prompt[MAX_MESSAGE];
    char prompt_reply[MAX_MESSAGE];
    char result[MAX_MESSAGE];
    char cont[MAX_MESSAGE];

    // translation
    char translation_prompt[] = "\n(1) enter an english word: ";

    // conversion
    char value[MAX_MESSAGE];
    char currency[MAX_MESSAGE];
    char convert[MAX_MESSAGE];
    char conversion_prompt[] = "\n(2) enter conversion request [(value) (current currency) (currency to convert to)]: ";

    // voting
    char voting_prompt[MAX_MESSAGE];
    char voting_choice[MAX_MESSAGE];
    bool has_voted = false;
    char init_voting_prompt[] = "\n(3) please choose a command, (1) show candidates, (2) secure voting, (3) voting summary, (1, 2, or 3): ";
    char key[MAX_MESSAGE];
    char secure_voting_prompt[] = "\n(3)(2) please enter the ID of the candidate you wish to vote for: ";
    char vote[MAX_MESSAGE];
    char voting_summary_error[] = "\nresult: you have not voted yet, unable to see voting summary\n\n(type 'exit' to exit or any letter to continue): ";
    int encrypt_key;
    int ID;

    cout << endl;
    get_server_addr(server_port, server_IP);

    cout << "server port: " << server_port << endl;
    cout << "server IP: " << server_IP << endl;

    // address initialization
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_IP);

    // tCP socket creation
    client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sockfd == -1)
    {
        cout << "as client: socket() system call failed" << endl;
        exit(1);
    }

    // connection request to server
    if (connect(client_sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1)
    {
        cout << "as client: connect() system call failed" << endl;
        exit(1);
    }
    cout << "connected to server..." << endl;

    // loop for continuous interaction with indirection server
    while (1)
    {
        bzero(prompt, MAX_MESSAGE);
        bzero(prompt_reply, MAX_MESSAGE);
        bzero(result, MAX_MESSAGE);
        bzero(cont, MAX_MESSAGE);

        bzero(value, MAX_MESSAGE);
        bzero(currency, MAX_MESSAGE);
        bzero(convert, MAX_MESSAGE);

        cout << service_menu;

        cin >> service_choice;

        while (getchar() != '\n')
        {
        };

        // send service choice to server
        if (send(client_sockfd, service_choice, strlen(service_choice), 0) == -1)
        {
            cout << "as client sending message: send() system call failed" << endl;
        }

        if (strcmp(service_choice, "1") == 0)
        {
            // translation service loop
            while (1)
            {
                bzero(prompt, MAX_MESSAGE);
                bzero(prompt_reply, MAX_MESSAGE);
                bzero(result, MAX_MESSAGE);
                bzero(cont, MAX_MESSAGE);

                cout << translation_prompt;
                cin >> prompt_reply;
                while (getchar() != '\n')
                {
                };

                // send a response to the server
                if (send(client_sockfd, prompt_reply, strlen(prompt_reply), 0) == -1)
                {
                    cout << "as client sending message: send() system call failed" << endl;
                }

                // receive results
                readBytes = recv(client_sockfd, result, MAX_MESSAGE, 0);
                if (readBytes == -1)
                {
                    cout << "as client receiving message: recv() system call failed" << endl;
                    exit(1);
                }

                cout << result;

                // let server know if we want to continue or quit this service session

                cin >> cont;
                while (getchar() != '\n')
                {
                };

                // send a response to the server
                if (send(client_sockfd, cont, strlen(cont), 0) == -1)
                {
                    cout << "as client sending message: send() system call failed" << endl;
                }
                if (strcmp(cont, "exit") == 0)
                {
                    break; // break because server will now be looking for clinet to make service choice
                }
            }
        }
        else if (strcmp(service_choice, "2") == 0)
        {
            // conversion service loop
            while (1)
            {
                bzero(prompt, MAX_MESSAGE);
                bzero(prompt_reply, MAX_MESSAGE);
                bzero(result, MAX_MESSAGE);
                bzero(cont, MAX_MESSAGE);

                bzero(value, MAX_MESSAGE);
                bzero(currency, MAX_MESSAGE);
                bzero(convert, MAX_MESSAGE);

                cout << conversion_prompt;
                cin >> value >> currency >> convert;
                while (getchar() != '\n')
                {
                };

                // set up format for conversion Request
                strcpy(prompt_reply, value);
                strcat(prompt_reply, " ");
                strcat(prompt_reply, currency);
                strcat(prompt_reply, " ");
                strcat(prompt_reply, convert);

                // send a response to the server
                if (send(client_sockfd, prompt_reply, strlen(prompt_reply), 0) == -1)
                {
                    cout << "as client sending message: send() system call failed" << endl;
                }

                // receive results
                readBytes = recv(client_sockfd, result, MAX_MESSAGE, 0);
                if (readBytes == -1)
                {
                    cout << "as client receiving message: recv() system call failed" << endl;
                    exit(1);
                }

                cout << result;

                // let server know if we want to continue or quit this service session

                cin >> cont;
                while (getchar() != '\n')
                {
                };

                // send a response to the server
                if (send(client_sockfd, cont, strlen(cont), 0) == -1)
                {
                    cout << "as client sending message: send() system call failed" << endl;
                }
                if (strcmp(cont, "exit") == 0)
                {
                    break; // break because server will now be looking for clinet to make service choice
                }
            }
        }
        else if (strcmp(service_choice, "3") == 0)
        {
            // voting service loop
            while (1)
            {
                bzero(voting_prompt, MAX_MESSAGE);
                bzero(voting_choice, MAX_MESSAGE);
                bzero(prompt, MAX_MESSAGE);
                bzero(prompt_reply, MAX_MESSAGE);
                bzero(result, MAX_MESSAGE);
                bzero(cont, MAX_MESSAGE);

                bzero(key, MAX_MESSAGE);
                bzero(vote, MAX_MESSAGE);

                cout << init_voting_prompt;
                cin >> voting_choice;
                while (getchar() != '\n')
                {
                };

                // send service choice to server
                if (send(client_sockfd, voting_choice, strlen(voting_choice), 0) == -1)
                {
                    cout << "as client sending message: send() system call failed" << endl;
                }

                // show candidates
                if (strcmp(voting_choice, "1") == 0)
                {
                    // receive results for show candidates
                    readBytes = recv(client_sockfd, result, MAX_MESSAGE, 0);
                    if (readBytes == -1)
                    {
                        cout << "as client receiving message: recv() system call failed" << endl;
                        exit(1);
                    }

                    cout << result;
                }
                // secure voting
                else if (strcmp(voting_choice, "2") == 0)
                {
                    // receive encryption key
                    readBytes = recv(client_sockfd, key, MAX_MESSAGE, 0);
                    if (readBytes == -1)
                    {
                        cout << "as client receiving message: recv() system call failed" << endl;
                        exit(1);
                    }

                    // ask user to put in vote
                    cout << secure_voting_prompt;
                    cin >> prompt_reply;
                    while (getchar() != '\n')
                    {
                    };

                    // encrypt ID
                    encrypt_key = atoi(key);
                    ID = atoi(prompt_reply);

                    // send vote to server
                    if (has_voted)
                    {
                        ID = encrypt(-1, encrypt_key);
                        strcpy(vote, to_string(ID).c_str());
                    }
                    else
                    {
                        ID = encrypt(ID, encrypt_key);
                        strcpy(vote, to_string(ID).c_str());
                    }

                    if (send(client_sockfd, vote, strlen(vote), 0) == -1)
                    {
                        cout << "as client sending message: send() system call failed" << endl;
                    }

                    // receive vote confirmation
                    readBytes = recv(client_sockfd, result, MAX_MESSAGE, 0);
                    if (readBytes == -1)
                    {
                        cout << "as client receiving message: recv() system call failed" << endl;
                        exit(1);
                    }

                    const char *search = strstr(result, "vote has been successfully placed");
                    if (search != NULL)
                    {
                        has_voted = true;
                    }

                    cout << result;
                }
                // voting summary
                else if (strcmp(voting_choice, "3") == 0)
                {
                    // receive results for voting summary
                    readBytes = recv(client_sockfd, result, MAX_MESSAGE, 0);
                    if (readBytes == -1)
                    {
                        cout << "as client receiving message: recv() system call failed" << endl;
                        exit(1);
                    }

                    // only display the results if client has voted
                    if (has_voted)
                    {
                        cout << result;
                    }
                    else
                    {
                        cout << voting_summary_error;
                    }
                }
                else
                {
                    // receive error
                    readBytes = recv(client_sockfd, result, MAX_MESSAGE, 0);
                    if (readBytes == -1)
                    {
                        cout << "as client receiving message: recv() system call failed" << endl;
                        exit(1);
                    }

                    cout << result;
                }

                // let server know if we want to continue or quit this service session
                cin >> cont;
                while (getchar() != '\n')
                {
                };

                // send a response to the server
                if (send(client_sockfd, cont, strlen(cont), 0) == -1)
                {
                    cout << "as client sending message: send() system call failed" << endl;
                }
                if (strcmp(cont, "exit") == 0)
                {
                    break; // break because server will now be looking for clinet to make service choice
                }
            }
        }
        // client wants to end session
        else if (strcmp(service_choice, "exit") == 0)
        {
            cout << "ending client session..." << endl;
            close(client_sockfd);
            exit(0);
        }
        else
        {
            cout << "\ninvalid choice\n";
        }
    }

    close(client_sockfd);
    return 0;
}

void get_server_addr(uint16_t &port, char *IP)
{
    cout << "please enter the port number of the indirection server (number between 8000 to 9000): ";
    cin >> port;
    cout << "please enter the IP address of the indirection server (XXX.XXX.XXX.XXX) (127.0.0.1 if server is on local host): ";
    cin >> IP;
}

int encrypt(int ID, int key)
{
    return ID * key;
}