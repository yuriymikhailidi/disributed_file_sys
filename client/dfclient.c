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
#include <arpa/inet.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#define BUFSIZE 1024
#define SERV_HOST_LEN 100
#define UNAME_LEN 100
#define PASS_LEN 100
#define DIR_LEN 100
#define COMMENT_LEN 10
#define SERVER_NUM 4


/* server values read from the config file */
typedef struct ServerVals {
    char *ip[SERVER_NUM];
    char *port[SERVER_NUM];
    char *serverDir[SERVER_NUM];
    char *username;
    char *password;
} ServerVals;

/*
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

ServerVals *readConfigVals(FILE *config);
int userValidation(int sockfd[], int sockArrSize,  ServerVals* serverVals);

int main(int argc, char **argv) {

    FILE *config;

    char username[BUFSIZE], password[BUFSIZE];
    char userInputBuf[256];

    char *filepath = NULL;
    /* check command line arguments */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <config-filepath>\n", argv[0]);
        exit(0);
    }
    filepath = argv[1];

    char inputBuf[BUFSIZE];
    char command[BUFSIZE], fileName[BUFSIZE];

    config = fopen(filepath, "rb");
    if (!config)
        error("Config file not opened\n");

    //setting up the server and user info ///
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    struct sockaddr_in serveraddr[4];
    int sockfd[4]; /* socket */
    int portno; /* port to listen on */

    ServerVals *serverVals;
    serverVals = readConfigVals(config);
    int serverIndex = 0;
    /*Create necessary connection for client-server*/
    while (serverIndex < SERVER_NUM) {
        bzero((char *) &serveraddr[serverIndex], sizeof(serveraddr[serverIndex]));
        portno = atoi(serverVals->port[serverIndex]);
        serveraddr[serverIndex].sin_family = AF_INET;
        serveraddr[serverIndex].sin_addr.s_addr = inet_addr(serverVals->ip[serverIndex]);
        serveraddr[serverIndex].sin_port = htons(portno);
        sockfd[serverIndex] = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd[serverIndex] < 0)
            error("ERROR opening socket.");
        if (setsockopt(sockfd[serverIndex], SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout,
                       sizeof(timeout)) < 0) {
            char* errMsg = calloc(100, sizeof (char ));
            snprintf(errMsg, 100, "Socket setting options failed for Server %d.", serverIndex);
            error(errMsg);
        }
        if (connect(sockfd[serverIndex], (struct sockaddr *) &serveraddr[serverIndex],
                    sizeof(serveraddr[serverIndex])) < 0) {
            error("Connecting to the server failed.");
        }
        serverIndex++;

    }

    printf("client set up complete\n");
    fclose(config);
    ///////////////////////////////////
    /* user authentication */
    if(userValidation(sockfd, SERVER_NUM, serverVals) < 0){
        error("ERROR user/password in conf is invalid.");
    }
    /////////////////////////////////`

    while (1) {
        bzero(inputBuf, BUFSIZE);
        bzero(command, BUFSIZE);
        bzero(fileName, BUFSIZE);
        printf("Enter get <filename>, put <filename>, "
               "list or exit:");
        fgets(inputBuf, BUFSIZE, stdin);
        sscanf(inputBuf, "%s %s", command, fileName);

        /* check if exit */
        if (strcmp(command, "exit") == 0) {
            printf("Exiting...\n");
            exit(0);
        }
        /* check for file */
        if (strlen(fileName) > 0) {

            if (strcmp(command, "get") == 0) {
                printf("get <%s>\n", fileName);
                sprintf(userInputBuf, "%s %s", command, fileName);
                int userLen = htonl(strlen(userInputBuf));
                serverIndex = 0;
                while(serverIndex < SERVER_NUM){
                    if(write(sockfd[serverIndex], (char*) &userLen, sizeof (userLen)) < 0){
                        error("ERROR failed write user buff len.");
                    }
                    if(write(sockfd[serverIndex], userInputBuf, strlen(userInputBuf)) < 0){
                        error("ERROR failed write user buff.");
                    }
                    serverIndex++;
                }
                continue;
            }
            if (strcmp(command, "put") == 0) {
                printf("put <%s>\n", fileName);
                continue;

            } else {
                /*handle wrong command */
                printf("Invalid command: %s\n", command);
                continue;
            }
        } else if (strcmp(command, "list") == 0) {
            /*list */
            printf("list\n");
            continue;
        } else {
            /* handle file missing */
            printf("usage. get <filename>, put <filename>, list, and exit.\n");
            continue;
        }
    }
}
int userValidation(int sockfd[], int sockArrSize,  ServerVals* serverVals){
    char userBuf[256];
    sprintf(userBuf, "%s %s", serverVals->username, serverVals->password);
    int userLen = htonl(strlen(userBuf));
    int serverIndex = 0;
    while(serverIndex < SERVER_NUM){
        if(write(sockfd[serverIndex], (char*) &userLen, sizeof (userLen)) < 0){
            error("ERROR failed write user buff len.");
        }
        if(write(sockfd[serverIndex], userBuf, strlen(userBuf)) < 0){
            error("ERROR failed write user buff.");
        }
        serverIndex++;
    }
    return 0;
}
ServerVals *readConfigVals(FILE *config) {
    ServerVals *serverVals = malloc(sizeof(ServerVals));
    char fileBuffer[BUFSIZE];
    /* values for config */
    char comment[COMMENT_LEN], serverHostInf[SERV_HOST_LEN], serverDir[DIR_LEN],
            username[UNAME_LEN], password[PASS_LEN];
    int index = 0;
    while (fgets(fileBuffer, sizeof(fileBuffer), config)) {
        if (strstr(fileBuffer, "Server")) {
            serverVals->ip[index] = calloc(BUFSIZE, sizeof(char));
            serverVals->port[index] = calloc(BUFSIZE, sizeof(char));
            serverVals->serverDir[index] = calloc(BUFSIZE, sizeof(char));
            sscanf(fileBuffer, "%s %s %s", comment, serverDir, serverHostInf);
            strcpy(serverVals->serverDir[index], serverDir);
            char *hostToken = strtok(serverHostInf, ":");
            strcpy(serverVals->ip[index], hostToken);
            hostToken = (strtok(NULL, ":"));
            strcpy(serverVals->port[index], hostToken);
            index++;
            bzero(comment, strlen(comment));
            bzero(serverDir, strlen(serverDir));
            bzero(serverHostInf, strlen(serverHostInf));
        } else if (strstr(fileBuffer, "Username")) {
            /*username */
            serverVals->username = calloc(UNAME_LEN, sizeof(char));
            sscanf(fileBuffer, "%s %s", comment, username);
            strcpy(serverVals->username, username);
            bzero(username, strlen(username));
        } else if (strstr(fileBuffer, "Password")) {
            /*password*/
            serverVals->password = calloc(PASS_LEN, sizeof(char));
            sscanf(fileBuffer, "%s %s", comment, password);
            strcpy(serverVals->password, password);
            bzero(password, strlen(password));
        }
    }
    return serverVals;
}

#pragma clang diagnostic pop