//
// Created by Yuriy Mikhaildi on 12/3/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#define BUFSIZE 1024

typedef struct Server {
    int id;
    int sockfd; /* socket */
    int portno; /* port to listen on */
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
} Server;

/*
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

Server *provide_server_info(int id, char *host, int port);

int main(int argc, char **argv) {

    FILE *config;

    char valueOne[BUFSIZE], valueTwo[BUFSIZE];
    char username[BUFSIZE], password[BUFSIZE];

    char *filepath = NULL;
    /* check command line arguments */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <config-filepath>\n", argv[0]);
        exit(0);
    }


    filepath = argv[1];
    char fileBuffer[BUFSIZE];
    char inputBuf[BUFSIZE];
    char command[BUFSIZE], fileName[BUFSIZE];

    Server *One = malloc(sizeof(Server));
    Server *Two = malloc(sizeof(Server));
    Server *Three = malloc(sizeof(Server));
    Server *Four = malloc(sizeof(Server));

    config = fopen(filepath, "rb");

    if (!config)
        error("Config file not opened\n");
    //setting up the server and user info ///
    int i = 0;
    while (fgets(fileBuffer, sizeof(fileBuffer), config)) {
        sscanf(fileBuffer, "%s %s\n", valueOne, valueTwo);
        i++;
        if (i == 1) {
            One->id = i;
            One->portno = atoi(valueTwo);
            One->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (One->sockfd < 0)
                error("ERROR opening socket");
            One->server = gethostbyname(valueOne);
            bzero((char *) &One->serveraddr, sizeof(One->serveraddr));
            One->serveraddr.sin_family = AF_INET;
            bcopy((char *) One->server->h_addr,
                  (char *) &One->serveraddr.sin_addr.s_addr, One->server->h_length);
            One->serveraddr.sin_port = htons(One->portno);
        }
        if (i == 2) {
            Two->id = i;
            Two->portno = atoi(valueTwo);
            Two->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (Two->sockfd < 0)
                error("ERROR opening socket");
            Two->server = gethostbyname(valueOne);
            bzero((char *) &Two->serveraddr, sizeof(Two->serveraddr));
            Two->serveraddr.sin_family = AF_INET;
            bcopy((char *) Two->server->h_addr,
                  (char *) &Two->serveraddr.sin_addr.s_addr, Two->server->h_length);
            Two->serveraddr.sin_port = htons(Two->portno);
        }
        if (i == 3) {
            Three->id = i;
            Three->portno = atoi(valueTwo);
            Three->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (Three->sockfd < 0)
                error("ERROR opening socket");
            Three->server = gethostbyname(valueOne);
            bzero((char *) &Three->serveraddr, sizeof(Three->serveraddr));
            Three->serveraddr.sin_family = AF_INET;
            bcopy((char *) Three->server->h_addr,
                  (char *) &Three->serveraddr.sin_addr.s_addr, Three->server->h_length);
            Three->serveraddr.sin_port = htons(Three->portno);
        }
        if (i == 4) {
            Four->id = i;
            Four->portno = atoi(valueTwo);
            Four->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (Four->sockfd < 0)
                error("ERROR opening socket");
            Four->server = gethostbyname(valueOne);
            bzero((char *) &Three->serveraddr, sizeof(Three->serveraddr));
            Four->serveraddr.sin_family = AF_INET;
            bcopy((char *) Four->server->h_addr,
                  (char *) &Four->serveraddr.sin_addr.s_addr, Four->server->h_length);
            Four->serveraddr.sin_port = htons(Four->portno);
        }
        if (i == 5) {
            strcpy(username, valueTwo);
        }
        if (i == 6) {
            strcpy(password, valueTwo);
        }
    }
    printf("client set up complete\n");
    ///////////////////////////////////
    while (1) {
        bzero(inputBuf, BUFSIZE);
        bzero(command, BUFSIZE);
        bzero(fileName, BUFSIZE);
        printf("Enter get <filename>, put <filename>, "
               "list or exit:");
        fgets(inputBuf, BUFSIZE, stdin);
        sscanf(inputBuf, "%s %s", command, fileName);
        if (strcmp(command, "get") == 0) {
            printf("Get: %s\n", fileName );
            continue;
        }
        if (strcmp(command, "put") == 0) {
            printf("Put: %s\n", fileName );
            continue;

        }
        if (strcmp(command, "list") == 0) {
            printf("List\n");
            continue;

        }
        if (strcmp(command, "exit") == 0) {
            printf("Exiting...\n");
            exit(0);
        }
        else{
            printf("Invalid command: %s\n", command);
            continue;
        }
        fclose(config);
    }

}

#pragma clang diagnostic pop