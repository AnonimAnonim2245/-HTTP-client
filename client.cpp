#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include  <iostream>
#include  <bits/stdc++.h>
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.hpp"
#include "requests.hpp"
#include "json.hpp"
using namespace std;
using json_t = nlohmann::json;

#define MAXLEN 500
bool isNumeric(string str) {
   for (int i = 0; i < str.length(); i++)
      if (isdigit(str[i]) == false)
         return false;
      return true;
}
int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd, sockfd2;
    int size_command = 32;
    int is_logged = 0;
    int is_token = 0;
    char *jwt = NULL;
    char *cookie = NULL;
    char** cookies = (char**)malloc(500 * sizeof(char*));
    int no_cookies = 0;
    char *access_token = (char*)malloc(BUFLEN * sizeof(char));
    char *BODY_DATA;
    char *command;
    char *SERVERADDR = "34.246.184.49 ";
    int PORTNO = 8080;
    int PORTNO2 = 80;
    

    while(1){
        std::string command;
        getline(cin, command);
        
        if(command == "register"){
            sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);
            char name[MAXLEN];
            char password[MAXLEN];
            printf("username=");
            cin.getline(name, MAXLEN);
            printf("password=");
            cin.getline(password, MAXLEN);
            json_t json = {{"username", name},{"password", password}};
            std::string s = json.dump();
            printf("%s\n", s.c_str());
            char* text = new char[s.length() + 1];
            strcpy(text, s.c_str());
            message = compute_post_request(SERVERADDR, "/api/v1/tema/auth/register", "application/json", &text, 1, jwt);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char *key = strstr(response, "ok");
            if(key!=NULL){
                printf("Utilizator înregistrat cu succes!\n");
                printf("SUCCESS: %s\n", message);
            }
            else{
                char *key = strstr(response, "error");
                key += strlen("error\":\"");
                char *key_end = strstr(key, "\"");
                *key_end = '\0';
                printf("ERROR: %s\n", key); 
            }
            close_connection(sockfd);
        }
        else if(command == "login"){
            sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);
            char name[MAXLEN];
            char password[MAXLEN];
            printf("username=");
            cin.getline(name, MAXLEN);
            printf("password=");
            cin.getline(password, MAXLEN);
            if(name == NULL || password == NULL){
                printf("Trebuie să introduci un nume de utilizator și o parolă!\n");
                continue;
            }
            json_t json = {{"username", name},{"password", password}};
            std::string s = json.dump();
            char* text = new char[s.length() + 1];  // Allocate enough space for s and the null terminator
            strcpy(text, s.c_str());
            message = compute_post_request(SERVERADDR, "/api/v1/tema/auth/login", "application/json", &text, 1, jwt);            
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if(response == NULL){
                continue;
            }
            char *key = strstr(response, "ok");
            if(key!=NULL){
                printf("Utilizator logat cu succes!\n");
                is_logged = 1;
                cookies[no_cookies] = (char*)malloc(BUFLEN * sizeof(char));
                cookies[no_cookies] = basic_extract_cookie_response(response);
                no_cookies++;

            }
            else{
                char *key = strstr(response, "error");
                key += strlen("error\":\"");
                char *key_end = strstr(key, "\"");
                *key_end = '\0';
                printf("ERROR: %s\n", key);    

            }
            close_connection(sockfd);
        }
        else if(command == "enter_library"){
            if(is_logged == 0){
                printf("Trebuie să fii logat pentru a accesa biblioteca!\n");
            }
            else{
                sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);
                message = compute_get_request(SERVERADDR, "/api/v1/tema/library/access", NULL, cookies, no_cookies, jwt);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                char *key = strstr(response, "ok");
                if(key!=NULL){
                    is_token = 1;
                    char *key = strstr(response, "token");
                    key += strlen("token\":\"");
                    char *key_end = strstr(key, "\"");
                    *key_end = '\0';
                    jwt = (char*)malloc(BUFLEN * sizeof(char));
                    strcpy(jwt, key);
                    printf("Ai acces la bibliotecă!\n");
                }
                else{
                    char *key = strstr(response, "error");
                    
                    key += strlen("error\":\"");
                    char *key_end = strstr(key, "\"");
                    *key_end = '\0';
                    printf("ERROR: %s\n", key);    
                }
                close_connection(sockfd);
            }

        }
        else if(command == "add_book"){
            if(is_token == 0){
                printf("Trebuie să ai acces la bibliotecă pentru a adăuga o carte!\n");
            }
            else{
                sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);
                char title[MAXLEN];
                char author[MAXLEN];
                char genre[MAXLEN];
                char publisher[MAXLEN];
                char page_count[MAXLEN];
                printf("title=");
                cin.getline(title, MAXLEN);
                printf("author=");
                cin.getline(author, MAXLEN);
                printf("genre=");
                cin.getline(genre, MAXLEN);
                printf("publisher=");
                cin.getline(publisher, MAXLEN);
                printf("page_count=");
                cin.getline(page_count, MAXLEN);

                if(isNumeric(page_count) == 0){
                    printf("Trebuie să introduci un număr de pagini valid!\n");
                    continue;
                }
                char *text = (char*)malloc(LINELEN * sizeof(char));
                json_t json = {{"title", title},{"author", author},{"genre", genre},{"page_count", stoi(page_count)},{"publisher", publisher}};
                std::string s = json.dump();
                strcpy(text, s.c_str());
                text[strlen(text)] = '\0';
                if(text == NULL){
                    printf("Trebuie să introduci datele corect!\n");
                    continue;
                }
                
                message = compute_post_request(SERVERADDR, "/api/v1/tema/library/books", "application/json", &text, 1, jwt);
                send_to_server(sockfd, message);
                if(response == NULL){
                    continue;
                }
                response = receive_from_server(sockfd);
                char *key = strstr(response, "ok");
                if(key!=NULL){
                    if(jwt == NULL){
                        printf("NULL\n");
                    }
                    else{
                        printf("NOT NULL\n");
                    }
                }
                else{
                    char *key = strstr(response, "error");
                    key += strlen("error\":\"");
                    char *key_end = strstr(key, "\"");
                    *key_end = '\0';
                    printf("ERROR: %s\n", key);    
                }
                close_connection(sockfd);
            }
            continue;
        }
        else if(command == "get_books"){
            if(is_token == 0){
                printf("Trebuie să ai acces la bibliotecă pentru a vedea cărțile!\n");
            }
            else{                
                sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);
                message = compute_get_request(SERVERADDR, "/api/v1/tema/library/books", NULL, cookies, no_cookies, jwt);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                char *key= strstr(response, "\r\n\r\n");
                key += strlen("\r\n\r\n");
                printf("%s\n", key);
                close_connection(sockfd);
            }
        }
        else if(command == "get_book"){
            if(is_token == 0){
                printf("Trebuie să ai acces la bibliotecă pentru a vedea cărțile!\n");
            }
            else{
                sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);
                char id[100];
                printf("id=");
                cin.getline(id, MAXLEN);
                char *url = (char*)malloc(LINELEN * sizeof(char));
                strcpy(url, "/api/v1/tema/library/books/");
                strcat(url, id);
                message = compute_get_request(SERVERADDR, url, NULL, cookies, no_cookies, jwt);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                char *key = strstr(response, "error");
                if(key==NULL){
                    printf("Carte accesata cu succes!\n");
                    char *key= strstr(response, "\r\n\r\n");
                    key += strlen("\r\n\r\n");
                    printf("%s\n", key);

                }
                else{
                    char *key = strstr(response, "error");
                    key += strlen("error\":\"");
                    char *key_end = strstr(key, "\"");
                    *key_end = '\0';
                    printf("Cartea cu id=%s nu exista\n", id);    
                }
                close_connection(sockfd);
            }
        }
        else if(command == "delete_book"){
            if(is_token == 0){
                printf("Trebuie să ai acces la bibliotecă pentru a șterge o carte!\n");
            }
            else{
                sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);
                char id[100];
                printf("id=");
                cin.getline(id, MAXLEN);
                char *url = (char*)malloc(LINELEN * sizeof(char));
                strcpy(url, "/api/v1/tema/library/books/");
                strcat(url, id);
                message = compute_delete_request(SERVERADDR, url, NULL, cookies, no_cookies, jwt);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                char *key = strstr(response, "ok");
                if(key!=NULL){
                    printf("Carte ștearsă cu succes!\n");
                }
                else{
                    char *key = strstr(response, "error");
                    key += strlen("error\":\"");
                    char *key_end = strstr(key, "\"");
                    *key_end = '\0';
                    printf("ERROR: %s\n", key);    
                }
                close_connection(sockfd);
            }
        }
        else if(command == "logout"){
            if(is_logged == 0){
                printf("Trebuie să fii logat pentru a te deconecta!\n");
            }
            else{
                sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);
                message = compute_get_request(SERVERADDR, "/api/v1/tema/auth/logout", NULL, cookies, no_cookies, jwt);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                char *key = strstr(response, "ok");
                if(key!=NULL){
                    printf("Deconectare cu succes!\n");
                    is_logged = 0;
                    is_token = 0;
                    free(jwt);
                    jwt = NULL;
                    for(int i = 0; i < no_cookies; i++){
                        if(cookies[i] != NULL){
                        cookies[i] = NULL;
                        }
                    }
                    no_cookies = 0;
                }
                else{
                    char *key = strstr(response, "error");
                    key += strlen("error\":\"");
                    char *key_end = strstr(key, "\"");
                    *key_end = '\0';
                    printf("ERROR: %s\n", key);    
                }
                close_connection(sockfd);
            }
        }
        else if(command == "exit"){
            break;
        }
        else{
            printf("Comanda nu există!\n");
        }
        

    }


    // Ex 1.1: GET dummy from main server
   
    /*sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);
    char *request_type = argv[0];
    char *request_data = argv[1];
    char *request_data2 = argv[2];
    message = compute_post_request(SERVERADDR, "/api/v1/tema/auth/register", "application/json", NULL, 0, NULL, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("Response:\n%s\n\n", response);
    close_connection(sockfd);
    sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);


    // Ex 1.2: POST dummy and print response from main server
    char *TEXT_POST = "Desteapta-te romane";

    message = compute_post_request(SERVERADDR, "/api/v1/dummy", "application/x-www-form-urlencoded", &TEXT_POST, 1, 0, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("Response:\n%s\n\n", response);
    close_connection(sockfd);

    // Ex 2: Login into main server
    sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);
    char *text = "username=student&password=student";

    message = compute_post_request(SERVERADDR, "/api/v1/auth/login", "application/x-www-form-urlencoded", &text, 1, NULL, 0);
    
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("Response:\n%s\n\n", response);
    char *cookie = basic_extract_cookie_response(response);

    // Ex 3: GET weather key from main server
    close_connection(sockfd);
    sockfd = open_connection(SERVERADDR, PORTNO, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(SERVERADDR, "/api/v1/weather/key", NULL, &cookie, 1);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("Response:\n%s\n\n", response);


    char *weather_key = basic_extract_key_response(response);

    // Ex 4: GET weather data from OpenWeather API

    char *OPENWEATHERHOST = "api.openweathermap.org";
    char *weather_data = calloc(LINELEN, sizeof(char));
    char *weather_endpoint = "/data/2.5/weather";
    sprintf(weather_data, "lat=%f&lon=%f&appid=%s", 44.7398, 22.2767, weather_key);
    struct hostent* weather_host = gethostbyname(OPENWEATHERHOST);
    char* weather_ip = inet_ntoa(*((struct in_addr *)weather_host->h_addr_list[0]));

    sockfd2 = open_connection(weather_ip, PORTNO2, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(weather_ip, weather_endpoint, weather_data, NULL, 0);
    send_to_server(sockfd2, message);
    response = receive_from_server(sockfd2);
    printf("Response:\n%s\n\n", response);
    
    // Ex 5: POST weather data for verification to main server
    char *weather_data2 = basic_extract_json_response(response);
    //printf("Weather data: %s\n", weather_data2);
    message = compute_post_request(SERVERADDR, "/api/v1/weather/44.7398/22.2767", "application/json", &weather_data2, 1, &cookie, 1);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("Response:\n%s\n\n", response);
    //printf("Cookie: %s\n", cookie);
    
    // Ex 6: Logout from main server
    message = compute_get_request(SERVERADDR, "/api/v1/auth/logout", NULL, &cookie, 1);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("Response:\n%s\n\n", response);
    // BONUS: make the main server return "Already logged in!"
    message = compute_post_request(SERVERADDR, "/api/v1/auth/login", "application/x-www-form-urlencoded", &text, 1, NULL, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("Response1:\n%s\n", response);
    char* cookie2 = basic_extract_cookie_response(response);

    message = compute_post_request(SERVERADDR, "/api/v1/auth/login", "application/x-www-form-urlencoded", &text, 1, &cookie2, 1);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("Response2:\n%s\n", response);
    close_connection(sockfd);
    free(message);
    free(response);*/

    // free the allocated data at the end!

    return 0;
}
