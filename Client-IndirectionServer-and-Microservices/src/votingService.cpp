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
#include <time.h>
#include <string.h>

using namespace std;

#define SERVER_PORT 8003
#define MAX_MESSAGE 2048

int decrypt(int ID, int key);

bool is_voting_open(time_t &end_UTC);

int main()
{
    struct sockaddr_in server, client;
    int voting_sockfd;
    int addr_len = sizeof(struct sockaddr_in);
    int encrypt_key = 4;
    int ID;

    char init_choice[MAX_MESSAGE];
    char result[MAX_MESSAGE];

    char key[MAX_MESSAGE];
    char vote[MAX_MESSAGE];

    // ID, name
    unordered_map<int, string> candidates;
    candidates[1003] = "Azlan Amjad";
    candidates[1002] = "LeBron James";
    candidates[1001] = "Walter White";
    candidates[1000] = "Stephen Curry";

    // ID, votes
    unordered_map<int, int> voting;
    voting[1003] = 122;
    voting[1002] = 78;
    voting[1001] = 58;
    voting[1000] = 70;

    // get current time
    time_t current_time_local = time(0);
    // convert current time to gmt time
    struct tm *voting_end_local = localtime(&current_time_local);

    int month, day, hour, minute;

    cout << "\nplease provide the date you want voting to end this year (local time)" << endl;
    cout << "month (0-11): ";
    cin >> month;
    while (getchar() != '\n')
    {
    };
    cout << "day (1-31): ";
    cin >> day;
    while (getchar() != '\n')
    {
    };
    cout << "hour (0-24): ";
    cin >> hour;
    while (getchar() != '\n')
    {
    };
    cout << "minute (0-60): ";
    cin >> minute;
    while (getchar() != '\n')
    {
    };
    cout << endl;

    voting_end_local->tm_mon = month;
    voting_end_local->tm_mday = day;
    voting_end_local->tm_hour = hour;
    voting_end_local->tm_min = minute;
    voting_end_local->tm_sec = 0;
    char *end_time_string = asctime(voting_end_local);

    cout << "voting will end (local time): " << end_time_string << endl;

    // convert to gmt
    time_t end_local = mktime(voting_end_local);
    struct tm *gmt_voting_end = gmtime(&end_local);
    end_time_string = asctime(gmt_voting_end);
    time_t end_UTC = mktime(gmt_voting_end);

    cout << "voting will end (UTC): " << end_time_string << endl;

    // server address initialization
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // UDP socket creation
    voting_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (voting_sockfd == -1)
    {
        cout << "as voting server: socket() system call failed" << endl;
        exit(1);
    }

    // bind IP address and port to end point (translator_sockfd)
    if (bind(voting_sockfd, (struct sockaddr *)&server, addr_len) == -1)
    {
        cout << "as voting server: bind() system call failed" << endl;
        exit(1);
    }
    cout << "UDP voting server is ready to receive information" << endl;

    while (1)
    {
        bzero(init_choice, MAX_MESSAGE);
        bzero(result, MAX_MESSAGE);

        bzero(key, MAX_MESSAGE);
        bzero(vote, MAX_MESSAGE);

        // receive service choice
        if (recvfrom(voting_sockfd, init_choice, MAX_MESSAGE, 0, (struct sockaddr *)&client, (socklen_t *)&addr_len) == -1)
        {
            cout << "as voting server receiving from client: recvfrom() system call failed" << endl;
            exit(1);
        }

        cout << "client chose service number: " << init_choice << endl;

        // show candidates
        if (strcmp(init_choice, "1") == 0)
        {
            // send candidates as results
            strcpy(result, "ID: name\n");
            unordered_map<int, string>::iterator search = candidates.begin();
            for (; search != candidates.end(); search++)
            {
                strcat(result, to_string(search->first).c_str());
                strcat(result, ": ");
                strcat(result, search->second.c_str());
                strcat(result, "\n");
            }

            // send result back to client
            if (sendto(voting_sockfd, result, strlen(result), 0, (struct sockaddr *)&client, addr_len) == -1)
            {
                cout << "as voting server sending: sendto() system call failed" << endl;
                exit(1);
            }
        }
        // secure voting
        else if (strcmp(init_choice, "2") == 0)
        {
            // send key for decryption
            strcpy(key, to_string(encrypt_key).c_str());
            if (sendto(voting_sockfd, key, strlen(key), 0, (struct sockaddr *)&client, addr_len) == -1)
            {
                cout << "as voting server sending: sendto() system call failed" << endl;
                exit(1);
            }

            // receive ID to vote for
            if (recvfrom(voting_sockfd, vote, MAX_MESSAGE, 0, (struct sockaddr *)&client, (socklen_t *)&addr_len) == -1)
            {
                cout << "as voting server receiving from client: recvfrom() system call failed" << endl;
                exit(1);
            }

            // decrypt ID
            ID = atoi(vote);
            ID = decrypt(ID, encrypt_key);

            cout << "client has voted for: " << ID << endl;

            bool open = is_voting_open(end_UTC);

            // vote for specificied candidate
            unordered_map<int, int>::iterator search = voting.find(ID);
            // voting is open
            if (open)
            {
                // client program has told us that it has already voted
                if (ID == -1)
                {
                    strcpy(result, "you have already voted\n");
                }
                // could not find ID
                else if (search == voting.end())
                {
                    strcpy(result, "candidate ID does not exist\n");
                }
                else
                {
                    search->second++;
                    strcpy(result, "vote has been successfully placed\n");
                }
            }
            // exceeded time
            else
            {
                strcpy(result, "voting has closed, time exceeded\n");
            }

            // send back confirmation of successful vote or error message
            if (sendto(voting_sockfd, result, strlen(result), 0, (struct sockaddr *)&client, addr_len) == -1)
            {
                cout << "as voting server sending: sendto() system call failed" << endl;
                exit(1);
            }
        }
        // voting results
        else if (strcmp(init_choice, "3") == 0)
        {
            // if voting is not open we can see
            bool can_see = !is_voting_open(end_UTC);
            if (can_see)
            {
                // voting results as results
                strcpy(result, "ID: number of votes\n");
                unordered_map<int, int>::iterator search = voting.begin();
                for (; search != voting.end(); search++)
                {
                    strcat(result, to_string(search->first).c_str());
                    strcat(result, ": ");
                    strcat(result, to_string(search->second).c_str());
                    strcat(result, "\n");
                }
            }
            else
            {
                strcpy(result, "voting is still on going, can not display voting summary\n");
            }

            // send result back to client
            if (sendto(voting_sockfd, result, strlen(result), 0, (struct sockaddr *)&client, addr_len) == -1)
            {
                cout << "as voting server sending translated word: sendto() system call failed" << endl;
                exit(1);
            }
        }
    }
}

int decrypt(int ID, int key)
{
    return ID / key;
}

bool is_voting_open(time_t &end_UTC)
{
    // check if voting is still open
    time_t current_time_local = time(0);

    // get local time and print it out
    struct tm *current_local = localtime(&current_time_local);
    char *current_time_string = asctime(current_local);
    cout << "local time: " << current_time_string;

    // get UTC time and print it out
    struct tm *current_gmt = gmtime(&current_time_local);
    current_time_string = asctime(current_gmt);
    cout << "UTC: " << current_time_string << endl;
    time_t current_UTC = mktime(current_gmt);

    // end must be greater
    // end - current >= 0 then we have not passed end yet, therefore voting is open
    double time_diff = difftime(end_UTC, current_UTC);

    if (time_diff >= 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}