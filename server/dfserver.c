//
// Created by Yuriy Mikhaildi on 12/3/21.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>
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
#define MAX_FILES 30
#define USR_ARR_LEN 10
#define PASS_ARR_LEN 10
#define SRVR_NAME_LEN 20
#define SRVR_PORT_LEN 25
#define USR_NAME_LEN 25
#define USR_PASS_LEN 25
#define USR_VAL_FAIL -1
#define CMND_LIST_LEN 4
#define PART_NUM 2

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

void getFilePart(const char *dirUser);

int grabingAllFiles(const char *filePart, char **files);

int getFileCount(const char *fileName, char *const *files, int fileCount);

int passFileToClient(int connfd, int writeBytes, char *messageBuf, const char *fileName, const char *dirUser,
                      char *const *files, int *fileCount);

int userValidation(const char *username, const char *password);

static void *create_thread(void *vargp) {
    int connfd = *((int *) vargp);
    pthread_detach(pthread_self());
    free(vargp);
    processRequest(connfd);
    close(connfd);
    return NULL;
}

int openListenfd(int port) {
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *) &optval, sizeof(int)) < 0)
        return -1;

    /* listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short) port);
    if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
} /* end open_listenfd */

int main(int argc, char **argv) {
    int *connfdp; /* socket */
    socklen_t clientlen; /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    char confFileName[] = "./server/dfs.conf"; /* server conf */
    FILE *config;
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
        *connfdp = accept(listenfd, (struct sockaddr *) &clientaddr, (socklen_t * ) & clientlen);
        pthread_create(&tid, NULL, create_thread, connfdp);
    }
}

int processRequest(int connfd) {
    /* User Validation Before ACK commands */
    printf("Server %s waiting for input.\n", ServerConfig.serverName);

    int readBytes, writeBytes, partOfFile;
    int size = 0, indicator = 0;

    /* local arr */
    char messageBuf[BUFSIZE], messageBufCopy[BUFSIZE];
    char username[USR_NAME_LEN];
    char password[USR_PASS_LEN];
    char command[BUFSIZE];
    char fileName[BUFSIZE];
    char CmndListException[CMND_LIST_LEN];

    memset(messageBuf, 0, strlen(messageBuf));
    memset(command, 0, strlen(command));
    memset(fileName, 0, strlen(fileName));

    /* read user info buffer*/
    readBytes = read(connfd, (char *) &size, sizeof(size));
    if (readBytes < 0) {
        error("ERROR reading size message.");
    }
    size = ntohl(size);
    readBytes = read(connfd, messageBuf, size);
    if (readBytes < 0) {
        error("ERROR reading user info.");
    }

    memcpy(messageBufCopy, messageBuf, size);
    printf("Server %s received: %s. Len: %d\n",
           ServerConfig.serverName, messageBufCopy, size);
    strncpy(CmndListException, messageBufCopy, CMND_LIST_LEN);
    if (strcmp(CmndListException, "list") == 0) {
        /* if list there is no file */
        sscanf(messageBufCopy, "%s %s %s", command, username, password);
    } else {
        /* else command with a file */
        sscanf(messageBufCopy, "%s %s %s %s", command, fileName, username, password);
    }
    /* validate user */
    indicator = userValidation(username, password);
    if (!indicator) {
        printf("Server %s User: <%s> is not validated.\n", ServerConfig.serverName, username);
        return USR_VAL_FAIL;
    }
    /* create dir for the user
     * Resource:
     * https://stackoverflow.com/questions/10147990/how-to-create-directory-with-right-permissions-using-c-on-posix
     * https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
     * https://c-for-dummies.com/blog/?p=3246
     * */
    struct stat dirStats = {0};
    char dirServer[30];
    char dirUser[30];

    sprintf(dirServer, "./server/%s", ServerConfig.serverName);
    int resServ = stat(dirServer, &dirStats);
    if (resServ < 0) {
        mkdir(dirServer, 0777);
    }
    sprintf(dirUser, "./server/%s/%s", ServerConfig.serverName, username);
    int resUsr = stat(dirUser, &dirStats);
    if (resUsr < 0) {
        mkdir(dirUser, 0777);
        printf("Server %s created user dir: %s\n", ServerConfig.serverName, dirUser);
    }
    /* ----------------------- */
    if (strcmp(command, "get") == 0) {
        /* ACK the input */
        char filePart[100];
        /* get the file parts from user dir
         * Resources:
         * https://c-for-dummies.com/blog/?p=3246
         */
        getFilePart(dirUser);

        int filePartLen = htonl(strlen(filePart));
        writeBytes = write(connfd, (char *) &filePartLen, sizeof(filePartLen));
        if (writeBytes < 0) {
            error("ERROR in writing to client part len.");
        }
        writeBytes = write(connfd, filePart, strlen(filePart));
        if (writeBytes < 0) {
            error("ERROR in writing to client part of file.");
        }
        char *files[50];
        /* write file parts into array for later user */
        if (grabingAllFiles(filePart, files) < 0)
            error("ERROR in finding all files.");

        int fileCount = 0;
        fileCount = getFileCount(fileName, files, fileCount);

        if (passFileToClient(connfd, writeBytes, messageBuf, fileName, dirUser, files, &fileCount) < 0)
            error("ERROR in sending file to client.");
    }
    if (strcmp(command, "put") == 0) {
        /* ACK the input */
        bzero(messageBuf, BUFSIZE);
        int index = 0;
        while (index < PART_NUM) {
            size = 0;
            /* read user info */
            readBytes = read(connfd, (char *) &partOfFile, sizeof(partOfFile));
            if (readBytes < 0) {
                error("ERROR reading size message.");
            }
            /* read user info */
            readBytes = read(connfd, (char *) &size, sizeof(size));
            if (readBytes < 0) {
                error("ERROR reading size message.");
            }
            size = ntohl(size);
            readBytes = read(connfd, messageBuf, size);
            if (readBytes < 0) {
                error("ERROR reading user info.");
            }
            char filePath[100];
            sprintf(filePath, "%s/%s.%d", dirUser, fileName, partOfFile + 1);
            FILE *currentFile = fopen(filePath, "wb");
            if (currentFile == NULL)
                error("ERROR in opening the file for put.");
            fprintf(currentFile, "%s", messageBuf);
            fclose(currentFile);
            bzero(messageBuf, strlen(messageBuf));
            index++;
        }
        printf("Server %s received all parts of file <%s>", ServerConfig.serverName, fileName);
    }
    if (strcmp(command, "list") == 0) {
        /* ACK the input */
        printf("list is not implemented at the moment\n");
        return 0;
    } else {
        return 0;
    }
}

int userValidation(const char *username, const char *password) {
    int indicator;
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
    return indicator;
}

int passFileToClient(int connfd, int writeBytes, char *messageBuf, const char *fileName, const char *dirUser,
                     char *const *files, int *fileCount) {
    int filePartSendIndicator[4];
    writeBytes = write(connfd, fileCount, sizeof((*fileCount)));
    if (writeBytes < 0) {
        error("ERROR in writing to client file count.");
    }

    for (int fileIndex = 0; fileIndex < MAX_FILES; fileIndex++) {
        if (files[fileIndex] != NULL && strstr(files[fileIndex], fileName) != NULL) {
            /* setting up the dir for reading files */
            char partFileName = (files[fileIndex][strlen(files[fileIndex]) - 1]);
            int partFileNumber = partFileName - '0';
            char whereToFind[50];
            sprintf(whereToFind, "%s/%s", dirUser, files[fileIndex]);
            printf("Server %s looking in %s", ServerConfig.serverName, dirUser);
            /* open file to send */
            FILE *fileToSend = fopen(whereToFind, "r+b");
            if (fileToSend == NULL)
                error("ERROR opening the file to send.");
            /* get the file size */
            fseek(fileToSend, 0l, SEEK_END);
            long file_size = ftell(fileToSend);
            rewind(fileToSend);

            int fileChar;
            int writtenBytes = 0;
            bzero(messageBuf, strlen(messageBuf));
            if (fileToSend != NULL) {
                /* read the file into buffer */
                while ((fileChar = fgetc(fileToSend)) != EOF) {
                    messageBuf[writtenBytes++] = (char) fileChar;
                }
            }
            /* check if the part already been sent */
            if (filePartSendIndicator[partFileNumber - 1] != 1) {

                writeBytes = write(connfd, &partFileName, sizeof(char));
                if (writeBytes < 0) {
                    error("ERROR in writing to client file name.");
                }
                writeBytes = write(connfd, (char *) &file_size, sizeof(file_size));
                if (writeBytes < 0) {
                    error("ERROR in writing to client file size.");
                }

                writeBytes = write(connfd, messageBuf, file_size);
                if (writeBytes < 0) {
                    error("ERROR in writing to client file.");
                }
                /* buffer has left server */
                filePartSendIndicator[partFileNumber - 1] = 1;
            }
            fclose(fileToSend);
        }
    }
    return 0;
}

int getFileCount(const char *fileName, char *const *files, int fileCount) {
    /* Count parts of file */
    int partOne, partTwo, partThree, partFour = 0;
    for (int fileIndex = 0; fileIndex < MAX_FILES; fileIndex++) {
        if ((files[fileIndex] != NULL) && (strstr(files[fileIndex], fileName) != NULL)) {
            char fileNameFromArr = (files[fileIndex][strlen(files[fileIndex]) - 1]);
            int partNumber = fileNameFromArr - '0';
            printf("Server %s is checking part: %d", ServerConfig.serverName, partNumber);
            if (partNumber == 1 && partOne == 0) {
                partOne = 1;
                fileCount++;
            }
            if (partNumber == 2 && partTwo == 0) {
                partTwo = 1;
                fileCount++;
            }
            if (partNumber == 3 && partThree == 0) {
                partThree = 1;
                fileCount++;
            }
            if (partNumber == 4 && partFour == 0) {
                partFour = 1;
                fileCount++;
            }
        }
    }
    return fileCount;
}

int grabingAllFiles(const char *filePart, char **files) {
    /* Rsources
     * https://www.geeksforgeeks.org/isdigit-function-in-c-c-with-examples/#:~:text=The%20isdigit(c)%20is%20a,9'%20and%20zero%20for%20others.
     */
    char *fileToken = strtok(filePart, "\n");
    int fileIndex = 0;
    while (fileToken != NULL) {
        size_t tokenLen = strlen(fileToken);
        int counter = 0;
        /* read file name till digit */
        if (isdigit(fileToken[strlen(fileToken) - 1]) != 0) {
            files[fileIndex] = calloc(strlen(fileToken), sizeof(char));
            strncpy(files[fileIndex], fileToken, strlen(fileToken));
        }
        fileToken = strtok(NULL, "\n");
        fileIndex++;
    }
    return 0;
}

void getFilePart(const char *dirUser) {
    char filePart[100];
    DIR *readDir;
    struct dirent *serverDir;
    if ((readDir = opendir(dirUser)) == NULL){
        error("ERROR opening dir for user.");
    }
    if (readDir) {
        printf("Server %s reading dir %s\n", ServerConfig.serverName, dirUser);
        while ((serverDir = readdir(readDir)) != NULL) {
            strcat(filePart, serverDir->d_name);
            strcat(filePart, "\n");
        }
        printf("Server %s is grabbing %s", ServerConfig.serverName, filePart);
        closedir(readDir);
    }
}

#pragma clang diagnostic pop