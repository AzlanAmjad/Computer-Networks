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
#include <string.h>

using namespace std;

#define SERVER_PORT 8001
#define MAX_MESSAGE 2048

int main() {
    struct sockaddr_in server, client;
    int translator_sockfd;
    int addr_len = sizeof(struct sockaddr_in);

    char word_to_translate[MAX_MESSAGE];
    char translated_word[MAX_MESSAGE];

    unordered_map<string, string> dictionary;
    dictionary["Hello"] = "Bonjour";
    dictionary["School"] = "L'école";
    dictionary["Engineering"] = "Ingénierie";
    dictionary["Computer"] = "Ordinateur";
    dictionary["Write"] = "Écrivez";

    // server address initialization
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // UDP socket creation
    translator_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (translator_sockfd == -1) {
        cout << "as translator server: socket() system call failed" << endl;
        exit(1);
    }

    // bind IP address and port to end point (translator_sockfd)
    if (bind(translator_sockfd, (struct sockaddr *)&server, addr_len) == -1) {
        cout << "as translator server: bind() system call failed" << endl;
        exit(1);
    }
    cout << "UDP translator server is ready to receive information" << endl;

    // while loop for server client interaction
    while(1) {
        bzero(word_to_translate, MAX_MESSAGE);
        bzero(translated_word, MAX_MESSAGE);

        // receive word from client
        if (recvfrom(translator_sockfd, word_to_translate, MAX_MESSAGE, 0, (struct sockaddr *)&client, (socklen_t *)&addr_len) == -1) {
            cout << "as translator server receiving from client: recvfrom() system call failed" << endl;
            exit(1);
        }

        cout << "translator server received: " << word_to_translate << endl;
        string key(word_to_translate);
        
        unordered_map<string, string>::iterator search = dictionary.find(key);
        if (search == dictionary.end()) {
            strcpy(translated_word, "word not found in translation dictionary");
        }
        else {
            strcpy(translated_word, search->second.c_str());
        }

        cout << "translated word: " << translated_word << endl;

        // send translated word back to client
        if (sendto(translator_sockfd, translated_word, strlen(translated_word), 0, (struct sockaddr *)&client, addr_len) == -1) {
            cout << "as translator server sending translated word: sendto() system call failed" << endl;
            exit(1);
        }
    }

    close(translator_sockfd);
    return 0;
}
