#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <csignal>


int createTCPIpv4Socket();

sockaddr_in* createIPv4Address(char *ip, int port);

void startListeningAndPrintMessagesOnNewThread(int fd);

void listenAndPrint(int fd);

int main() {

    int socketFD = createTCPIpv4Socket();
    struct sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);

    int result = connect(socketFD, reinterpret_cast<const sockaddr *>(address), sizeof (*address));

    if (result == 0)
        printf("Connessione effettuata!\n");

    char *name = NULL;
    size_t nameSize = 0;
    printf("Inserisci il tuo nome...\n");
    ssize_t nameCount = getline(&name, &nameSize, stdin);
    name[nameCount-1] = 0;


    char *line = NULL;
    size_t lineSize = 0;
    printf("Invia il tuo messaggio(scrivi exit)...\n");


    startListeningAndPrintMessagesOnNewThread(socketFD);

    char buffer[1024];

    while(true){

        ssize_t charCount = getline(&line, &lineSize, stdin);
        line[charCount-1] = 0;

        sprintf(buffer, "%s:%s", name, line);

        if(charCount>0){
            if(strcmp(line, "exit")==0){
                break;
            }
            ssize_t amountWasSent = send(socketFD, buffer, strlen(buffer), 0);
        }
    }

    close(socketFD);

    return 0;
}

void startListeningAndPrintMessagesOnNewThread(int fd) {

    pthread_t id;
    pthread_create(&id, NULL, reinterpret_cast<void *(*)(void *)>(listenAndPrint), reinterpret_cast<void *>(fd));

}

void listenAndPrint(int fd) {
    char buffer[1024];

    while(true){

        ssize_t amountReceived = recv(fd, buffer, 1024, 0);

        if(amountReceived>0)
        {
            buffer[amountReceived] = 0;
            printf("%s\n ", buffer);

        }


        if(amountReceived==0)
            break;
    }

    close(fd);
}

sockaddr_in* createIPv4Address(char *ip, int port) {

    struct sockaddr_in *address = static_cast<sockaddr_in *>(malloc(sizeof(struct sockaddr_in)));
    address->sin_port = htons(port);
    address->sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    return address;
}

int createTCPIpv4Socket() { return socket(AF_INET, SOCK_STREAM, 0); }
