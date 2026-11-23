#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined _WIN32
#include <winsock.h>
typedef int socklen;
#else
#define closesocket close
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef socklen_t socklen;
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define DEFAULT_PORT 5004
#define QLEN 6
#define BUFFERSIZE 512

void ErrorHandler(const char *errorMessage)
{
    printf("%s", errorMessage);
}

void ClearWinSock()
{
#if defined _WIN32
    WSACleanup();
#endif
}

int main(int argc, char *argv[])
{
    // passo 1
    int port = DEFAULT_PORT;
    if (argc > 1)
    {
        port = atoi(argv[1]);
        if (port < 1 || port > 65535)
        {
            printf("Numero di porta non valido: %s\n", argv[1]);
            return -1;
        }
    }

#if defined _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ErrorHandler("Errore in WSAStartup()\n");
        return -1;
    }
#endif

    // Creazione socket
    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0)
    {
        ErrorHandler("Creazione socket fallita\n");
        closesocket(serverSocket);
        ClearWinSock();
        return -1;
    }

    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = htonl(INADDR_ANY);
    sad.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&sad, sizeof(sad)) < 0)
    {
        ErrorHandler("bind() fallita\n");
        closesocket(serverSocket);
        ClearWinSock();
        return -1;
    }

    if (listen(serverSocket, QLEN) < 0)
    {
        ErrorHandler("listen() fallita\n");
        closesocket(serverSocket);
        ClearWinSock();
        return -1;
    }

    printf("Server in ascolto sulla porta %d\n", port);

    struct sockaddr_in cad;
    while (1)
    {

        socklen clientLen = sizeof(cad);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&cad, &clientLen);
        if (clientSocket < 0)
        {
            ErrorHandler("accept() fallita\n");
            continue; // il server resta in ascolto
        }

        //printf("Client connesso: %s\n", inet_ntoa(cad.sin_addr));
        //closesocket(clientSocket);

        // Passo 4
        printf("Connesso con il client il cui nome IP: %s\n", inet_ntoa(cad.sin_addr));
        // Passo 5
        const char *welcome = "connessione avvenuta";
        int welcomeLen = (int)strlen(welcome);
        if (send(clientSocket, welcome, welcomeLen, MSG_NOSIGNAL) != welcomeLen)
        {
            ErrorHandler("Errore durante l'invio del messaggio di benvenuto\n");
            closesocket(clientSocket);
            continue;
        }
        // Passo 7: ricezione
        char buf[BUFFERSIZE];
        while (1)
        {
            int bytesRcvd = recv(clientSocket, buf, BUFFERSIZE - 1, 0);
            if (bytesRcvd <= 0)
            {
                printf("Client disconnesso\n");
                break;
            }
            buf[bytesRcvd] = '\0';

            int n1, n2, n3;
            //int res;
            if (sscanf(buf, "%d %d %d", &n1, &n2, &n3) != 3) {
                ErrorHandler("Il messaggio ricevuto non è valido\n");
                break;
            }
            //Passo 8: calcolo e stampa
            int res = n1 + n2 - n3;
            printf("La somma di %d + %d - %d = %d\n", n1, n2, n3, res);
            //Passo 9
            char reply[BUFFERSIZE];
            if (res == 100) {
                strcpy(reply, "QUIT");
            } else {
                snprintf(reply, sizeof(reply), "%d", res);
            }
            int replyLen = (int)strlen(reply);
            if (send(clientSocket, reply, replyLen, MSG_NOSIGNAL) != replyLen) {
                ErrorHandler("Invio della risposta fallita\n");
                break;
            }

            if (res == 100) {
                break; // termina conversazione con client
            }
        }
        closesocket(clientSocket); //chiude connessione con il client
    }
}