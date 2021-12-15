//
// Created by Yuriy Mikhaildi on 12/3/21.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
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
#define SRVR_PORT_LEN 25
#define USR_NAME_LEN 25
#define USR_PASS_LEN 25
#define USR_VAL_FAIL -1
#define CMND_LIST_LEN 4
#define CMND_GET_LEN 3
#define CMND_PUT_LEN 3

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

static void *create_thread(void *vargp) {
    int connfd = *((int *) vargp);
    pthread_detach(pthread_self());
    free(vargp);
    processRequest(connfd);
    close(connfd);
    return NULL;
}
int openListenfd(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;

    /* listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
} /* end open_listenfd */

int main(int argc, char **argv) {
    int sockfd, connfd, *connfdp; /* socket */
    socklen_t clientlen; /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    char confFileName[] = "./server/dfs.conf"; /* server conf */
    FILE *config;
    pid_t pid;
    pthread_t tid;


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

    int listenfd = openListenfd(atoi(ServerConfig.portNumber));
    while (1) {
        connfdp = (int *) (malloc(sizeof(int)));
        *connfdp = accept(listenfd, (struct sockaddr *) &clientaddr, (socklen_t *) &clientlen);
        pthread_create(&tid, NULL, create_thread, connfdp);
    }
}
int processRequest(int connfd) {
    /* User Validation Before ACK commands */
    printf("Server %s waiting for input.\n", ServerConfig.serverName);

    int readBytes, writeBytes;
    int size = 0, indicator = 0;

    char buf[BUFSIZE], bufCopy[BUFSIZE];
    char username[USR_NAME_LEN];
    char password[USR_PASS_LEN];
    char command[BUFSIZE];
    char fileName[BUFSIZE];
    char CmndListException[CMND_LIST_LEN];

    memset(buf, 0, strlen(buf));
    memset(command, 0, strlen(command));
    memset(fileName, 0, strlen(fileName));

    /* read user info */
    readBytes = read(connfd, (char *) &size, sizeof(size));
    if (readBytes < 0) {
        error("ERROR reading size message.");
    }
    size = ntohl(size);
    readBytes = read(connfd, buf, size);
    if (readBytes < 0) {
        error("ERROR reading user info.");
    }

    memcpy(bufCopy, buf, size);
    printf("Server %s received: %s. Len: %d\n",
           ServerConfig.serverName, bufCopy, size);
    strncpy(CmndListException, bufCopy, CMND_LIST_LEN);
    if (strcmp(CmndListException, "list") == 0) {
        sscanf(bufCopy,"%s %s %s", command, username, password);
    } else {
        sscanf(bufCopy,"%s %s %s %s", command, fileName, username, password);
    }
    /* validate user */
    for (int index = 0; index < USR_ARR_LEN; index++) {
        if (UserConfig.usr[index] == NULL) {
            break;
        }
        if ((strcmp(UserConfig.usr[index], username) == 0) &&
            (strcmp(UserConfig.pass[index], password) == 0)) {
            indicator = 1;
            break;
        } else {
            continue;
        }
    }
    if (!indicator) {
        printf("Server %s User: <%s> is not validated.\n", ServerConfig.serverName, username);
        return USR_VAL_FAIL;
    }
    /* create dir for the user
     * Resource:
     * https://stackoverflow.com/questions/10147990/how-to-create-directory-with-right-permissions-using-c-on-posix
     * https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
     * */
    struct stat dirStats = {0};
    char dirServer[30];
    char dirUser[30];

    sprintf(dirServer, "./server/%s", ServerConfig.serverName);
    int resServ = stat(dirServer, &dirStats);
    if(resServ < 0) {
        mkdir(dirServer, 0777);
    }
    sprintf(dirUser, "./server/%s/%s", ServerConfig.serverName, username);
    int resUsr = stat(dirUser, &dirStats);
    if(resUsr < 0) {
        mkdir(dirUser, 0777);
        printf("Server %s created user dir: %s\n", ServerConfig.serverName, dirUser);
    }
    /* ----------------------- */
    if (strcmp(command, "get") == 0) {
        /* ACK the input */
        return 0;
    }
    if (strcmp(command, "put") == 0) {
        /* ACK the input */
        return 0;
    }
    if (strcmp(command, "list") == 0) {
        /* ACK the input */
        return 0;
    } else {
        return 0;
    }
    return 0;
}

#pragma clang diagnostic pop