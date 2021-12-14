//
// Created by Yuriy Mikhaildi on 12/3/21.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <pthread.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#define BUFSIZE 1024
#define LISTENQ  1024  /* second argument to listen() */
#define USR_ARR_LEN 10
#define PASS_ARR_LEN 10
#define SRVR_NAME_LEN 20
#define SRVR_PORT_LEN 20
#define USR_NAME_LEN 100
#define USR_PASS_LEN 100
#define USR_VAL_FAIL -1
/*
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/* User information */
struct UserConfig {
    char *usr[USR_ARR_LEN];
    char *pass[PASS_ARR_LEN];
} UserConfig;

/*Server information */
struct ServerConfig {
    char serverName[SRVR_NAME_LEN];
    char portNumber[SRVR_PORT_LEN];
} ServerConfig;

/* request handler */
int processRequest(int);

int main(int argc, char **argv) {
    int sockfd, connfd; /* socket */
    socklen_t clientlen; /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    char confFileName[] = "dfs.conf"; /* server conf */
    FILE *config;
    pid_t pid;

    if (argc != 3) {
        printf("usage: <serverName> <portNumber>\n");
        exit(1);
    }

    strcpy(ServerConfig.serverName, argv[1]);
    strcpy(ServerConfig.portNumber, argv[2]);

    if ((config = fopen(confFileName, "r")) == NULL)
        error("error opening the config file.");

    int count = 0;
    char configBuffer[100];

    while (!feof(config)) {
        fgets(configBuffer, 100, config);
        UserConfig.usr[count] = calloc(sizeof(configBuffer), sizeof(char));
        UserConfig.pass[count] = calloc(sizeof(configBuffer), sizeof(char));
        sscanf(configBuffer, "%s %s", UserConfig.usr[count], UserConfig.pass[count]);
        count++;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket.");

    /* setsockopt: Handy debugging trick that lets
     * us rerun the server immediately after we kill it;
     * otherwise we have to wait about 20 secs.
     * Eliminates "ERROR on binding: Address already in use" error.
     */
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *) &optval, sizeof(int)) < 0)
        return -1;

    /*
     * build the server's Internet address
     */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(atoi(ServerConfig.portNumber));

    if (bind(sockfd, (struct sockaddr *) &serveraddr,
             sizeof(serveraddr)) < 0) {
        error("ERROR on binding.");
    }

    if (listen(sockfd, LISTENQ) < 0)
        error("ERROR on listen for socket.");

    /* Resources used to come up with request loop.
     * https://stackoverflow.com/questions/33508997/waitpid-wnohang-wuntraced-how-do-i-use-these/34845669
     * https://canvas.colorado.edu/courses/76700/files/36338717?module_item_id=3158491
     */
    printf("Server <%s> ready for connection.\n", ServerConfig.serverName);
    while(1){
        if((connfd = accept(sockfd, (struct sockaddr* ) &clientaddr, &clientlen)) < 0){
            error("ERROR on accept from client sock.");
        }
        /* fork new process */
        pid = fork();
        if(pid == 0){
            /* close socket connection */
            if(close(sockfd) < 0)
                error("ERROR closing the client sock.");
            /* process the request on TCP connection */
            if(processRequest(connfd) < 0)
                error("ERROR processing the request.");
            /* close connection after request handle */
            if(close(connfd) <0)
                error("ERROR closing the client connection.");
            exit(0);
        }
        /* close connection if the fork failed */
        close(connfd);
        waitpid(-1, NULL, WNOHANG);
    }

}
int processRequest(int connfd){
    /* User Validation Before ACK commands */
    char command[BUFSIZE], fileName[BUFSIZE];
    int readBytes, WriteBytes;

    char buf[BUFSIZE];
    int size = 0, indicator = 0;
    char username[USR_NAME_LEN];
    char password[USR_PASS_LEN];
    /* read user info */
    if(read(connfd, (char *) &size, sizeof (size)) < 0){
        error("ERROR reading size message.");
    }
    size = ntohl(size);
    if(read(connfd, buf, size) < 0){
        error("ERROR reading user info.");
    }
    printf("Server %s received: <%s>.\n", ServerConfig.serverName, buf);
    /* parse the buff input */
    if(sscanf(buf, "%s %s", username, password) < 0){
        error("ERROR scanning buf for usr/pass.");
    }
    /* validate user */
    for(int index = 0; index < USR_ARR_LEN; index++){
        if(UserConfig.usr[index] == NULL){
            break;
        }if((strcmp(UserConfig.usr[index], username) == 0) &&
           (strcmp(UserConfig.pass[index], password) == 0)){
            indicator = 1;
            break;
        } else {
            continue;
        }
    }
    if(!indicator){
        printf("Server %s User: <%s> is not validated.\n", ServerConfig.serverName, username);
        return USR_VAL_FAIL;
    }
    bzero(buf, BUFSIZE);
    size = 0;
    if(read(connfd, (char *) &size, sizeof (size)) < 0){
        error("ERROR reading size message.");
    }
    size = ntohl(size);
    if(read(connfd, buf, size) < 0){
        error("ERROR reading user info.");
    }
    printf("Server %s received: <%s>.\n", ServerConfig.serverName, buf);

//    if (strcmp(command, "get") == 0) {
//        printf("Get: %s\n", fileName);
//    }
//    if (strcmp(command, "put") == 0) {
//        printf("Put: %s\n", fileName);
//    }
//    if (strcmp(command, "list") == 0) {
//        printf("List\n");
//
//    } else {
//        printf("Invalid command: %s\n", command);
//    }


    return 0;

}

#pragma clang diagnostic pop