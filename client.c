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

#include "parson.h"

#define MAX_BUF 200
#define LEN 50

/* host and port number */
char host[20] = "34.254.242.81";
int port = 8080;

/* buffers for requests */
char *request;
char *response;

/* flags for user status */
int is_authenticated;
int has_access;

/* buffer used for user commands */
char buffer[MAX_BUF];
/* buffer for jwt token */
char *jwt_token;
/* buffer for session cookie */
char *session_cookie[1];
/* buffer for user credentials */
char *credentials_json[1];


/* ----------------------- helper functions ----------------------- */

void get_credentials_json(char username[], char password[]) {
    JSON_Value *val = json_value_init_object();
    if (val == NULL) {
        printf("json value failed");
        exit(-1);
    }

    JSON_Object *obj = json_value_get_object(val);
    if (obj == NULL) {
        printf("json object failed");
        exit(-1);
    }

    json_object_set_string(obj, "username", username);
    json_object_set_string(obj, "password", password);
    credentials_json[0] = json_serialize_to_string(val);
    json_value_free(val);
}

void send_post_request(int sockfd, char url[], char token[MAX_BUF]) {
    char content_type[LEN] = "application/json";

    request = compute_post_request("34.254.242.81:8080", url, content_type, credentials_json, 1, NULL, 0, token);
    printf("%s\n", request);

    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);
    printf("%s\n", response);

    /* free memory */
    free(request);
}

void send_get_delete_request(int sockfd, char url[], char token[], int delete_request) {
    request = compute_get_request("34.254.242.81:8080", url, NULL, session_cookie, 1, token, delete_request);
    printf("%s\n", request);

    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);
    printf("%s\n", response);

    /* free memory */
    free(request);
}

/* ----------------------- helper commands functions ----------------------- */

void register_user(int sockfd) {
    printf("-----register function-----\n");
    char url[LEN] = "/api/v1/tema/auth/register";
    char username[MAX_BUF];
    char password[MAX_BUF];

    /* check if user is already logged */
    if (is_authenticated) {
        printf("Error. Already logged in!\n");
        return;
    }

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

    /* convert credentials to json */
    get_credentials_json(username, password);

    /* send post request */
    send_post_request(sockfd, url, NULL);
    free(credentials_json[0]);

    /* error if credentials are already used */
    if (strstr(response, "is taken") != NULL) {
        printf("Invalid credentials.\n");
    } else {
        printf("Registration successful!\n");
    }

    free(response);
}

void login(int sockfd) {
    printf("-----login function-----\n");
    char url[LEN] = "/api/v1/tema/auth/login";
    char content_type[LEN] = "application/json";

    char username[MAX_BUF];
    char password[MAX_BUF];

    /* check if user is not logged already */
    if (is_authenticated) {
        printf("Already logged. Please logout first!\n");
        return;
    }

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

    /* convert credentials to json */
    get_credentials_json(username, password);

    /* send post request */
    send_post_request(sockfd, url, NULL);
    free(credentials_json[0]);

    /* error if credentials don't match */
    char *ptr = strstr(response, "Set-Cookie: ");
    if (response == NULL || ptr == NULL) {
        is_authenticated = 0;
        has_access = 0;
        printf("Wrong credetials.\n");
        free(response);
        return;
    }

    /* save session cookie */
    strtok(ptr, ";");
    ptr += 12;
    int len = strlen(ptr) + 1; 
    char *temp = malloc(len * sizeof(char));

    if (!temp) {
        printf("malloc failed\n");
        free(response);
        return;
    }
    memcpy(temp, ptr, len);
    session_cookie[0] = temp;

    /* set flags for connection */
    is_authenticated = 1;
    has_access = 0;

    free(response);
    printf("Logged in successfully!\n");
}

void enter_library(int sockfd) {
    printf("-----enter library function-----\n");
    char url[LEN] = "/api/v1/tema/library/access";

    /* check if user is authenticated */
    if (!is_authenticated) {
        printf("Error. Authenticate first!\n");
        return;
    }

    send_get_delete_request(sockfd, url, jwt_token, 0);
    char *temp = strstr(response, "token");
    if (temp == NULL) {
        printf("Error\n");
        return;
    }

    /* save jwt_token token */
    temp += 8;

    /* get token */
    int len = strlen(temp) + 1;
    jwt_token = malloc(len * sizeof(char));
    if (jwt_token == NULL) {
        printf("malloc failed\n");
        return -1;
    }
    memcpy(jwt_token, temp, len);
    jwt_token[strlen(jwt_token) - 2] = '\0';

    printf("Entered library successfully!\n");
    has_access = 1;
    free(response);

}

void get_books(int sockfd) {
    printf("-----get books function-----\n");
    char url[LEN] = "/api/v1/tema/library/books";

    /* check if user has access to library */
    if (!has_access) {
        printf("Error. No access to library!\n");
        return;
    }

    send_get_delete_request(sockfd, url, jwt_token, 0);
    printf("%s\n", response);

    free(response);
}

void get_book(int sockfd) {
    printf("-----get book function-----\n");
    char book_id[MAX_BUF];
    char url[LEN] = "/api/v1/tema/library/books/";

    /* check if user has access to library */
    if (!has_access) {
        printf("Error. No access to library!\n");
        return;
    }

    /* get book id */
    printf("id=\n");
    fgets(book_id, MAX_BUF, stdin);

    /* check if id is valid */
    int nr = 0;
    char tmp[strlen(book_id) + 1];
    memcpy(tmp, book_id, strlen(book_id) + 1);

    char *token = strtok(tmp, " \n");
    while (token != NULL) {
        nr++;

        if (nr == 2) {
            printf("Invalid book id\n");
            return;
        }
        token = strtok(NULL, " \n");
    }

    /* check not to have letters  or symbols */
    book_id[strlen(book_id) - 1] = '\0';
    for (int i = 0; i < strlen(book_id); i++) {
        if (book_id[i] != '0' && book_id[i] != '1' && book_id[i] != '2' &&
            book_id[i] != '3' && book_id[i] != '4' && book_id[i] != '5' &&
            book_id[i] != '6' && book_id[i] != '7' && book_id[i] != '8' &&
            book_id[i] != '9') {
            printf("Invalid book id\n");
            return;
        }
    }

    strcat(url, book_id);
    send_get_delete_request(sockfd, url, jwt_token, 0);

    /* print output */
    if (strstr(response, "No book was found!") != NULL) {
        printf("Book not found.\n");
    } else {
        printf("%s\n", response);
    }

    free(response);
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
    char url[LEN] = "/api/v1/tema/library/books/";

    /* check if user has access to library */
    if (!has_access) {
        printf("Error. No access to library!\n");
        return;
    }

    /* get book id */
    printf("id=\n");
    fgets(book_id, MAX_BUF, stdin);

    /* check if id is valid */
    int nr = 0;
    char tmp[strlen(book_id) + 1];
    memcpy(tmp, book_id, strlen(book_id) + 1);

    char *token = strtok(tmp, " \n");
    while (token != NULL) {
        nr++;

        if (nr == 2) {
            printf("Invalid book id\n");
            return;
        }
        token = strtok(NULL, " \n");
    }

    /* check not to have letters  or symbols */
    book_id[strlen(book_id) - 1] = '\0';
    for (int i = 0; i < strlen(book_id); i++) {
        if (book_id[i] != '0' && book_id[i] != '1' && book_id[i] != '2' &&
            book_id[i] != '3' && book_id[i] != '4' && book_id[i] != '5' &&
            book_id[i] != '6' && book_id[i] != '7' && book_id[i] != '8' &&
            book_id[i] != '9') {
            printf("Invalid book id\n");
            return;
        }
    }

    strcat(url, book_id);
    send_get_delete_request(sockfd, url, jwt_token, 0);

    /* print output */
    if (strstr(response, "No book was found!") != NULL) {
        printf("Book not found.\n");
    } else {
        printf("Book deleted.\n");
    }

    free(response);
}

void logout(int sockfd) {
    printf("-----logout function-----\n");
    char url[LEN] = "/api/v1/tema/auth/logout";

    if (!is_authenticated) {
        printf("Not logged in\n");
        return;
    }

    send_get_delete_request(sockfd, url, jwt_token, 0);
    free(response);

    /* reset status */
    is_authenticated = 0;
    has_access = 0;

    /* free session cookie*/
    free(session_cookie[0]);
    session_cookie[0] = NULL;

    if (jwt_token != NULL) {
        free(jwt_token);
        jwt_token = NULL;
    }

    printf("Successfully logged out.\n");
}

/* ----------------------- MAIN -----------------------*/

int main(int argc, char *argv[])
{
    int sockfd;
    jwt_token = NULL;

    /* read commands from user */
    while(1) {
        memset(buffer, 0, sizeof(buffer));

        /* get command */
        printf("insert command\n");
        fgets(buffer, MAX_BUF, stdin);
        printf("%s", buffer);

        /* open connection with server */
        sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            printf("error sockfd");
            return -1;
        }

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
            close_connection(sockfd);
            break;
        } else {
            /* invalid command */
            printf("Invalid command\n");
        }
        close_connection(sockfd);
    }

    if (session_cookie[0] != NULL)
        free(session_cookie[0]);

    if (jwt_token != NULL)
        free(jwt_token);

    return 0;
}
