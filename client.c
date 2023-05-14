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
#define LEN 20

/* host and port number */
char host[20] = "34.254.242.81";
int port = 8080;

/* flags for user status */
int is_authenticated;
int has_access;

/* buffer used for user commands */
char buffer[MAX_BUF];
/* buffer for jwt token */
char jwt[MAX_BUF];
/* buffer for session cookie */
char session_cookie[MAX_BUF];


/* ----------------------- helper functions ----------------------- */

void get_credentials_json(char username[], char password[]) {

}

/* ----------------------- helper commands functions ----------------------- */

void register_user(int sockfd) {
    printf("-----register function-----\n");
    char url[LEN] = "/api/v1/tema/auth/register";
    char content_type[LEN] = "application/json";
    char username[MAX_BUF];
    char password[MAX_BUF];

    /* get username */
    printf("username=\n");
    fgets(username, MAX_BUF, stdin);

    /* check if username has spaces */
    int nr = 0;
    char tmp[strlen(username) + 1];
    memcpy(tmp, username, strlen(username) + 1);

    char *token = strtok(tmp, " \n");
    while (token != NULL) {
        nr++;

        if (nr == 2) {
            printf("invalid username\n");
            return;
        }
        token = strtok(NULL, " \n");
    }
    
    /* get password */
    printf("password=\n");
    fgets(password, MAX_BUF, stdin);

    /* check if password has spaces */
    nr = 0;
    tmp[strlen(password) + 1];
    memcpy(tmp, password, strlen(password) + 1);

    token = strtok(tmp, " \n");
    while (token != NULL) {
        nr++;

        if (nr == 2) {
            printf("invalid password\n");
            return;
        }
        token = strtok(NULL, " \n");
    }

    /* delete endline */
    username[strlen(username) - 1] = '\0';
    password[strlen(password) - 1] = '\0';

    /* send post request */


    /* error if credetials are already used */
}

void login(int sockfd) {
    printf("-----login function-----\n");
    char url[LEN] = "/api/v1/tema/auth/login";
    char content_type[LEN] = "application/json";

    /* error if credentials don't match */
}

void enter_library(int sockfd) {
    printf("-----enter library function-----\n");
    char url[LEN] = "/api/v1/tema/library/access";

    /* check if user is authenticated */
    if (!is_authenticated) {
        printf("Error. Authenticate first!\n");
        return;
    }
}

void get_books(int sockfd) {
    printf("-----get books function-----\n");
    char url[LEN] = "/api/v1/tema/library/books";

    /* check if user has access to library */
    if (!has_access) {
        printf("Error. No access to library!\n");
        return;
    }
}

void get_book(int sockfd) {
    printf("-----get book function-----\n");
    char book_id[MAX_BUF];
    char url[LEN] = "/api/v1/tema/library/books/:%s", book_id;

    /* check if user has access to library */
    if (!has_access) {
        printf("Error. No access to library!\n");
        return;
    }

    /* error if book id is invalid */
}

void add_book(int sockfd) {
    printf("-----add book function-----\n");
    char url[LEN] = "/api/v1/tema/library/books";
    char content_type[LEN] = "application/json";

    /* check if user has access to library */
    if (!has_access) {
        printf("Error. No access to library!\n");
        return;
    }

    /* error if information is incomplete or not respecting format */
}

void delete_book(int sockfd) {
    printf("-----delete book function-----\n");
    char book_id[MAX_BUF];
    char url[LEN] = "/api/v1/tema/library/books/:%s", book_id;

    /* check if user has access to library */
    if (!has_access) {
        printf("Error. No access to library!\n");
        return;
    }

    /* error if id is invalid */
}

void logout(int sockfd) {
    printf("-----logout function-----\n");
    char url[LEN] = "/api/v1/tema/auth/logout";

    if (!is_authenticated) {
        printf("Not logged in\n");
        return;
    }

    /* reset status */
    is_authenticated = 0;
    has_access = 0;
    memset(session_cookie, 0, sizeof(session_cookie));
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
        } else if (strcmp(buffer, "add_book\n") == 0) {
            add_book(sockfd);
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
