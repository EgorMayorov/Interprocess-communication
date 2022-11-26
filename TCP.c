#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<errno.h>

#define PORTNUM 1300
#define BUFLEN 256

int sockfd, newsockfd;

void handler (int sig){
    shutdown(newsockfd, 2);
    close(newsockfd);
    shutdown(sockfd, 2);
    close(sockfd);
    printf("\nРабота программы завершена сигналом SIGINT\n");
    exit(0);
}

void f (char *buf){
    for (int i = 0; buf[i] != '\0'; i++){
        if ((buf[i] >= 'a') && (buf[i] <= 'z')) buf[i] += 'A'-'a';
    }
}

int main(int argc, char* argv[]){
    if (argc != 2) return 0;
    signal(SIGINT, handler);
    errno = 0;
    struct sockaddr_in server_addr, client_addr;
    char buf[BUFLEN];
    if (!strcmp(argv[1], "client")){
        printf("Программа запущена как клиент\n");
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0) {
            printf("Ошибка при создании сокета\n%s\n", strerror(errno));
            return 0;
        }
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(PORTNUM);
        client_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORTNUM);
        int ok = inet_aton("192.168.55.208", &(server_addr.sin_addr));      //возможно нужно изменять адрес
        if (!ok){
            printf("Невалидный IP-адрес\n%s\n", strerror(errno));
            return 0;
        }
        if (bind(sockfd, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0){
            printf("Ошибка функции bind\n%s\n", strerror(errno));
            return 0;
        }
        int server_len = sizeof(server_addr);
        if (connect(sockfd, (struct sockaddr *) &server_addr, server_len) < 0){
            printf("Не удалось установить соединение с сервером\n%s\n", strerror(errno));
            return 0;
        }
        while (1) {
            fgets(buf, BUFLEN, stdin);
            if (send(sockfd, buf, strlen(buf)+1, 0) < 0){
                printf("Ошибка отправки сообщения\n%s\n", strerror(errno));
                return 0;
            }
            if (recv(sockfd, &buf, BUFLEN, 0) < 0){
                printf("Ошибка получения сообщения\n%s\n", strerror(errno));
                return 0;
            }
            if ((!strcmp(buf, "QUIT\n")) || (!strcmp(buf, "END SERVER\n"))) break;
            if ((!strcmp(buf, "QUIT")) || (!strcmp(buf, "END SERVER"))){
                printf("\n");
                break;
            }
            printf("%s", buf);
        }
        shutdown(sockfd, 2);
        close(sockfd);
        if (!strncmp(buf, "QUIT", 4))
            printf("Работа клиента закончена получением строки quit\n");
        if (!strncmp(buf, "END", 3))
            printf("Работа клиента закончена получением строки end server\n");
    }else if (!strcmp(argv[1], "server")){
        printf("Программа запущена как сервер\n");
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0) {
            printf("Ошибка при создании сокета\n%s\n", strerror(errno));
            return 0;
        }
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORTNUM);
        int ok = inet_aton("192.168.55.208", &(server_addr.sin_addr));      //возможно стоит изменять адрес
        if (!ok){
            printf("Невалидный IP-адрес\n%s\n", strerror(errno));
            return 0;
        }
        if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
            printf("Ошибка функции bind\n%s\n", strerror(errno));
            return 0;
        }
        if (listen(sockfd, 5) < 0){
            printf("Ошибка прослушивания очереди\n%s\n", strerror(errno));
            return 0;
        }
        while (1){
            memset(&client_addr, 0, sizeof(client_addr));
            int client_len = sizeof(client_addr);
            if ((newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len)) < 0){
                printf("Ошибка функции accept\n%s\n", strerror(errno));
                return 0;
            }
            while (1){
                if (recv(newsockfd, &buf, BUFLEN, 0) < 0){
                    printf("Ошибка чтения сокета\n%s\n", strerror(errno));
                    return 0;
                }
                f(buf);
                if (send(newsockfd, buf, strlen(buf)+1, 0) < 0){
                    printf("Ошибка записи в сокет\n%s\n", strerror(errno));
                    return 0;
                }
                if ((!strcmp(buf, "QUIT\n")) || (!strcmp(buf, "QUIT"))) break;
                if ((!strcmp(buf, "END SERVER\n")) || (!strcmp(buf, "END SERVER\n"))) break; 
            }
            shutdown(newsockfd, 2);
            close(newsockfd);
            if (!strcmp(buf, "END SERVER\n")) break;
            if (!strcmp(buf, "END SERVER")){
                printf("\n");
                break;
            }
        }
        shutdown(sockfd, 2);
        close(sockfd);
        printf("Работа сервера завершена получением строки END SERVER\n");
    }else{
        printf("Неверные данные\n");
        return 0;
    }
    return 0;
}
