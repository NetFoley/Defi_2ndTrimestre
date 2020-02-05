#if defined (WIN32)
    #include <winsock2.h>
    typedef int socklen_t;
#elif defined (linux)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define PORT 23


void iniTab(char * tab, int nMax);

int main(void)
{
    #if defined (WIN32)
        WSADATA WSAData;
        int erreur = WSAStartup(MAKEWORD(2,2), &WSAData);
    #else
        int erreur = 0;
    #endif

    SOCKET sock;
    SOCKADDR_IN sin;
    SOCKADDR_IN csin;
    char buffer[32];
    socklen_t recsize = sizeof(csin);
    int sock_err;

    int nbMots = 0;

    /************************************
    Lecture du fichier de mot aléatoire
    ************************************/
    FILE * pFile;

    pFile = fopen("mots.txt", "r");
    if (pFile != NULL)
    {
        char a = ' ';
        while(a != EOF)
        {
            a = fgetc(pFile);
            if(a != EOF && a == '\n')
            {
                nbMots++;
            }
        }
        fclose(pFile);
    }
    printf("Nombre de mots : %i\n", nbMots);

    /**************************
    TOUT METTRE DANS UN TAB
    **************************/
    char tabMots[25][30];
    for(int i = 0; i < 25; i++)
        iniTab(tabMots, 30);
    char nvMot[2] = {"0"};
    nvMot[1] = '\0';
    pFile = fopen("mots.txt", "r");
    if (pFile != NULL)
    {
        char a = ' ';
        int i = 0;
        while(a != EOF)
        {
            a = fgetc(pFile);
            nvMot[0] = a;
            if(a != EOF && a != '\n')
            {
                strcat(tabMots[i], nvMot);
            }
            if(a == '\n')
            {
                i++;
            }
        }
        fclose(pFile);
    }

    char * mot = malloc(sizeof(char));
    strcpy(mot, "");
    int ligne = 0;

    srand(time(0));
    int numRand = rand() % nbMots + 1;
    printf("Mot aleatoire numero %i\n", numRand);

    pFile = fopen("mots.txt", "r");
    if (pFile != NULL)
    {
        printf("\nLecture du fichier de mots\n");
        char a = ' ';
        int nbChar = 0;
        while(a != EOF && ligne < numRand)
        {
            a = fgetc(pFile);
            nvMot[0] = a;
            if(a != EOF && a != '\n')
            {
                nbChar++;
                mot = realloc(mot, nbChar*sizeof(char));
                strcat(mot, nvMot);
            }
            if(a == '\n')
            {
                ligne++;
                strcat(mot, " ");
                printf("%s(%i)(%i)\n",mot, nbChar, ligne);
                nbChar = 1;
                if(ligne < numRand)
                {
                    mot = realloc(mot, nbChar*sizeof(char));
                    strcpy(mot, "");
                }
            }
        }
        fclose(pFile);
    }
    printf("Mot choisis aleatoire : %s(%i)\n\n", mot, ligne);

    /*************************************
    Transformation du mot
    *************************************/
    char * motTrans = malloc(sizeof(mot));

    strcpy(motTrans, mot);

    int nbrTrans[26];
    for(int i = 0; i < 26; i++)
    {
        nbrTrans[i] = rand() % 25 + 1;
        printf("%i\n",nbrTrans[i]);
    }
    for(int k = 0; k < strlen(mot)-1; k++)
    {
        motTrans[k] += nbrTrans[motTrans[k] - 'a'];
        while(motTrans[k] > 'z')
            motTrans[k] -= 26;
        while(motTrans[k] < 'a')
            motTrans[k] += 26;
    }

    printf("Nouveau mot %s\n", motTrans);

    /* Si les sockets Windows fonctionnent */
    if(!erreur)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);

        /* Si la socket est valide */
        if(sock != INVALID_SOCKET)
        {
            printf("La socket %d est maintenant ouverte en mode TCP/IP\n", sock);

            /* Configuration */
            sin.sin_addr.s_addr    = htonl(INADDR_ANY);   /* Adresse IP automatique */
            sin.sin_family         = AF_INET;             /* Protocole familial (IP) */
            sin.sin_port           = htons(PORT);         /* Listage du port */
            sock_err = bind(sock, (SOCKADDR*)&sin, sizeof(sin));

            /* Si la socket fonctionne */
            if(sock_err != SOCKET_ERROR)
            {
                /* Démarrage du listage (mode server) */
                sock_err = listen(sock, 5);
                printf("Listage du port %d...\n", PORT);

                /* Si la socket fonctionne */
                if(sock_err != SOCKET_ERROR)
                {
                    /* Attente pendant laquelle le client se connecte */
                    printf("Patientez pendant que le client se connecte sur le port %d...\n", PORT);

                    sock = accept(sock, (SOCKADDR*)&csin, &recsize);
                    printf("Un client se connecte avec la socket %d de %s:%d\n", sock, inet_ntoa(csin.sin_addr), htons(csin.sin_port));

                    if(sock_err != SOCKET_ERROR)
                        printf("Chaine envoyee : %s\n", buffer);
                    else
                        printf("Erreur de transmission\n");

                    sock_err = send(sock, motTrans, 32, 0);
                    int victoire = 0;
                    while(recv(sock,buffer,32,0) != SOCKET_ERROR && strcmp(buffer, "stop") != 0)
                    {

                        if(strcmp(buffer, "power") == 0)
                        {
                            printf("\nActivation du pouvoir du joueur client !\n");
                            for(int i = 0; i < (strlen(mot)-1)/2; i++)
                            {
                                motTrans[i] = mot[i];
                            }
                        }

                        int lettreIdem = 0;
                        printf("Recu : %s\n", buffer);
                        for(int i = 0; i < strlen(mot)-1; i++)
                        {
                            if(buffer[0] == mot[i])
                            {
                                lettreIdem++;
                                motTrans[i] = mot[i];
                            }
                        }
                        if(lettreIdem < 1)
                            printf("Aucune lettre correspondante\n");
                        else
                            printf("%i lettres trouves !\n", lettreIdem);

                        fflush(stdin);
                        gets(&buffer);
                        //scanf(&buffer);
                        if(strcmp(mot, buffer) == 0)
                            victoire++;

                        else if(strcmp("power", buffer) == 0)
                        {
                            int nSyno;
                            if(numRand % 2 == 0)
                                nSyno = numRand - 2;
                            else
                                nSyno = numRand - 1;


                            printf("VOUS UTILISEZ VOTRE POUVOIR !\nSynonyme du mot : %s\n", tabMots[nSyno]);
                        }

                        if(victoire == 0)
                        {
                            sock_err = send(sock, motTrans, 32, 0);
                        }
                        else
                        {
                            strcat(buffer, "VICTORY");
                            sock_err = send(sock, buffer, 32, 0);
                        }

                        if(sock_err != SOCKET_ERROR)
                            printf("Chaine envoyee : %s\n", motTrans);
                        else
                            printf("Erreur de transmission\n");

                        strcpy(buffer, " ");

                    }

                    /* Il ne faut pas oublier de fermer la connexion (fermée dans les deux sens) */
                    shutdown(sock, 2);
                }
            }

            /* Fermeture de la socket */
            printf("Fermeture de la socket...\n");
            closesocket(sock);
            printf("Fermeture du serveur terminee\n");
        }

        #if defined (WIN32)
            WSACleanup();
        #endif
    }

    /* On attend que l'utilisateur tape sur une touche, puis on ferme */
    getchar();

        free(mot);
    return EXIT_SUCCESS;
}

void iniTab(char * tab, int nMax)
{
    for(int i = 0; i < nMax ; i++)
    {
        tab[i] = ' ';
    }
}
