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
#include <ctype.h>


int main(int argc, char *argv[])
{
    char buffer[100], myToken[10000];
    char *cookie = malloc(1000*sizeof(char));
    int status = 0, access = 0;
    char *token = malloc(1000*sizeof(char));

    while(1) {
        fgets(buffer,100,stdin);
        buffer[strcspn(buffer, "\r\n")] = 0; 

        if(strcmp(buffer,"register") == 0)
        {
            char username[100];
            printf("username=");
            fgets(username,100,stdin);
            username[strcspn(username, "\r\n")] = 0;

            char password[100];
            printf("password=");
            fgets(password,100,stdin);
            password[strcspn(password, "\r\n")] = 0;
            
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            json_object_set_string(root_object, "username", username);
            json_object_set_string(root_object, "password", password);
            char *serialized_string = json_serialize_to_string_pretty(root_value);
            
            char *message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register","application/json", serialized_string, NULL);

            int socket = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            
            send_to_server(socket, message);

            char *response = receive_from_server(socket);
            char *p = strtok(response,"{");
            p = strtok(NULL,"}");
            
            json_free_serialized_string(serialized_string);
            json_value_free(root_value);
            
            if(p==NULL){
                printf("You have registered successfully!\n");
            }
            else{
                printf("Error: The username %s is taken!\n",username);
            }
            
            close_connection(socket);
            continue;
        }
        else if(strcmp(buffer,"login") == 0) {
            if(status == 1)
            {
                printf("Error: You are already logged in\n");
                continue;
            }

            char username[100];
            printf("username=");
            fgets(username,100,stdin);
            username[strcspn(username, "\r\n")] = 0;
            char password[100];
            printf("password=");
            fgets(password,100,stdin);
            password[strcspn(password, "\r\n")] = 0;
            

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            json_object_set_string(root_object, "username", username);
            json_object_set_string(root_object, "password", password);
            char *serialized_string = json_serialize_to_string_pretty(root_value);

            
            char *message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login","application/json", serialized_string, NULL);

            int socket = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            
            send_to_server(socket, message);

            char *response = receive_from_server(socket);
            // printf("%s",response);
            char *p = strtok(response,"{");
            p = strtok(NULL,"}");
            
            json_free_serialized_string(serialized_string);
            json_value_free(root_value);
            
            if(p==NULL){
                printf("You have logged in successfully!\n");
            }
            else{
                printf("Error: Credentials are not good!\n");
                continue;
            }

            response = strstr(response, "connect.sid=");
	        response = strtok(response , ";");
            strcpy(cookie,response);
            close_connection(socket);
            status = 1;
            continue;
        }
        else if(strcmp(buffer,"enter_library") == 0) {
            if(status == 0) {
                printf("Error: You must be logged in!\n");
                continue;
            }
            else
                printf("You have entered the library successflly!\n");
            access = 1;

            char *message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access", cookie, NULL);

            int socket = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            
            send_to_server(socket, message);

            char *response = receive_from_server(socket);
            char *p = strtok(response,"{");
            p = strtok(NULL,"}");
            
	        strcpy(token, p+9);
            token[strcspn(token, "\"")] = 0;
            strcpy(myToken,token); //nu stiu de ce nu merge sa lucrez direct cu tokenul

            close_connection(socket);
            continue;
        }
        else if(strcmp(buffer,"get_books") == 0) {
            if(status == 0) {
                printf("Error: You must be logged in!\n");
                continue;
            }

            char *message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books", NULL, myToken);

            int socket = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            
            send_to_server(socket, message);

            char *response = receive_from_server(socket);
            char *p = strtok(response,"[");
            p = strtok(NULL,"]");
            if(p==NULL){
                printf("Error: You haven't entered the library yet\n");
                close_connection(socket);
                continue;
            }
            printf("Books:\n%s\n",p);
            close_connection(socket);
            continue;
        }
        else if(strcmp(buffer,"get_book") == 0) {
           if(status == 0) {
                printf("Error: You must be logged in!\n");
                continue;
            }
            if(access == 0) {
                printf("Error: You haven't entered the library yet\n");
                continue;
            }
            char id[100];
            printf("id=");
            fgets(id,100,stdin);
            id[strcspn(id, "\n\r")] = 0;
            char addr[1000];
            sprintf(addr,"/api/v1/tema/library/books/%s",id);
            char *message = compute_get_request("34.241.4.235", addr, NULL, myToken);

            int socket = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            
            send_to_server(socket, message);

            char *response = receive_from_server(socket);
            char *p = strtok(response,"[");
            p = strtok(NULL,"]");
            if(p==NULL){
                printf("Error: There is no book with that id\n");
                close_connection(socket);
                continue;
            }
            printf("The book you are looking for:\n%s\n",p);
            close_connection(socket);
            continue;
        }
        else if(strcmp(buffer,"add_book") == 0) {

           if(status == 0) {
                printf("Error: You must be logged in!\n");
                continue;
            }
            if(access == 0) {
                printf("Error: You haven't entered the library yet\n");
                continue;
            }
            char title[100],author[100],genre[100],publisher[100],page_count_s[100];
            int page_count;

            printf("title=");
            fgets(title,100,stdin);
            title[strcspn(title, "\n\r")] = 0;

            printf("author=");
            fgets(author,100,stdin);
            author[strcspn(author, "\n\r")] = 0;

            printf("genre=");
            fgets(genre,100,stdin);
            genre[strcspn(genre, "\n\r")] = 0;

            printf("publisher=");
            fgets(publisher,100,stdin);
            publisher[strcspn(publisher, "\n\r")] = 0;
            
            printf("page_count=");
            fgets(page_count_s,100,stdin);
            page_count_s[strcspn(page_count_s, "\n\r")] = 0;

            int ok = 1;
            for (int i = 0; page_count_s[i]!= '\0'; i++)
            {
                if (isdigit(page_count_s[i]) == 0){
                    printf("Error: You should have entered a number\n");
                    ok = 0;
                    break;
                }
            }
            if(ok == 0)
                continue;

            page_count = atoi(page_count_s);

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            char *serialized_string = NULL;

            json_object_set_string(root_object, "title", title);
            json_object_set_string(root_object, "author", author);
            json_object_set_string(root_object, "publisher", publisher);
            json_object_set_string(root_object, "genre", genre);
            json_object_set_number(root_object, "page_count", page_count);

            serialized_string = json_serialize_to_string_pretty(root_value);

            char *message = compute_post_request("34.241.4.235", "/api/v1/tema/library/books","application/json", serialized_string, myToken);
            
            int socket = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            
            send_to_server(socket, message);

            json_free_serialized_string(serialized_string);
            json_value_free(root_value);

            char *response = receive_from_server(socket);	
            if(strncmp(response+13,"OK",2) != 0)
            {
                printf("Error when trying to add book\n");
                close_connection(socket);
                continue;
            }
            printf("Book added successfully\n");
            close_connection(socket);
            continue;
        }
        else if(strcmp(buffer,"delete_book") == 0) {
            if(status == 0) {
                printf("Error: You must be logged in!\n");
                continue;
            }
            if(access == 0) {
                printf("Error: You haven't entered the library yet\n");
                continue;
            }
            char id[100];
            printf("id=");
            fgets(id,100,stdin);
            id[strcspn(id, "\n\r")] = 0;
            char addr[1000];
            sprintf(addr,"/api/v1/tema/library/books/%s",id);
            char *message = compute_delete_request("34.241.4.235", addr, myToken);

            int socket = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            
            send_to_server(socket, message);

            char *response = receive_from_server(socket);
            if(strncmp(response+13,"OK",2) != 0)
            {
                printf("Error: No book with that id\n");
                close_connection(socket);
                continue;
            }
            printf("You have successfully removed the book\n");
            close_connection(socket);
            continue;
        }
        else if(strcmp(buffer,"logout") == 0) {

            if(status == 0)
            {
                printf("Error: You are not logged in\n");
                continue;
            }
            
            char *message = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout", cookie, NULL);

            int socket = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            
            send_to_server(socket, message);

            char *response = receive_from_server(socket);
            char *p = strtok(response,"{");
            p = strtok(NULL,"}");
            
            
            if(p==NULL){
                printf("You have logged out successfully!\n");
            }
            else{
                printf("%s\n",p);
                continue;
            }

            memset(cookie, 0, 100);
            
            close_connection(socket);

            status = 0;
            access = 0;

            continue;
        }
        else if(strcmp(buffer,"exit") == 0) {
            break;
        }
        else {
            printf("Wrong command\n");
        }
    }
    free(cookie);
    free(token);
    return 0;
}
