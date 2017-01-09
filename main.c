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
//funzione per spedire (gestisce invii parziali)
int spedisci(char*buf, int dim, int socket_desc){	
    int spediti;
    int recv_bytes=0;
    while (recv_bytes<dim) {
    spediti = send(socket_desc, buf+ recv_bytes, dim - recv_bytes, 0);
    if (spediti < 0 && errno == EINTR) continue;
    if (spediti < 0) return -1;//error: return -1
    recv_bytes += spediti;
    
    
    //if (recv_bytes == 0)break;
}
 return spediti;   
}
//funzione per ricevere (gestisce invii parziali)
int ricevi(char *buf, int dim, int socket_desc){
    int recv_bytes=0;
    int letti=0;
    int flag=1;
    while (flag) {
    letti = recv(socket_desc, buf + recv_bytes, dim - recv_bytes, 0);
    if (letti < 0 && errno == EINTR) continue;
    if (letti < 0) return -1;//error: return -1
    recv_bytes += letti;
    if (recv_bytes > 0 && (buf[recv_bytes - 1] == '\0' || recv_bytes>32*sizeof(char))) {
    flag = 0;
    }
    if (recv_bytes == 0)break;
    }
    return recv_bytes;
    }
//Funzione che si occupa degli utenti loggati
int logged_server(int socket_desc, char * PATH){
       //-----------Cosa vuole fare il mio client? Scrivere Leggere o Cancellare-----------//
    
   
   char * opzione=(char *)malloc(2*sizeof(char));
   int opzione_len=sizeof(char);
   
        //Parte che legge l'opzione
    messaggio_t* messaggio=(messaggio_t*) malloc(sizeof(messaggio_t));
    messaggio->destinatario=(char*)malloc(32*sizeof(char));
    messaggio->oggetto=(char*)malloc(64*sizeof(char));
    messaggio->testo=(char*)malloc(256*sizeof(char));
    ricevi(opzione, opzione_len+1, socket_desc);

         //while(!memcmp(opzione[letti-1],"\0")){letti=(read(socket_desc, opzione, opzione_len))}
     
   
    if (DEBUG) fprintf(stderr, "opzione ricevuta: %s\n", opzione);
    int opzioneint=atoi(opzione);
    int ret;
     switch (opzioneint){
        
        case 1:    
    
    //----------------RICEVO IL DESTINATARIO-----------------//
    
    ret=ricevi(messaggio->destinatario, DESTINATARIO_LEN, socket_desc);
    user_t destinatario;
    destinatario.name=messaggio->destinatario;
    FILE* utenti=fopen("user_data","r");
    if(!cercaFile(utenti,destinatario)) {fprintf(stderr,"something gone wrong");return-1;}
    if (DEBUG) fprintf(stderr, "destinatario: %s %d\n", messaggio->destinatario,ret);
    
    
    //------------------RICEVO L'OGGETTO--------------------//
    
    ret=ricevi(messaggio->oggetto, OGGETTO_LEN, socket_desc);
   // messaggio->oggetto[ret]='\0';
    if (DEBUG) fprintf(stderr, "oggetto: %s\n", messaggio->oggetto);

        
        
     //------------------RICEVO IL TESTO--------------------//       
    ret=ricevi(messaggio->testo, TESTO_LEN+1,socket_desc);
    if (ret==-1) return -1;
    if (DEBUG) fprintf(stderr, "messaggio: %s\n", messaggio->testo);
    
    
    //-----------ARCHIVIO IL MESSAGGIO--------------------//
    //---per creare l'id del ricevente leggo il contatore 
    char * PATH_DESTINATARIO=(char*)malloc((strlen(BASE_PATH)+strlen(messaggio->destinatario))*sizeof(char));
    char * PATH_CONTATORE=(char *) malloc((strlen(BASE_PATH)+strlen(messaggio->destinatario)+strlen("/contatore"))*sizeof(char));
    char* contatore=(char*) malloc(sizeof(char));
    strcpy(PATH_DESTINATARIO,BASE_PATH);
    //strcat(PATH_DESTINATARIO,"/");
    strcat(PATH_DESTINATARIO,messaggio->destinatario);
    strcpy(PATH_CONTATORE,PATH_DESTINATARIO);
    strcat(PATH_CONTATORE,"/contatore");
    FILE * contadest= fopen(PATH_CONTATORE,"r");//tengo un contatore dei messagi ricevuti, per dare l'id al nome
    if(contadest==NULL) return -1;            //e sarà anche il nome del messaggio
    fscanf(contadest,"%s",contatore); 
    fclose(contadest);
    messaggio->ID=atoi(contatore)+1;  
    fprintf(stderr,"contatore: %s\n",contatore);
    contadest= fopen(PATH_CONTATORE,"w");//tengo un contatore dei messagi ricevuti, per dare l'id al nome
    if(contadest==NULL) return -1;  
    fprintf(contadest,"%d",messaggio->ID);          //e sarà anche il nome del messaggio
    fclose(contadest);
    snprintf (contatore, sizeof(contatore), "/%d", messaggio->ID);
    fprintf(stderr,"contatore: %s\n",contatore);
    strcat(PATH_DESTINATARIO,contatore);
    fprintf(stderr,"%s\n",PATH_DESTINATARIO);
    FILE * messfile= fopen(PATH_DESTINATARIO,"w");//salvo un messaggio in un file diverso.
    if (messfile==NULL) return -1;
    fprintf(messfile,"%s\n",messaggio->destinatario);
    fprintf(messfile,"%s\n",messaggio->oggetto);
    fprintf(messfile,"%s\n",messaggio->testo);
    fclose(messfile);
    break;
        /*
        case "L":
        break;
        case "C":
        break;*/
}
  return 0  ;
    
    
    }
int cercaFile(FILE * file,user_t user){
	char* letto=(char *)malloc(32*sizeof(char));
	char* pass=(char *)malloc(32*sizeof(char));
	int usrname_len= strlen(user.name);
	int pass_len= strlen(user.password);
	fgets(letto,32*sizeof(char), file);
	fprintf(stderr,"letto:%s len:%d\n",letto,(int)strlen(letto));
	fprintf(stderr,"username:%s len:%d\n",user.name,(int)strlen(user.name));
	while(!feof(file)) {
		if(!memcmp(letto, user.name, strlen(letto)-1)){
			printf("trovato username\n");
			fprintf(stderr,"1 letto:%s usrnm:%s risultato:%d\n",letto,user.name,memcmp(letto,user.name, strlen(letto)-1));
			fgets(pass,32*sizeof(char), file);
			if(!memcmp(pass,user.password, strlen(pass)-1)){                   
				//Trova lo username e corrisponde alla password 
				printf("trovata password\n");
				return 1;                         
			}
			else {
				//Trova lo username ma la password è sbagliata
				printf("errata password\n");     
				return 2;
			}
		}
		fgets(pass,32*sizeof(char),file); 
		fprintf(stderr,"%s\n",pass);
		fgets(letto,32*sizeof(char), file);
		fprintf(stderr,"%s\n",letto);
		fprintf(stderr,"2 letto:%s| len:%d\n",letto,(int)strlen(letto));
		fprintf(stderr,"2 username:%s| len:%d\n",user.name,(int)strlen(user.name));
    } 
    free(letto); free(pass);
    //Non trova lo username   
    printf("non c'è l'username\n");
    return 0;
}

//1---stabilire una connessione
void connection_handler(int socket_desc) {
    int ret;
    char PATH [0];
    //strcpy(PATH,BASE_PATH);
    char* allowed_command = SERVER_COMMAND;//COMMON.H
    size_t allowed_command_len_min = 5*sizeof(char);//------> RICORDA DI AGGIUNGERE I CONTROLLI!!!!!!!
    size_t allowed_command_len_max = 32*sizeof(char);

    int buf_len=256*sizeof(char);
    // receive command from client
    char* recv_buf=(char*) malloc(256*sizeof(char));
    size_t recv_buf_len = 32*sizeof(char);
    int recv_bytes;
    size_t server_message_len = 256*sizeof(char);
 
    //FASE 1: Log_In e Registrazione 
    //Dati riguardanti lo user: alloco lo spazio e metto a 0 il contatore di default
    user_t* user=(user_t*)malloc(sizeof(user_t));
    user->name= (char*) malloc(32*sizeof(char));
    user->password= (char*) malloc(32*sizeof(char));
    user->msg_cont=0;
    char * old_or_new= (char *) malloc(sizeof(char));
    //-------RICEVO L'INFORMAZIONE: Se Registrazione o Log IN---------------//
    while ( (recv_bytes = recv(socket_desc, old_or_new, sizeof(old_or_new), 0)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    if (DEBUG) fprintf(stderr, "log 1 or new 0: %s \n", old_or_new);//check_this      
    
    
    //-------RICEVO USERNAME E PASSWORD-----------//
    while ( (recv_bytes = recv(socket_desc, user->name, 32*sizeof(char), 0)) < 0 ) {        
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    if (DEBUG) fprintf(stderr, "username received: %s \n", user->name);//check_this
    
    while ( (recv_bytes = recv(socket_desc, user->password, 32*sizeof(char), 0)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    
    if (DEBUG) fprintf(stderr, "password received: %s \n", user->password);//check_this
    
    
    FILE * file=fopen("user_data","r");
    if(file==NULL){
		fprintf(stderr,"impossibile aprire il file!\n");
		exit(1);
	}
   
   int ceono=cercaFile(file,*user);
   
   fprintf(stderr, "C'è o no: %d\n", ceono);
   fclose(file);
   char *logged=(char*) malloc(sizeof(char));//serve per avvisare il client se può loggarsi o no
   if(!memcmp(old_or_new,"0", sizeof(char))){ //è un utente che vuole registrarsi
		printf("registrazione\n");
		if(!ceono){ //lo username è disponibile
			printf("username disponibile\n");
			FILE * file=fopen("user_data","a");//apro il modalità append aggiungo nome e pwd
			if(file==NULL){
				fprintf(stderr,"impossibile aprire il file!\n");
				exit(1);
			}
			fprintf(file,"%s\n%s\n",user->name,user->password);
			fclose(file);
			printf("scrittura sul file fatta\n");
			//strcat(PATH,user->name);
			mkdir(PATH);
			//strcat(PATH,"/contatore.txt");
			strcat(PATH,"./");
			strcat(PATH,user->name);
			strcat(PATH,"/contatore.txt");
			fprintf(stderr,"\nthis is PATH |%s|\n",PATH);
			FILE * contatore=fopen(PATH,"w");
				
			if(contatore==NULL){
				fprintf(stderr,"impossibile aprire il file!\n");
				printf( "Error code opening file: %d\n", errno );
				printf( "Error opening file: %s\n", strerror( errno ) );
				exit(1);
			} 
			printf("contatore aperto con successo \n");  
			fclose(contatore);

			logged="1";
			printf("contatore fatto\n");
			while ( (ret = send(socket_desc,logged, 1, 0)) < 0 ) {
				if (errno == EINTR) continue;
				ERROR_HELPER(-1, "Cannot write to the socket");
				logged_server(socket_desc, PATH);
			}
			printf("chiamo logged_server\n");
			logged_server(socket_desc, PATH);
			printf("finito logged_server\n");
		}
		else{ 
			printf("username non disponibile\n");
			logged="0"; 
			while ( (ret = send(socket_desc,logged, 1, 0)) < 0 ) {
				if (errno == EINTR) continue;
				ERROR_HELPER(-1, "Cannot write to the socket");
				logged_server(socket_desc, PATH);}
			}
		fprintf(stderr, "sent: %s \n", logged);
	}
    
        //lo username è già stato usato
    else{//è un utente che vuole loggarsi
		printf("effettuo login\n");
        if(ceono==1){ 
			printf("username presente\n");
			logged="1";
			while ( (ret = send(socket_desc,logged, 1, 0)) < 0 ) {
				if (errno == EINTR) continue;
				ERROR_HELPER(-1, "Cannot write to the socket");
			}
			fprintf(stderr, "sent: %s \n", logged);
    
			logged_server(socket_desc,PATH);
		}
		else{ 
			printf("username assente\n");
			logged="0";
			while ( (ret = send(socket_desc,logged, 1, 0)) < 0 ) {
				if (errno == EINTR) continue;
				ERROR_HELPER(-1, "Cannot write to the socket");
			}
			fprintf(stderr, "sent: %s \n", logged);}
	}
    free(old_or_new);
    
    //---------Informo il client del mio risultato;-----------------------//
    


    


    //libero la memoria allocata
    printf("libero memoria\n");
    free(user->name);
    free(user->password);
    free(user);
    // close socket
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket for incoming connection");

    free(recv_buf);
    printf("fine libero memoria\n");
}





// --------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
	printf("sono il main\n");
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
