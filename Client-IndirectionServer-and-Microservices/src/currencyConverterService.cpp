/*
    Author: Azlan Amjad
    Assignment: A2, CPSC 441
    Date Created: October 20th, 2021
*/

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <unordered_map>
#include <string>
#include <stdlib.h>
#include <string.h>

using namespace std;

#define SERVER_PORT 8002
#define MAX_MESSAGE 2048

int main()
{
    struct sockaddr_in server, client;
    int converter_sockfd;
    int addr_len = sizeof(struct sockaddr_in);

    char conversion_request[MAX_MESSAGE];
    char conversion_result[MAX_MESSAGE];
    char value[MAX_MESSAGE];
    char currency[MAX_MESSAGE];
    char convert[MAX_MESSAGE];

    unordered_map<string, float> CAD;
    CAD["USD"] = 0.81;
    CAD["EUR"] = 0.70;
    CAD["GBP"] = 0.59;
    CAD["BTC"] = 0.000012;

    // server address initialization
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // UDP socket creation
    converter_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (converter_sockfd == -1)
    {
        cout << "as converter server: socket() system call failed" << endl;
        exit(1);
    }

    // bind IP address and port to end point (translator_sockfd)
    if (bind(converter_sockfd, (struct sockaddr *)&server, addr_len) == -1)
    {
        cout << "as converter server: bind() system call failed" << endl;
        exit(1);
    }
    cout << "UDP currency converter server is ready to receive information" << endl;

    while (1)
    {
        bzero(conversion_request, MAX_MESSAGE);
        bzero(conversion_result, MAX_MESSAGE);

        // receive conversion request
        if (recvfrom(converter_sockfd, conversion_request, MAX_MESSAGE, 0, (struct sockaddr *)&client, (socklen_t *)&addr_len) == -1)
        {
            cout << "as converter server receiving from client: recvfrom() system call failed" << endl;
            exit(1);
        }

        // WILL BE IN THIS FORMAT
        // [(value) (current currency) (currency to convert to)]
        cout << "currency converter server received: " << conversion_request << endl;

        // parse the request and return either the conversion or an error message
        sscanf(conversion_request, "%s %s %s", value, currency, convert);
        cout << "value: " << value << endl;
        cout << "currency: " << currency << endl;
        cout << "convert: " << convert << endl;

        while (1)
        {
            // if value is 0
            float val;
            if (strcmp(value, "0") == 0)
            {
                val = 0;
            }
            // value
            else
            {
                val = atoi(value);

                // if value has equaled 0 we have error
                if (val == 0)
                {
                    strcpy(conversion_result, "invalid input");
                    break;
                }
            }

            // currency
            // not converting CAD currency
            if (strcmp(currency, "CAD") != 0)
            {
                strcpy(conversion_result, "can not convert currency other than CAD");
                break;
            }

            // convert
            string key(convert);
            unordered_map<string, float>::iterator search = CAD.find(key);
            if (search == CAD.end())
            {
                strcpy(conversion_result, "could not find conversion rate for specified currency");
                break;
            }
            else
            {
                val *= search->second;
                string temp = to_string(val) + " " + convert;
                strcpy(conversion_result, temp.c_str());
                break;
            }
        }

        // send converted currency
        if (sendto(converter_sockfd, conversion_result, strlen(conversion_result), 0, (struct sockaddr *)&client, addr_len) == -1)
        {
            cout << "as converter server sending translated word: sendto() system call failed" << endl;
            exit(1);
        }
    }
}