#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <csignal>


int createTCPIpv4Socket();
sockaddr_in* createIPv4Address(char *ip, int port);
struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD);
void acceptNewConnectionAndReceiveAndPrintItsData(int serverSocketFD);
void receiveAndPrintIncomingData(int socketFD);

void startAcceptingIncomingConnections(int serverSocketFD);

void receiveAndPrintIncomingDataOnSeparateThread(AcceptedSocket *pSocket);

void sendReceivedMessageToTheOtherClients(char *buffer, int socketFD);

struct AcceptedSocket{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

int main() {
    int serverSocketFD = createTCPIpv4Socket();
    struct sockaddr_in *serverAddress = createIPv4Address("", 2000);

    int result = bind(serverSocketFD, reinterpret_cast<const sockaddr *>(serverAddress), sizeof(*serverAddress));
    if (result == 0)
        printf("Socket was bound successfully\n");

    int listenResult = listen(serverSocketFD, 10);

    startAcceptingIncomingConnections(serverSocketFD);

    shutdown(serverSocketFD, SHUT_RDWR);

    return 0;
}

struct AcceptedSocket acceptedSockets[10];
int acceptedSocketsCount = 0;


void startAcceptingIncomingConnections(int serverSocketFD) {

    while (true)
    {
        struct AcceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD);
        acceptedSockets[acceptedSocketsCount++] = *clientSocket;

        receiveAndPrintIncomingDataOnSeparateThread(clientSocket);

    }

}

void receiveAndPrintIncomingDataOnSeparateThread(struct AcceptedSocket *pSocket) {

    pthread_t id;
    pthread_create(&id, NULL, reinterpret_cast<void *(*)(void *)>(receiveAndPrintIncomingData),
                   reinterpret_cast<void *>(pSocket->acceptedSocketFD));

}
void receiveAndPrintIncomingData(int socketFD) {
    char buffer[1024];

    while(true){

        ssize_t amountReceived = recv(socketFD, buffer, 1024, 0);

        if(amountReceived>0)
        {
            buffer[amountReceived] = 0;
            printf("%s\n ", buffer);

            sendReceivedMessageToTheOtherClients(buffer, socketFD);
        }


        if(amountReceived==0)
            break;
    }

    close(socketFD);
}

void sendReceivedMessageToTheOtherClients(char *buffer, int socketFD) {
    for(int i = 0; i<acceptedSocketsCount; i++)
        if(acceptedSockets[i].acceptedSocketFD != socketFD){
            send(acceptedSockets[i].acceptedSocketFD, buffer, strlen(buffer),0);
        }
}

struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in  clientAddress;
    int clientAddressSize = sizeof (struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, reinterpret_cast<sockaddr *>(&clientAddress),
                              reinterpret_cast<socklen_t *>(&clientAddressSize));
    struct AcceptedSocket* acceptedSocket = static_cast<AcceptedSocket *>(malloc(sizeof(struct AcceptedSocket)));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD>0;

    if(!acceptedSocket->acceptedSuccessfully)
        acceptedSocket->error = clientSocketFD;

    return acceptedSocket;
}
int createTCPIpv4Socket() { return socket(AF_INET, SOCK_STREAM, 0); }
sockaddr_in* createIPv4Address(char *ip, int port) {

    struct sockaddr_in *address = static_cast<sockaddr_in *>(malloc(sizeof(struct sockaddr_in)));
    address->sin_port = htons(port);
    address->sin_family = AF_INET;

    if(strlen(ip) ==0)
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    return address;
}

