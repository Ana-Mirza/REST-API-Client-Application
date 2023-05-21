Name: Mirza Ana-Maria

Group: 321CA

## Homework 4

## Description
The scope of this homework was creating a client that would receive commands
from standard input and communicate with a REST API exposed by a server through
the HTTP protocol. The server simulates a virtual library receiving commands 
from the client.

## Flow of the Application
The interaction with the server is possible using the following configurations:

```
HOST: 34.254.242.81
PORT: 8080
```

For each of valid command from the user, the client creates a new HTTP 
GET/POST/DELETE request and sends it to the server.
The server allows for the following actions:

1. Register - allows the user to register a new account in the library database
by giving the username and password

```
register
username=<input_username>
password=<input_password>
```

If the user is already existent, the server responds with an error message.

2. Login - allows the user to login with an existend account

```
login
username=<input_username>
password=<input_password>
```

If the account is not valid, the server responds with an appropriate error. Otherwise, it returns a session cookie.

3. Enter library - allows the user to gain access in the library

```
enter_library
```

If the user is logged, the server returns a JWT token that gives access to the
library. Otherwise, the server returns a specific error.

4. Get books - allows the user to see all the books available

```
get_books
```

If the user is logged and has access to the library, a json output containing
all the books in the library for that account is returned by the server.
Otherwise, an error will be returned.

5. Get book - allows the user to see details of a book in the library

```
get_book
id=<input_book_id>
```

The server returns a json string containing the book specitications, or an error
if the book id is not valid or the user is not logged/has no access to library.

6. Add book - allows the user to add a book in the library

```
add_book
title=<input_title>
author=<input_author>
genre=<input_genre>
publisher=<input_publisher>
page_count=<input_page_nr>
```

For this command, the server adds the book in the library if all parameters are
valid and the user has access to library.

7. Delete book - allows user to delete a book from the library

```
delete_book
id=<input_book_id>
```

The server deletes the book with the given id, if existent, and if the user has access to library.

8. Logout - allows the user to logout from the account

```
logout
```

The logout is successful if the user was previously logged in. It invalidates
the session cookies and JWT token so that access to the library is no more
available.

## Additional Error Cases
The user can receive an error in the following situations:

* the command is not one of the above (invalid command)
* username or password have spaces included (login/register)
* register with existent account
* login with non-existent account
* login with wrong password
* title/author/publisher/genre is empty
* id is not a number
* page_count is not a number or is 0
