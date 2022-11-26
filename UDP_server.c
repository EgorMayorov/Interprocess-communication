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

#define PORTNUM 1300
#define BUFLEN 256

void f (char *buf){
    for (int i = 0; buf[i] != '\0'; i++){
        if ((buf[i] >= 'a') && (buf[i] <= 'z')) buf[i] += 'A'-'a';
    }
}

void handler (int sig){
    printf("\nРабота сервера завершена сигналом SIGINT\n");
    exit(0);
}

int main(){
    signal(SIGINT, handler);
    struct sockaddr_in server_addr, client_addr;
    char buf[BUFLEN];
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        printf("Ошибка при создании сокета\n");
        return 0;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORTNUM);
    int ok = inet_aton("192.168.1.42", &(server_addr.sin_addr));        //возможно нужно изменять адрес
    if (!ok){
        printf("Невалидный IP-адрес\n");
        return 0;
    }
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        printf("Ошибка функции bind\n");
        return 0;
    }
    /*if (listen(sockfd, 10) < 0){
        printf("Ошибка прослушивания очереди\n");
        return 0;
    }*/
    while (1){
        memset(&client_addr, 0, sizeof(client_addr));
        int client_len = sizeof(client_addr);
        if (recvfrom(sockfd, &buf, BUFLEN, 0, (struct sockaddr *) &client_addr, &client_len) < 0){
            printf("Ошибка чтения сокета\n");
            return 0;
        }
        f(buf);
        if (sendto(sockfd, buf, strlen(buf)+1, 0, (const struct sockaddr *) &client_addr, client_len) < 0){
            printf("Ошибка записи в сокет\n");
            return 0;
        }
        if (!strcmp(buf, "QUIT\n")) break;
        if (!strcmp(buf, "QUIT")){
            printf("\n");
            break;
        }
    }
    shutdown(sockfd, 2);
    close(sockfd);
    printf("Работа сервера завершена получением строки QUIT\n");
    return 0;
}
