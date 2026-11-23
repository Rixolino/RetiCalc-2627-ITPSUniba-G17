#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined _WIN32
#include <winsock.h>
#else
#define closesocket close
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

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

// scarta caratteri rimasti nell'input
void ClearInput()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

int main(void)
{
#if defined _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ErrorHandler("Errore in WSAStartup()\n");
        return -1;
    }
#endif

    // Passo 2: legge IP e porta da tastier
    char address[16];
    int port;
    printf("Inserire l'indirizzo IP del server: ");
    //scanf("%15s", address);

    if (scanf("%15s", address) != 1)
    {
        ErrorHandler("IP non valido\n");
        ClearWinSock();
        return -1;
    }
    printf("Porta: ");
    //scanf("%d", &port);
    if (scanf("%d", &port) != 1 || port < 1 || port > 65535)
    {
        ErrorHandler("Numero di porta non valido\n");
        ClearWinSock();
        return -1;
    }

    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_port = htons(port);
    sad.sin_addr.s_addr = inet_addr(address);
    if (sad.sin_addr.s_addr == INADDR_NONE)
    {
        ErrorHandler("Indirizzo IP non valido\n");
        ClearWinSock();
        return -1;
    }

    int clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket < 0)
    {
        ErrorHandler("Creazione socket fallita\n");
        closesocket(clientSocket);
        ClearWinSock();
        return -1;
    }

    // Passo 3: richiesta connessione
    if (connect(clientSocket, (struct sockaddr *)&sad, sizeof(sad)) < 0)
    {
        ErrorHandler("Connessione al server fallita\n");
        closesocket(clientSocket);
        ClearWinSock();
        return -1;
    }

    printf("Connessione avvenuta con successo\n");

    // Passo 6: ricezione e stampa
    char buf[BUFFERSIZE];
    int bytesRcvd = recv(clientSocket, buf, BUFFERSIZE - 1, 0);
    if (bytesRcvd <= 0)
    {
        ErrorHandler("Ricezione fallita o connessione chiusa dal server\n");
        closesocket(clientSocket);
        ClearWinSock();
        return -1;
    }
    buf[bytesRcvd] = '\0';
    printf("Server: %s\n", buf);

    // Passo 7: lettura interi ed invio al server
    int n1, n2, n3;
    int res;
    // lettura
    while (1)
    {
        printf("Inserire tre numeri interi: ");
        // scanf("%d", &n1);
        // scanf("%d", &n2);
        // scanf("%d", &n3);
        if (scanf("%d %d %d", &n1, &n2, &n3) != 3)
        {
            printf("Input non valido, riprova\n");
            ClearInput();
            continue;
        }
        // invio
        char msg[BUFFERSIZE];
        snprintf(msg, sizeof(msg), "%d %d %d", n1, n2, n3);
        int msgLen = (int)strlen(msg);
        if (send(clientSocket, msg, msgLen, 0) != msgLen)
        {
            ErrorHandler("Invio fallito\n");
            closesocket(clientSocket);
            ClearWinSock();
            return -1;
        }
        // Punto 10: ricezione risposta e stampa
        bytesRcvd = recv(clientSocket, buf, BUFFERSIZE - 1, 0);
        if (bytesRcvd <= 0)
        {
            ErrorHandler("Ricezione fallita o connessione chiusa dal server\n");
            closesocket(clientSocket);
            ClearWinSock();
            return -1;
        }
        buf[bytesRcvd] = '\0';
        printf("Server: %s\n", buf);

        if (strcmp(buf, "QUIT") == 0)
        {
            break;
        }
        printf("Risultato: %s\n", buf);
    }

    closesocket(clientSocket);
    ClearWinSock();
    return 0;
}
