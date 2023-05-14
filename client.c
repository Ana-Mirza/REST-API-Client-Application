#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

#define MAX_BUF 100

char host[20] = "34.254.242.81";
int port = 8080;
char buffer[MAX_BUF];
int is_connected;
int has_access;

/* ----------------------- helper command functions ----------------------- */

void register_user(int sockfd) {
    printf("-----register function-----\n");
}

void login(int sockfd) {
    printf("-----login function-----\n");
}

void enter_library(int sockfd) {
    printf("-----enter library function-----\n");
}

void get_books(int sockfd) {
    printf("-----get books function-----\n");
}

void get_book(int sockfd) {
    printf("-----get book function-----\n");
}

void delete_book(int sockfd) {
    printf("-----delete book function-----\n");
}

void logout(int sockfd) {
    printf("-----logout function-----\n");
}

/* ----------------------- MAIN -----------------------*/

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    /* open connection with server */
    sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("error sockfd");
        return -1;
    }

    /* read commands from user */
    while(1) {
        memset(buffer, 0, sizeof(buffer));

        /* get command */
        printf("insert command\n");
        fgets(buffer, MAX_BUF, stdin);
        printf("%s", buffer);

        if (strcmp(buffer, "register\n") == 0) {
            register_user(sockfd);
        } else if (strcmp(buffer, "login\n") == 0) {
            login(sockfd);
        } else if (strcmp(buffer, "enter_library\n") == 0) {
            enter_library(sockfd);
        } else if (strcmp(buffer, "get_books\n") == 0) {
            get_books(sockfd);
        } else if (strcmp(buffer, "get_book\n") == 0) {
            get_book(sockfd);
        } else if (strcmp(buffer, "delete_book\n") == 0) {
            delete_book(sockfd);
        } else if (strcmp(buffer, "logout\n") == 0) {
            logout(sockfd);
        } else if (strcmp(buffer, "exit\n") == 0) {
            break;
        } else {
            /* invalid command */
            printf("Invalid command\n");
        }
    }

    /* close connection */
    close_connection(sockfd);

    return 0;
}
