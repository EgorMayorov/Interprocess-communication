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

void handler (int sig){
    printf("\nРабота клиента завершена сигналом SIGINT\n");
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
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORTNUM);
    client_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORTNUM);
    int ok = inet_aton("192.168.1.42", &(server_addr.sin_addr));        //возможно нужно изменять адрес
    if (!ok){
        printf("Невалидный IP-адрес\n");
        return 0;
    }
    if (bind(sockfd, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0){
        printf("Ошибка функции bind\n");
        return 0;
    }
    while (1) {
        fgets(buf, BUFLEN, stdin);
        int server_len = sizeof(server_addr);
        /*if (!strcmp(buf, "quit\n")) break;
        if (!strcmp(buf, "quit")){
            printf("\n");
            break;
        }*/
        if (sendto(sockfd, buf, strlen(buf)+1, 0, (const struct sockaddr *) &server_addr, server_len) < 0){
            printf("Ошибка отправки сообщения\n");
            return 0;
        }
        if (recvfrom(sockfd, &buf, BUFLEN, 0, (struct sockaddr *) &server_addr, &server_len) < 0){
            printf("Ошибка получения сообщения\n");
            return 0;
        }
        if (!strcmp(buf, "QUIT\n")) break;
        if (!strcmp(buf, "QUIT")){
            printf("\n");
            break;
        }
        printf("%s", buf);
    }
    shutdown(sockfd, 2);
    close(sockfd);
    printf("Работа клиента закончена получением строки quit\n");
    return 0;
}
