#include <errno.h> //per leggere il valore di errno
#include <stdio.h> // libreria di base
#include <stdlib.h> // libreria permalloc etc..
#include <string.h> // libreria per le stringhe
#include <time.h> //per giorno e data correnti
#include <fcntl.h> //gestione files
#include <unistd.h>
#include <arpa/inet.h>  // htons()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>

#include "common.h"
/*CHE DEVE FARE IL MIO SERVER?
  1.Stabilire una connessione col client e Log_in e registrazione
  2.Inviare messaggi-----> un messaggio deve contenere almeno i campi Destinatario, Oggetto, Testo
  3.Ricevere Messaggi e Archiviarli
  4.Lettura messaggi archiviati
  5.Cancellare messaggi in archivio
*/
//funzione per cercare lo user in un file
char* cercaFile(FILE * file,user_t user){
        char* letto=malloc(32*sizeof(char));
        char* pass=malloc(32*sizeof(char));
        int usrname_len= strlen(user.name);
        int pass_len= strlen(user.password);
        fgets(letto,32*sizeof(char), file);
	    while(!feof(file)) {
            if(memcmp(letto,user.name, strlen(letto)-1)==0){
		        fgets(pass,32*sizeof(char), file);
                if(memcmp(pass,user.password, strlen(pass)-1)==0){                   

			 //Trova lo username e corrisponde alla password 
                         return "1";
                }
                else {
                         //Trova lo username ma la password è sbagliata
                         
                         return "0";
                }
            }
            
           fgets(pass,32*sizeof(char),file);
           fgets(letto,32*sizeof(char), file);   

         } free(letto); free(pass);
    //Non trova lo username   
    return "0";
}

//1---stabilire una connessione
void connection_handler(int socket_desc) {    
      
    int ret;
    //char* allowed_command = SERVER_COMMAND;//COMMON.H
    size_t allowed_command_len_min = 5*sizeof(char);
    size_t allowed_command_len_max = 32*sizeof(char);
    char* send_buf=(char*) malloc(2*sizeof(char));
    int buf_len=256*sizeof(char);
    // receive command from client
    char* recv_buf=(char*) malloc(256*sizeof(char));
    size_t recv_buf_len = 32*sizeof(char);
    int recv_bytes;
    size_t server_message_len = 256*sizeof(char);
 
    //FASE 1: Log_In e Registrazione 
    user_t* user=(user_t *) malloc(sizeof(user_t));
    user->name= (char*) malloc(32*sizeof(char));
    user->password= (char*) malloc(32*sizeof(char));
   /* //check this*/
    while ( (recv_bytes = recv(socket_desc, user->name, 32*sizeof(char), 0)) < 0 ) {
        
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    if (DEBUG) fprintf(stderr, "username received: %s \n", user->name);//check_this
    
    while ( (recv_bytes = recv(socket_desc, user->password, 32*sizeof(char), 0)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    
    if (DEBUG) fprintf(stderr, "username received: %s \n", user->password);//check_this
    
    FILE * file=fopen("user_data","r");
    if(file==NULL){
	 fprintf(stderr,"impossibile aprire il file!\n");
         exit(1);
	}
   
   char * ceono=cercaFile(file,*user);
   fprintf(stderr, "%s", ceono);
   char * opzione=(char *)malloc(sizeof(char));
   int opzione_len=sizeof(char);
   
    while ( (ret = send(socket_desc,ceono , 1, 0)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to the socket");
    }
    fprintf(stderr, "sent: %s \n", ceono);
    if (DEBUG) fprintf(stderr, "Message of %d bytes sent\n", ret);
    
    
     
    while ( (ret = send(socket_desc,ceono , 1, 0)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to the socket");
    }
    fprintf(stderr, "sent: %s \n", ceono);
    if (DEBUG) fprintf(stderr, "Message of %d bytes sent\n", ret);
    
    //Parte che legge l'opzione
    messaggio_t* messaggio;
     while ( (recv_bytes = recv(socket_desc, opzione, opzione_len, 0)) < 0 ) {
        
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    if (DEBUG) fprintf(stderr, "opzione ricevuta: %s \n", opzione);
     switch (option){
        
        case "N":
            while ( (recv_bytes = recv(socket_desc, opzione, opzione_len, 0)) < 0 ) {
        
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    if (DEBUG) fprintf(stderr, "opzione ricevuta: %s \n", opzione);
    while ( (recv_bytes = recv(socket_desc, opzione, opzione_len, 0)) < 0 ) {
        
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    if (DEBUG) fprintf(stderr, "opzione ricevuta: %s \n", opzione);
    while ( (recv_bytes = recv(socket_desc, opzione, opzione_len, 0)) < 0 ) {
        
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    if (DEBUG) fprintf(stderr, "opzione ricevuta: %s \n", opzione);
        break;
        case "L":
        break;
        case "C":
        break;
}
    // close socket
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket for incoming connection");
    free(send_buf);
    free(recv_buf);
}





// --------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    int ret;

    int socket_desc, client_desc;

    // some fields are required to be filled with 0
    struct sockaddr_in server_addr = {0}, client_addr = {0};

    int sockaddr_len = sizeof(struct sockaddr_in); // we will reuse it for accept() and bind()

    // initializzo la socket in ascolto
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);// 0 utilizzo il protocollo di deafult
    ERROR_HELPER(socket_desc, "Could not create socket");

    server_addr.sin_addr.s_addr = INADDR_ANY; // we want to accept connections from any interface
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!

    /* We enable SO_REUSEADDR to quickly restart our server after a crash:
     * for more details, read about the TIME_WAIT state in the TCP protocol */
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    ERROR_HELPER(ret, "Cannot set SO_REUSEADDR option");

    // bind address to socket Dalle slides:"invoca l'assegnazione di un indirizzo al socket"
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sockaddr_len);
    ERROR_HELPER(ret, "Cannot bind address to socket");

    // start listening
    ret = listen(socket_desc, MAX_CONN_QUEUE); // max number of connections the server can queue --- common.h
    ERROR_HELPER(ret, "Cannot listen on socket");

    // loop to handle incoming connections serially
    while (1) {
        client_desc = accept(socket_desc, (struct sockaddr*) &client_addr, (socklen_t*) &sockaddr_len);
        ERROR_HELPER(client_desc, "Cannot open socket for incoming connection");

        if (DEBUG) fprintf(stderr, "Incoming connection accepted...\n");

        connection_handler(client_desc);

        if (DEBUG) fprintf(stderr, "Done!\n");
    }

    exit(EXIT_SUCCESS); // this will never be executed

}
