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
/* buffer for book to be added */
char *book_buf[1];


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

void send_post_request(int sockfd, char url[], char *credentials[1], char token[MAX_BUF]) {
    char content_type[LEN] = "application/json";

    request = compute_post_request("34.254.242.81:8080", url, content_type, credentials, 1, NULL, 0, token);

    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);

    /* free memory */
    free(request);
}

void send_get_delete_request(int sockfd, char url[], char token[], int delete_request) {
    request = compute_get_request("34.254.242.81:8080", url, NULL, session_cookie, 1, token, delete_request);

    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);

    /* free memory */
    free(request);
}

/* ----------------------- helper commands functions ----------------------- */

void register_user(int sockfd) {
    char url[LEN] = "/api/v1/tema/auth/register";
    char username[MAX_BUF];
    char password[MAX_BUF];

    /* get username */
    printf("username=");
    fgets(username, MAX_BUF, stdin);

    /* get password */
    printf("password=");
    fgets(password, MAX_BUF, stdin);

    /* check if user is already logged */
    if (is_authenticated) {
        printf("Already logged in. Log out first!\n");
        return;
    }

    /* check if username has spaces */
    int nr = 0;
    char tmp[strlen(username) + 1];
    memcpy(tmp, username, strlen(username) + 1);

    char *token = strtok(tmp, " \n");
    while (token != NULL) {
        nr++;

        if (nr == 2) {
            printf("Invalid username!\n");
            return;
        }
        token = strtok(NULL, " \n");
    }

    /* check if password has spaces */
    nr = 0;
    char tmp2[sizeof(password) + 1];
    memcpy(tmp2, password, strlen(password) + 1);

    token = strtok(tmp2, " \n");
    while (token != NULL) {
        nr++;

        if (nr == 2) {
            printf("Invalid password!\n");
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
    send_post_request(sockfd, url, credentials_json, NULL);
    free(credentials_json[0]);

    if (strstr(response, "200 OK") != NULL || strstr(response, "201 Created") != NULL) {
        printf("Registration successful!\n");
    } else if (strstr(response, "error") != NULL) {
        /* get error and print */
        char *tmp = strtok(response, "{");
        tmp = strtok(NULL, "\n{");
        tmp += 9;
        tmp[strlen(tmp) - 2] = '\0';

        char str[strlen(tmp)];
        strcpy(str, tmp);
        str[strlen(tmp) + 1] = '\0';

        printf("%s\n", str);
    } else {
        printf("Error!\n");
    }

    free(response);
}

void login(int sockfd) {
    char url[LEN] = "/api/v1/tema/auth/login";

    char username[MAX_BUF];
    char password[MAX_BUF];

    /* get username */
    printf("username=");
    fgets(username, MAX_BUF, stdin);

    /* get password */
    printf("password=");
    fgets(password, MAX_BUF, stdin);

    /* check if user is not logged already */
    if (is_authenticated) {
        printf("Already logged. Please logout first!\n");
        return;
    }

    /* check if username has spaces */
    int nr = 0;
    char tmp[strlen(username) + 1];
    memcpy(tmp, username, strlen(username) + 1);

    char *token = strtok(tmp, " \n");
    while (token != NULL) {
        nr++;

        if (nr == 2) {
            printf("Invalid username!\n");
            return;
        }
        token = strtok(NULL, " \n");
    }

    /* check if password has spaces */
    nr = 0;
    char tmp2[sizeof(password) + 1];
    memcpy(tmp2, password, strlen(password) + 1);

    token = strtok(tmp2, " \n");
    while (token != NULL) {
        nr++;

        if (nr == 2) {
            printf("Invalid password!\n");
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
    send_post_request(sockfd, url, credentials_json, NULL);
    free(credentials_json[0]);

    /* error if credentials don't match */
    char *ptr = strstr(response, "Set-Cookie: ");
    if (response == NULL || ptr == NULL || strstr(response, "error") != NULL) {
        is_authenticated = 0;
        has_access = 0;
        
        /* get error and print */
        char *tmp = strtok(response, "{");
        tmp = strtok(NULL, "\n{");
        tmp += 9;
        tmp[strlen(tmp) - 2] = '\0';

        char str[strlen(tmp)];
        strcpy(str, tmp);
        str[strlen(tmp) + 1] = '\0';

        printf("%s\n", str);

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

    if (strstr(response, "200 OK") != NULL) {
        /* set flags for connection */
        is_authenticated = 1;
        has_access = 0;

        printf("Logged in successfully!\n");
    } else {
        printf("Error.\n");
    }

    free(response);
}

void enter_library(int sockfd) {
    char url[LEN] = "/api/v1/tema/library/access";

    /* check if user is authenticated */
    if (!is_authenticated) {
        printf("Authenticate first!\n");
        return;
    }

    send_get_delete_request(sockfd, url, jwt_token, 0);
    char *temp = strstr(response, "token");
    if (temp == NULL || strstr(response, "error") != NULL) {
        /* get error and print */
        char *tmp = strtok(response, "{");
        tmp = strtok(NULL, "\n{");
        tmp += 9;
        tmp[strlen(tmp) - 2] = '\0';

        char str[strlen(tmp)];
        strcpy(str, tmp);
        str[strlen(tmp) + 1] = '\0';

        printf("%s\n", str);

        free(response);
        return;
    }

    /* save jwt_token token */
    temp += 8;

    /* get token */
    int len = strlen(temp) + 1;
    jwt_token = malloc(len * sizeof(char));
    if (jwt_token == NULL) {
        printf("malloc failed\n");
        return;
    }
    memcpy(jwt_token, temp, len);
    jwt_token[strlen(jwt_token) - 2] = '\0';

    if (strstr(response, "200 OK") != NULL) {
        printf("Entered library successfully!\n");
        has_access = 1;
    } else {
        printf("Error.\n");
    }
    
    free(response);
}

void get_books(int sockfd) {
    char url[LEN] = "/api/v1/tema/library/books";

    /* check if user has access to library */
    if (!has_access) {
        printf("No access to library!\n");
        return;
    }

    send_get_delete_request(sockfd, url, jwt_token, 0);

    /* get list of books from response */
    char *tmp = strtok(response, "[");
    tmp = strtok(NULL, "\n[");
    char str[strlen(tmp) + 2];
    str[0] = '[';
    strcpy(&str[1], tmp);
    str[strlen(tmp) + 1] = '\0';

    /* get library in json format */
    JSON_Value *val = json_parse_string(str);
    char *books = json_serialize_to_string_pretty(val);
    printf("%s\n\n", books);

    json_free_serialized_string(books);
    json_value_free(val);

    free(response);
}

void get_book(int sockfd) {
    char book_id[MAX_BUF];
    char url[LEN] = "/api/v1/tema/library/books/";

    /* get book id */
    printf("id=");
    fgets(book_id, MAX_BUF, stdin);

    /* check if user has access to library */
    if (!has_access) {
        printf("No access to library!\n");
        return;
    }

    /* check if id is valid */
    int nr = 0;
    char tmp[strlen(book_id) + 1];
    memcpy(tmp, book_id, strlen(book_id) + 1);

    char *token = strtok(tmp, " \n");
    while (token != NULL) {
        nr++;

        if (nr == 2) {
            printf("Invalid book id!\n");
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
            printf("Invalid book id!\n");
            return;
        }
    }

    strcat(url, book_id);
    send_get_delete_request(sockfd, url, jwt_token, 0);

    if (strstr(response, "200 OK") != NULL) {
        char *tmp = strtok(response, "{");
        tmp = strtok(NULL, "\n{");
        char str[strlen(tmp) + 2];
        str[0] = '{';
        strcpy(&str[1], tmp);
        str[strlen(tmp) + 1] = '\0';

        /* get library in json format */
        JSON_Value *val = json_parse_string(str);
        char *books = json_serialize_to_string_pretty(val);
        printf("%s\n\n", books);

        json_free_serialized_string(books);
        json_value_free(val);
    } else if (strstr(response, "error") != NULL) {
        /* get error and print */
        char *tmp = strtok(response, "{");
        tmp = strtok(NULL, "\n{");
        tmp += 9;
        tmp[strlen(tmp) - 2] = '\0';

        char str[strlen(tmp)];
        strcpy(str, tmp);
        str[strlen(tmp) + 1] = '\0';

        printf("%s\n", str);
    } else {
        printf("Error!\n");
    }

    free(response);
}

void add_book(int sockfd) {
    char url[LEN] = "/api/v1/tema/library/books";

    char title[MAX_BUF], author[MAX_BUF], genre[MAX_BUF];
    char publisher[MAX_BUF], page_count[MAX_BUF];

    /* get title */
    printf("title=");

    memset(title, 0, MAX_BUF);
    fgets(title, MAX_BUF, stdin);
    title[strlen(title) - 1] = '\0';

    /* get author */
    printf("author=");

    memset(author, 0, MAX_BUF);
    fgets(author, MAX_BUF, stdin);
    author[strlen(author) - 1] = '\0';

    /* get genre */
    printf("genre=");

    memset(genre, 0, MAX_BUF);
    fgets(genre, MAX_BUF, stdin);
    genre[strlen(genre) - 1] = '\0';

    /* get publisher */
    printf("publisher=");

    memset(publisher, 0, MAX_BUF);
    fgets(publisher, MAX_BUF, stdin);
    publisher[strlen(publisher) - 1] = '\0';

    /* get page count */
    printf("page_count=");

    memset(page_count, 0, MAX_BUF);
    fgets(page_count, MAX_BUF, stdin);

    /* check if user has access to library */
    if (!has_access) {
        printf("No access to library!\n");
        return;
    }

    /* checkif title is valid */
    if (strcmp(title, "") == 0) {
        printf("Invalid title!\n");
        return;
    }

    /* check if author is valid */
    if (strcmp(author, "") == 0) {
        printf("Invalid author!\n");
        return;
    }

    /* check if genre is valid */
    if (strcmp(genre, "") == 0) {
        printf("Invalid genre!\n");
        return;
    }

    /* check if publisher is valid */
    if (strcmp(publisher, "") == 0) {
        printf("Invalid publisher!\n");
        return;
    }

    /* check if page count is valid */
    int nr = 0;
    char tmp[strlen(page_count) + 1];
    memcpy(tmp, page_count, strlen(page_count) + 1);

    char *token = strtok(tmp, " \n");
    while (token != NULL) {
        nr++;

        if (nr == 2) {
            printf("Invalid page count!\n");
            return;
        }
        token = strtok(NULL, " \n");
    }

    /* check not to have letters or symbols */
    page_count[strlen(page_count) - 1] = '\0';
    for (int i = 0; i < strlen(page_count); i++) {
        if (i == 0 && page_count[i] == '0') {
            printf("Invalid page count!\n");
            return;
        }

        if (page_count[i] != '0' && page_count[i] != '1' && page_count[i] != '2' &&
            page_count[i] != '3' && page_count[i] != '4' && page_count[i] != '5' &&
            page_count[i] != '6' && page_count[i] != '7' && page_count[i] != '8' &&
            page_count[i] != '9') {
            printf("Invalid page count!\n");
            return;
        }
    }

    /* compute payload */
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    json_object_set_string(obj, "title", title);
    json_object_set_string(obj, "author", author);
    json_object_set_string(obj, "genre", genre);
    json_object_set_string(obj, "page_count", page_count);
    json_object_set_string(obj, "publisher", publisher);
    book_buf[0] = json_serialize_to_string(val);

    send_post_request(sockfd, url, book_buf, jwt_token);
    
    if (strstr(response, "200 OK") != NULL) {
        printf("Book successfully added!\n");
    } else if (strstr(response, "error") != NULL) {
        /* get error and print */
        char *tmp = strtok(response, "{");
        tmp = strtok(NULL, "\n{");
        tmp += 9;
        tmp[strlen(tmp) - 2] = '\0';

        char str[strlen(tmp)];
        strcpy(str, tmp);
        str[strlen(tmp) + 1] = '\0';

        printf("%s\n", str);
    } else {
        printf("Error!\n");
    }

    /* free memory */
    free(book_buf[0]);
    json_value_free(val);
    free(response);
}

void delete_book(int sockfd) {
    char book_id[MAX_BUF];
    char url[LEN] = "/api/v1/tema/library/books/";

    /* get book id */
    printf("id=");
    fgets(book_id, MAX_BUF, stdin);

    /* check if user has access to library */
    if (!has_access) {
        printf("No access to library!\n");
        return;
    }

    /* check if id is valid */
    int nr = 0;
    char tmp[strlen(book_id) + 1];
    memcpy(tmp, book_id, strlen(book_id) + 1);

    char *token = strtok(tmp, " \n");
    while (token != NULL) {
        nr++;

        if (nr == 2) {
            printf("Invalid book id!\n");
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
            printf("Invalid book id!\n");
            return;
        }
    }

    strcat(url, book_id);
    send_get_delete_request(sockfd, url, jwt_token, 1);

    /* print output */
    if (strstr(response, "200 OK") != NULL) {
        printf("Book deleted successfully.\n");
    } else if (strstr(response, "error") != NULL) {
        /* get error and print */
        char *tmp = strtok(response, "{");
        tmp = strtok(NULL, "\n{");
        tmp += 9;
        tmp[strlen(tmp) - 2] = '\0';

        char str[strlen(tmp)];
        strcpy(str, tmp);
        str[strlen(tmp) + 1] = '\0';

        printf("%s\n", str);
    } else {
        printf("Error!\n");
    }

    free(response);
}

void logout(int sockfd) {
    char url[LEN] = "/api/v1/tema/auth/logout";

    if (!is_authenticated) {
        printf("Not logged in!\n");
        return;
    }

    send_get_delete_request(sockfd, url, jwt_token, 0);

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

    if (strstr(response, "200 OK") != NULL) {
        printf("Logged out successfully!\n");
    } else if (strstr(response, "error") != NULL) {
        /* get error and print */
        char *tmp = strtok(response, "{");
        tmp = strtok(NULL, "\n{");
        tmp += 9;
        tmp[strlen(tmp) - 2] = '\0';

        char str[strlen(tmp)];
        strcpy(str, tmp);
        str[strlen(tmp) + 1] = '\0';

        printf("%s\n", str);
    } else {
        printf("Error!\n");
    }

    free(response);
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
        fgets(buffer, MAX_BUF, stdin);

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
            printf("Invalid command!\n");
        }
        close_connection(sockfd);
    }

    if (session_cookie[0] != NULL)
        free(session_cookie[0]);

    if (jwt_token != NULL)
        free(jwt_token);

    return 0;
}
