#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons() and inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>

#include "common.h"
/*CHE DEVE FARE IL MIO CLIENT?
  1.Stabilire una connessione col server e inviare username e password.
    Se fallisce il login: chiudo la sessione
    Se fallisce la registrazione, chiedo un altro user_name
  2.Inviare  messaggi
  3.Ricevere ed archiviare i messaggi
  4.Inviare comandi per la lettura          
  5. la cancellazione dei messaggi 

*/
void logged_client(int socket_desc){////////////////////come tenere il conto dell'id del messaggio
    char * option=(char *) malloc(sizeof(char));
    int ret;
    int option_len=sizeof(char);
    while(1){
    fprintf(stderr, "Se vuoi scrivere un nuovo messaggio premi N, se vuoi leggere un vecchio messaggio premi L, se vuoi cancellarne uno premi C, se vuoi uscire premi E");
    scanf("%s",option);
    if (!memcmp(option, "E", option_len)) break;//L'utente vuole uscire
    while ( (ret = send(socket_desc, option, option_len, 0)) < 0) {//Mando l'opzione al server
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    
    if (DEBUG) fprintf(stderr, "sent: %s \n", option);
    switch (option){
        case "N"://voglio scrivere un nuovo messaggio
        char nome[32];
        char oggetto[64];
        char messaggio[1000];
        fprintf(stderr, "A chi vuoi inviare il tuo messaggio?");//chiedo il nome
        scanf("%s",nome);
        while ( (ret = send(socket_desc, nome, sizeof(nome), 0)) < 0) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
        }    
        if (DEBUG) fprintf(stderr, "sent: %s \n", nome);
        fprintf(stderr, "Quale è l'oggetto del tuo messaggio?");//chiedo l'oggetto
        scanf("%s",oggetto);
        while ( (ret = send(socket_desc, oggetto, sizeof(oggetto), 0)) < 0) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
        
        }
    
    if (DEBUG) fprintf(stderr, "sent: %s \n", oggetto);   
    fprintf(stderr, "Scrivi il tuo messaggio qui:"); //chiedo il messaggio
        scanf("%s",messaggio); 
      while ( (ret = send(socket_desc, messaggio, sizeof(messaggio), 0)) < 0) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
        }
    
    if (DEBUG) fprintf(stderr, "sent: %s \n", messaggio);            
        break;
       /* case "L":
        break;
        case "C":
        break;*/
        }
    return
    
    }
    
    
    
    
    
    
    
    
    
    
    
    }
int main(int argc, char* argv[]) {
    
    int ret;
    char* old_or_new=(char *)malloc(sizeof(char));
    size_t allowed_command_len_min = 5*sizeof(char);
    size_t allowed_command_len_max = 32*sizeof(char);
    // variables for handling a socket
    int socket_desc;
    struct sockaddr_in server_addr = {0}; // some fields are required to be filled with 0

    // create a socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_HELPER(socket_desc, "Could not create socket");

    // set up parameters for the connection
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!
//FASE 1: Log_In e Registrazione 
    // initiate a connection on the socket
    ret = connect(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    ERROR_HELPER(ret, "Could not create connection");

    if (DEBUG) {//CHICK CHECK THIS
    fprintf(stderr, "Connection established!\n");    
    printf("Benvenuto! Se sei un nuovo utente premi '0', '1' altrimenti. \n");
    scanf ("%s",old_or_new);

    }
    //------------------AVVERTO SERVER SE REGISTRAZIONE O LOG IN------------------//
     while ( (ret = send(socket_desc, old_or_new, sizeof(char), 0)) < 0) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    
    if (DEBUG) fprintf(stderr, "sent: %s \n", old_or_new);
    
    char* recv_buf=(char *) malloc(sizeof(char));
    size_t recv_buf_len = sizeof(char);
    int recv_bytes;

    /////////////HANDLE USERNAME AND PASSWORD////////////
    char* usr_name=(char*)malloc(32*sizeof(char));
    char* usr_pwd=(char*)malloc(32*sizeof(char));
    size_t usr_name_len = 32*sizeof(char);
    size_t usr_pwd_len = 32*sizeof(char);
    printf("Inserisci uno user name (minimo 8 massimo 32 caratteri)\n"); scanf("%s",usr_name); //---->INSERISCI CONTROLLI 
    char * PATH=(char*)malloc(strlen(BASE_PATH)+strlen(usr_name));
    strcpy(PATH,BASE_PATH);
    while ( (ret = send(socket_desc, usr_name, usr_name_len, 0)) < 0) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    
    if (DEBUG) fprintf(stderr, "sent: %s \n", usr_name);
   printf("Inserisci una password (minimo 8 massimo 32 caratteri)\n"); scanf("%s",usr_pwd);   //---->INSERISCI CONTROLLI
   while ( (ret = send(socket_desc, usr_pwd, usr_pwd_len, 0)) < 0) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    
    if (DEBUG) fprintf(stderr, "sent: %s \n", usr_pwd);



        // risposta sul nome---> dice se per lo user può fare il login se è disponibile per chi si registra
    while ( (recv_bytes = recv(socket_desc, recv_buf, recv_buf_len, 0)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }

    if (DEBUG){ 
        if(!memcmp(old_or_new,"0",sizeof(char))){ //è un utente che vuole registrarsi 
            if(!memcmp(recv_buf,"0",sizeof(char))){
                fprintf(stderr,"Siamo spiacenti ma lo username da lei scelto è già in uso da un altro utente. Arrivederci\n");}
            else {
                fprintf(stderr,"Benvenuto %s",usr_name);
                logged_client(socket_desc);
                }
            }
        else{
            if(!memcmp(recv_buf,"0",sizeof(char))){
                fprintf(stderr,"sono spiacente ma lo Username o la password inserite non sono presenti nel sitema. Arrivederci\n");
                }
            else{ 
                fprintf(stderr,"Bentornato %s",usr_name);
                logged_client(socket_desc);
                }
            }
            }
    recv_buf[recv_bytes] = '\0'; // add string terminator manually!*/

    

    // close the socket
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket");

    printf("Answer from server: %s", recv_buf);

    if (DEBUG) fprintf(stderr, "Exiting...\n");

    exit(EXIT_SUCCESS);


}
