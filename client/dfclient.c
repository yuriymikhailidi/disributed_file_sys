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
#include <openssl/md5.h>

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


int allocatePartSize(int partSize, int fileSizeIndicator, char **serverPart);

unsigned long getHashRowVal(int serverIndex, int *cMask, int partCalcSize, char **serverPart, size_t partSize,
                            const unsigned char *outBuf, MD5_CTX *md5Ctx, unsigned char *hash, const FILE *fileToPut);

int main(int argc, char **argv) {

    FILE *config;

    char userInputBuf[256];

    char *filepath = NULL;
    /* check command line arguments */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <config-filepath>\n", argv[0]);
        exit(0);
    }
    filepath = argv[1];


    config = fopen(filepath, "rb");
    if (!config)
        error("ERROR Config file not opened.\n");

    //setting up the server and user info ///
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    struct sockaddr_in serveraddr[4];
    int sockfd[4]; /* socket */
    int portno; /* port to listen on */
    ServerVals *serverVals;
    serverVals = readConfigVals(config);
    /*Create necessary connection for client-server*/
    for (int serverIndex = 0; serverIndex < SERVER_NUM; serverIndex++) {
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
            char *errMsg = calloc(100, sizeof(char));
            snprintf(errMsg, 100, "ERROR Socket setting options failed for Server %d.", serverIndex);
            error(errMsg);
        }
        if (connect(sockfd[serverIndex], (struct sockaddr *) &serveraddr[serverIndex],
                    sizeof(serveraddr[serverIndex])) < 0) {
            error("ERROR Connecting to the server failed.");
        }
    }

    printf("client set up complete\n");
    fclose(config);
    int readBytes = 0, writeBytes = 0;
    char inputBuf[BUFSIZE];
    char command[BUFSIZE], fileName[BUFSIZE];
    while (1) {
        memset(inputBuf, 0, strlen(inputBuf));
        memset(command, 0, strlen(command));
        memset(fileName, 0, strlen(fileName));
        printf("Enter get <filename>, put <filename>, list: ");
        fgets(inputBuf, BUFSIZE, stdin);
        sscanf(inputBuf, "%s %s", command, fileName);
        strtok(inputBuf, "\n");
        sprintf(userInputBuf, "%s %s %s", inputBuf, serverVals->username, serverVals->password);
        int userLen = htonl(strlen(userInputBuf));
        printf("Client sent: %s\n", userInputBuf);
        for (int serverIndex = 0; serverIndex < SERVER_NUM; serverIndex++) {
            printf("Writing to Server %s\n", serverVals->serverDir[serverIndex]);
            writeBytes = write(sockfd[serverIndex], (char *) &userLen, sizeof(userLen));
            if (writeBytes < 0) {
                error("ERROR failed write user buff len.");
            }
            writeBytes = write(sockfd[serverIndex], userInputBuf, strlen(userInputBuf));
            if (writeBytes < 0) {
                error("ERROR failed write user buff.");
            }
            printf("Client wrote %d bytes\n", writeBytes);
        }
        /* check for file */
        if (strcmp(command, "get") == 0) {
            printf("get <%s>\n", fileName);

            char partBuffer[BUFSIZE];
            char allParts[SERVER_NUM][BUFSIZE];

            long partLen;
            int partCount[SERVER_NUM];

            for (int partIndex = 0; partIndex < SERVER_NUM; partIndex++) {
                readBytes = read(sockfd[partIndex], &partCount[partIndex], sizeof(partCount[partIndex]));
                if (readBytes < 0) {
                    error("ERROR reading size message.");
                }
            }

            for (int serverIndex = 0; serverIndex < SERVER_NUM; serverIndex++) {
                for (int partIndex = 0; partIndex < partCount[serverIndex]; partIndex++) {
                    char messageChar;

                    readBytes = read(sockfd[serverIndex], &messageChar, sizeof(char));
                    if (readBytes < 0)
                        error("ERROR in getting message char.");

                    readBytes = read(sockfd[serverIndex], (char *) partLen, sizeof(partLen));
                    if (readBytes < 0)
                        error("ERROR in getting part len.\n");

                    readBytes = read(sockfd[serverIndex], partBuffer, partLen);
                    if (readBytes < 0)
                        error("ERROR in getting the file.\n");

                    int serverPartIndicator = messageChar - '0';
                    if (serverPartIndicator == 1)
                        strcpy(allParts[0], partBuffer);
                    if (serverPartIndicator == 2)
                        strcpy(allParts[1], partBuffer);
                    if (serverPartIndicator == 3)
                        strcpy(allParts[2], partBuffer);
                    if (serverPartIndicator == 4)
                        strcpy(allParts[3], partBuffer);
                    bzero(partBuffer, sizeof(partBuffer));
                    partLen = 0;
                }
            }

            /* save file */
            FILE* fileToSave = fopen(fileName, "wb");
            if (fileToSave == NULL) {
                error("ERROR in saving the file to client.");
            }
            for (int partIndex = 0; partIndex < SERVER_NUM; partIndex++) {
                fprintf(fileToSave, "%c", partBuffer[partIndex]);
            }
            continue;

        } else if (strcmp(command, "put") == 0) {
            printf("put <%s>\n", fileName);
            int filePartTable[4][4][2] =
                 { {{0, 1},{1, 2},{2, 3},{3, 0}},
                 { {3, 0},{0, 1},{1, 2},{2, 3} },
                 { {2, 3},{3, 0},{0, 1},{1, 2} },
                 { {1, 2},{2, 3},{3, 0},{0, 1} } };
            int fileSizeIndicator = 0, serverIndex = 0,
                    cMask = 0, partCalcSize = 0;
            long fileSize;
            char *serverPart[SERVER_NUM];
            size_t partSize = 0;
            unsigned char outBuf[MD5_DIGEST_LENGTH];
            MD5_CTX md5Ctx;
            unsigned char *hash;
            unsigned long hashRow = 0;

            FILE *fileToPut = fopen(fileName, "r+b");
            if (fileToPut == NULL) {
                error("ERROR put couldn't open file.");
            }

            /* find file size
             * Resource:
             * https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
             * */
            fseek(fileToPut, 0L, SEEK_END);
            fileSize = ftell(fileToPut);
            rewind(fileToPut);

            /* find part size,
             * if even or odd */
            partCalcSize = fileSize / 4;
            int res = fileSize % 2;
            if (res != 0) {
                fileSizeIndicator = 1;
            }

            /* allocate memory for each part */
            if (allocatePartSize(partCalcSize, fileSizeIndicator, serverPart) < 0) {
                error("ERROR allocating mem for parts");
            }
            hashRow = getHashRowVal(serverIndex,
                                    &cMask, partCalcSize,
                                    serverPart, partSize,
                                    outBuf, &md5Ctx, hash, fileToPut);
            char writeChar;
            int theExtraByte = 0, messageLen, partToBeSent;
            for (int key = 0; key < SERVER_NUM; key++) {
                for (int val = 0; val < 2; val++) {
                    if (filePartTable[hashRow][key][val] == 3 && fileSizeIndicator) {
                        theExtraByte = 1;
                    }
                    messageLen = strlen(serverPart[filePartTable[hashRow][key][val]]);
                    messageLen = htonl(messageLen);
                    partToBeSent = (filePartTable[hashRow][key][val]);
                    if (write(sockfd[key], &partToBeSent, sizeof(partToBeSent)) < 0) {
                        error("ERROR sending part number");
                    }
                    if (write(sockfd[key], &messageLen, sizeof(messageLen)) < 0) {
                        error("ERROR sending len part of message");
                    }
                    if (write(sockfd[key], serverPart[filePartTable[hashRow][key][val]],
                              strlen(serverPart[filePartTable[hashRow][key][val]])) < 0) {
                        error("ERROR sending the part file.");
                    }
                }
            }
            continue;
        } else if (strcmp(command, "list") == 0) {
            printf("list is not implemented at the moment\n");
            continue;
        } else {
            /*handle wrong command */
            printf("Invalid command provided %s\n", command);
            continue;
        }
    }
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

/* preform MD5 hashing
   * Resources:
   * https://www.openssl.org/docs/man1.1.1/man3/MD5.html
   * https://stackoverflow.com/questions/7627723/how-to-create-a-md5-hash-of-a-string-in-c
   * https://stackoverflow.com/questions/10324611/how-to-calculate-the-md5-hash-of-a-large-file-in-c
   * */
unsigned long getHashRowVal(int serverIndex, int *cMask, int partCalcSize, char **serverPart, size_t partSize,
                            const unsigned char *outBuf, MD5_CTX *md5Ctx, unsigned char *hash, const FILE *fileToPut) {
    unsigned long hashRow;
    MD5_Init(md5Ctx);
    while (((*cMask) = fgetc(fileToPut)) != EOF) {
        serverPart[serverIndex][partSize++] = (char) (*cMask);
        MD5_Update(md5Ctx, cMask, 1);
        if (serverIndex < SERVER_NUM - 1) {
            if (partSize >= partCalcSize) {
                serverIndex++;
                partSize = 0;
            }
        }
    }
    MD5_Final(outBuf, md5Ctx);
    char temp[SERVER_NUM];
    hash = calloc(sizeof(MD5_DIGEST_LENGTH), sizeof(char));
    for (int index = 0; index < MD5_DIGEST_LENGTH; index++) {
        sprintf(temp, "%02x", (unsigned int) outBuf[index]);
        strcat(hash, temp);
    }
    char first_five[5];
    strncpy(first_five, hash, 5);
    /*Find the mod result */
    hashRow = (long) strtol(first_five, NULL, 16) % 4;
    return hashRow;
}

int allocatePartSize(int partCalcSize, int fileSizeIndicator, char **serverPart) {
    for (int serverIndex = 0; serverIndex < SERVER_NUM; serverIndex++) {
        if (fileSizeIndicator && serverIndex == 3) {
            serverPart[serverIndex] = calloc(sizeof(partCalcSize) + 1, sizeof(char));
        } else {
            serverPart[serverIndex] = calloc(sizeof(partCalcSize), sizeof(char));
        }
    }
    return 0;
}

#pragma clang diagnostic pop