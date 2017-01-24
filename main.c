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
int logged_server(int socket_desc, user_t* loggeduser){
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
    char ID[5]={0};
        char* PATH_LETTO=(char *)malloc((strlen(BASE_PATH)+ MITTENTE_LEN)*sizeof(char));
     switch (opzioneint){
        
    case 1:    
    
    //----------------RICEVO IL DESTINATARIO-----------------//
    
    ret=ricevi(messaggio->destinatario, DESTINATARIO_LEN, socket_desc);
    user_t* destinatario=(user_t *)malloc(sizeof(user_t));
    destinatario->name = messaggio->destinatario;
    fprintf(stderr, "1destinatario: %s %d\n", destinatario->name,(int)strlen(messaggio->destinatario));
    destinatario->name[strlen(destinatario->name)-1]='\0';

    FILE* utenti=fopen("user_data","r");
    if(!cercaFile(utenti,*destinatario)) {
        fprintf(stderr,"something gone wrong");
        return-1;
    }
    fprintf(stderr, "destinatario: %s %d\n", messaggio->destinatario,ret);
    
    fclose(utenti);
    //------------------RICEVO L'OGGETTO--------------------//
    
    ret=ricevi(messaggio->oggetto, OGGETTO_LEN, socket_desc);
   // messaggio->oggetto[ret]='\0';
    if (DEBUG) fprintf(stderr, "oggetto: %s\n", messaggio->oggetto);
    
        
        
     //------------------RICEVO IL TESTO--------------------//       
    ret=ricevi(messaggio->testo, TESTO_LEN+1,socket_desc);
    if (ret==-1) return;
    if (DEBUG) fprintf(stderr, "messaggio: %s\n", messaggio->testo);
    
    //-----------ARCHIVIO IL MESSAGGIO--------------------//
    //---per creare l'id del ricevente leggo il contatore 
    unsigned int  contatore;
    char * PATH_DESTINATARIO=(char*)malloc((strlen(BASE_PATH)+strlen(messaggio->destinatario))*sizeof(char));

    strcpy(PATH_DESTINATARIO,BASE_PATH);
    //strcat(PATH_DESTINATARIO,"/");
    strcat(PATH_DESTINATARIO,messaggio->destinatario);
    FILE * messfile= fopen(PATH_DESTINATARIO,"r+");//leggo il contatore
    
    fscanf(messfile,"%u",&contatore);
    fprintf(stderr,"contatore:%u",contatore);
    contatore++;
    rewind(messfile);
    fprintf(messfile,"%u",contatore);
    fprintf(stderr,"contatore:%u",contatore);
    fseek(messfile,0,SEEK_END);
    //FILE * messfile= fopen(PATH_DESTINATARIO,"a");//salvo un messaggio in un file diverso.
    if (messfile==NULL) return -1;
    fprintf(messfile,"%u\n",contatore);
    fprintf(messfile,"%s\n",loggeduser->name);
    fprintf(messfile,"%s",messaggio->oggetto);
    fprintf(messfile,"%s",messaggio->testo);
    fclose(messfile);
    free(destinatario);
    break;
    //-------------L'utente vuole LEGGERE il messaggio-------------//
    case 2:
    
    ret=ricevi(ID,6*sizeof(char),socket_desc);
    if(ret<0) fprintf(stderr,"errore nella ricezione\n");
    fprintf(stderr,"ID: %s\n",ID);

    strcpy(PATH_LETTO,BASE_PATH);
    strcat(PATH_LETTO,loggeduser->name);
    fprintf(stderr,"PATH: %s\n",PATH_LETTO);
    FILE * filemessaggio= fopen(PATH_LETTO,"r");
    if(filemessaggio==NULL) {
        fprintf(stderr,"FILE NULL\n");
        return -1;}
    fprintf(stderr,"FILE NOT NULL\n");
    char id_letto[5];
    char buffer_mitt[MITTENTE_LEN];
    char buffer_ogg[OGGETTO_LEN];
    char buffer_mes[TESTO_LEN];
    int flag=0;
    fgets(id_letto,6*sizeof(char),filemessaggio);//la prima riga che è il contatore
    if(atoi(ID)<= atoi(id_letto)){
    fprintf(stderr,"ho fatto la compare id_letto: %s\n",id_letto);
    while(!feof(filemessaggio)){
    fgets(id_letto,5*sizeof(char),filemessaggio);
    fprintf(stderr,"id_letto: %s id_letto len %d ID len %d\n",id_letto, (int)strlen(id_letto), (int)strlen(ID));
    fgets(buffer_mitt,MITTENTE_LEN,filemessaggio);
    fprintf(stderr,"mittente: %s\n",buffer_mitt);
    fgets(buffer_ogg,OGGETTO_LEN,filemessaggio);
    fprintf(stderr,"oggetto: %s\n",buffer_ogg);
    fgets(buffer_mes,TESTO_LEN,filemessaggio);
    fprintf(stderr,"CERCO testo: %s\n",buffer_mes);
    if(strlen(id_letto)==strlen(ID) && !memcmp(id_letto,ID,strlen(ID))) {flag=1; break;}
    memset(buffer_mitt,0,MITTENTE_LEN);
    memset(buffer_ogg,0,OGGETTO_LEN);
    memset(buffer_mes,0,TESTO_LEN);
    }
    fclose(filemessaggio);
    if(!flag) ret=-1;
    else {
        strcpy(messaggio->destinatario,buffer_mitt);
        strcpy(messaggio->oggetto,buffer_ogg);
        strcpy(messaggio->testo,buffer_mes);
        ret= 0;}}
    else ret=-1;
    if(!ret){
    char* permesso="1";
    ret=spedisci(permesso, 2*sizeof(char), socket_desc);
    if(ret<0)fprintf(stderr,"Problemi con l'invio del codice");
    fprintf(stderr,"Permesso Inviato %s",permesso);
    ret=spedisci(messaggio->destinatario, MITTENTE_LEN, socket_desc);
    if(ret<0)fprintf(stderr,"Problemi con l'invio del mittente");
    fprintf(stderr,"Permesso Inviato %s",permesso);
    ret=spedisci(messaggio->oggetto, OGGETTO_LEN, socket_desc);
    if(ret<0)fprintf(stderr,"Problemi con l'invio dell'oggetto");
    fprintf(stderr,"Permesso Inviato %s",permesso);
    ret=spedisci(messaggio->testo, TESTO_LEN, socket_desc);
    fprintf(stderr,"%s\n",messaggio->testo);
    if(ret<0)fprintf(stderr,"Problemi con l'invio del testo");
    }
   
    break;
    case 3:
       /* ret=ricevi(ID,5*sizeof(char),socket_desc);
    if(ret<0) fprintf(stderr,"errore nella ricezione\n");
    fprintf(stderr,"ID: %s\n",ID);*/
    strcpy(ID,"2\n");
    fprintf(stderr,"ID: %s\n",ID);
    char* PATH_NUOVO=(char *)malloc((strlen(BASE_PATH)+ MITTENTE_LEN)*sizeof(char));
    strcpy(PATH_LETTO,BASE_PATH);
    strcpy(PATH_NUOVO,BASE_PATH);//apro in parallelo un altro file, copiando il primo
    strcat(PATH_LETTO,loggeduser->name);
    strcat(PATH_NUOVO,"copia");
    strcat(PATH_NUOVO,loggeduser->name);
    fprintf(stderr,"PATH: %s\n",PATH_LETTO);
    FILE * filevecchio= fopen(PATH_LETTO,"r");
    FILE * filenuovo= fopen(PATH_NUOVO,"w");
    if(filevecchio==NULL) {
        fprintf(stderr,"FILE NULL\n");
        return -1;}
    fprintf(stderr,"FILE NOT NULL\n");
    char id_letto2[5];
    //messaggio_t INBOX[MAX_INBOX]={NULL};//check this
    char buffer_mitt2[MITTENTE_LEN];
    char buffer_ogg2[OGGETTO_LEN];
    char buffer_mes2[TESTO_LEN];
    int flag2=0;
    fgets(id_letto2,5*sizeof(char),filevecchio);//la prima riga che è il contatore
    fprintf(filenuovo,"%s",id_letto2);
    if(atoi(ID)<= atoi(id_letto2)){
    while(!feof(filevecchio)){
    fgets(id_letto2,5*sizeof(char),filevecchio);
    fprintf(stderr,"id_letto: %s\n",id_letto2);
    fgets(buffer_mitt2,MITTENTE_LEN,filevecchio);
    fprintf(stderr,"mittente: %s\n",buffer_mitt2);
    fgets(buffer_ogg2,OGGETTO_LEN,filevecchio);
        fprintf(stderr,"ogg: %s\n",buffer_ogg2);
    fgets(buffer_mes2,TESTO_LEN,filevecchio);
        fprintf(stderr,"mex: %s\n",buffer_mes2);
    if(strlen(id_letto2)==strlen(ID) && !memcmp(id_letto2,ID,strlen(ID))) {
        flag2=1;
        fgets(id_letto2,2,filevecchio); 
        fgets(buffer_mitt2,MITTENTE_LEN,filevecchio);
        fgets(buffer_ogg2,OGGETTO_LEN,filevecchio);
        fgets(buffer_mes2,TESTO_LEN,filevecchio);
        strcpy(id_letto2,"");
        strcpy(buffer_mitt2,"");
        strcpy(buffer_ogg2,"");
        strcpy(buffer_mes2,"");
        }
    fprintf(filenuovo,"%s",id_letto2);
    fprintf(filenuovo,"%s",buffer_mitt2);
    fprintf(filenuovo,"%s",buffer_ogg2);
    fprintf(filenuovo,"%s",buffer_mes2);
    memset(id_letto,0,2);
    memset(buffer_mitt2,0,MITTENTE_LEN);
    memset(buffer_ogg2,0,OGGETTO_LEN);
    memset(buffer_mes2,0,TESTO_LEN);
    }
    fclose(filevecchio);
    fclose(filenuovo);
    if(!flag2) ret=-1;
    else {
        strcpy(messaggio->destinatario,buffer_mitt2);
        strcpy(messaggio->oggetto,buffer_ogg2);
        strcpy(messaggio->testo,buffer_mes2);
        ret= 0;}}
    else ret=-1;
    if(!ret){
    char persmesso='1';
    //inserire qui codice per cancellazione messaggi
    }//    fclose(filevecchio);
    fprintf(stderr,"%d",ret);
    break;
}
  return 0  ;
    
    
    }
    
int cercaMessaggio(char ID[5],messaggio_t* mesletto,user_t* loggeduser){ /////NB: in questa parte di codice utilizzo messaggi->destinatario per salvare il MITTENTE
    //free(PATH_LETTO);
    }    
int cercaFile(FILE * file,user_t user){
        char* letto=(char *)malloc(32*sizeof(char));
        char* pass=(char *)malloc(32*sizeof(char));
        int usrname_len= strlen(user.name);
        int pass_len= strlen(user.password);
        int letto_len;
        fgets(letto,32*sizeof(char), file);
        //letto_len=strlen(letto);
        fprintf(stderr,"1 letto:%s zap len:%d\n",letto, (int)strlen(letto));
        fprintf(stderr,"2username:%s zap len:%d\n",user.name,usrname_len);
	    while(!feof(file)) {
            letto_len=(int)strlen(letto)-1;
            fprintf(stderr,"TU CERCHI QUESTO letto:%s usrnm:%s risultato:%d\n",letto,user.name, memcmp(letto,user.name, usrname_len-1));
            if(letto_len==usrname_len  && !memcmp(letto, user.name, usrname_len)){
                fprintf(stderr,"TU CERCHI QUESTO letto:%s usrnm:%s risultato:%d\n",letto,user.name, memcmp(letto,user.name, usrname_len-1));
		        fgets(pass,32*sizeof(char), file);
                if(strlen(pass)-1==pass_len && !memcmp(pass,user.password, pass_len)){                   
			 //Trova lo username e corrisponde alla password 
             
                         return 1;
                         
                }
                else {
                         //Trova lo username ma la password è sbagliata
                         
                         return 2;
                }
            }
            
           fgets(pass,32*sizeof(char),file); 
           fprintf(stderr,"%s\n",pass);
           fgets(letto,32*sizeof(char), file);
        fprintf(stderr,"%s\n",letto);
        fprintf(stderr,"letto:%s len:%d\n",letto,(int)strlen(letto));
        fprintf(stderr,"3 username:%s len:%d\n",user.name,(int)strlen(user.name));
         } free(letto); free(pass);
    //Non trova lo username   
    return 0;
}

//1---stabilire una connessione
void connection_handler(int socket_desc) {    
      
    int ret;
     char * PATH=(char*)malloc(strlen(BASE_PATH)+32*sizeof(char));
    strcpy(PATH,BASE_PATH);
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
    //Dati riguardanti lo user: alloco lo spazio
    user_t* user=(user_t*)malloc(sizeof(user_t));
    user->name= (char*) malloc(32*sizeof(char));
    user->password= (char*) malloc(32*sizeof(char));
    char * old_or_new= (char *) malloc(sizeof(char));
    
    //-------RICEVO L'INFORMAZIONE: Se Registrazione o Log IN---------------//
    while ( (recv_bytes = ricevi(old_or_new, sizeof(old_or_new),socket_desc)) < 0 ) {
        
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    if (DEBUG) fprintf(stderr, "log 1 or new 0: %s \n", old_or_new);//check_this      
    
    
    //-------RICEVO USERNAME E PASSWORD-----------//
    while ( (recv_bytes = ricevi(user->name, 32*sizeof(char), socket_desc)) < 0 ) {
        
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    if (DEBUG) fprintf(stderr, "username received: %s \n", user->name);//check_this
    user->name[strlen(user->name)-1]='\0';
    while ( (recv_bytes = ricevi(user->password, 32*sizeof(char), socket_desc)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to socket");
    }
    
    if (DEBUG) fprintf(stderr, "password received: %s \n", user->password);//check_this
    
    
    FILE * file=fopen("user_data","r");
    if(file==NULL){
	 fprintf(stderr,"impossibile aprire il file!\n");
         exit(1);
	}
   user->password[strlen(user->password)-1]='\0';
   int ceono=cercaFile(file,*user);
   
   fprintf(stderr, "C'è o no: %d", ceono);
   fclose(file);
   char *logged=(char*) malloc(sizeof(char));//serve per avvisare il client se può loggarsi o no
   if(!memcmp(old_or_new,"0", sizeof(char))){ //è un utente che vuole registrarsi
       if(!ceono){ //lo username è disponibile
        
                FILE * file=fopen("user_data","a");//apro il modalità append aggiungo nome e pwd
                if(file==NULL){
                fprintf(stderr,"impossibile aprire il file!\n");
                    exit(1);
                    }
                fprintf(file,"%s\n",user->name);
                fprintf(file,"%s\n",user->password);
                fclose(file);
                strcat(PATH,user->name);
                FILE * messaggi=fopen(PATH,"w");
                fprintf(messaggi,"0\n");  
                        if(file==NULL){
                        fprintf(stderr,"impossibile aprire il file!\n");
                            exit(1);
                        }   
                fclose(messaggi);
                
                logged="1";
                
                    while ( (ret = spedisci(logged, 2, socket_desc)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to the socket");
        logged_server(socket_desc, user);
    }
  
    logged_server(socket_desc, user);}
                
        else{ logged="0"; 
                            while ( (ret = spedisci(logged, 2, socket_desc)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to the socket");
        logged_server(socket_desc, user);}
    }fprintf(stderr, "sent: %s \n", logged);}
    
        //lo username è già stato usato
    else{//è un utente che vuole loggarsi
        if(ceono==1){ logged="1";
                while ( (ret = spedisci(logged, 2, socket_desc)) < 0 ) {
                    
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to the socket");
    }
    fprintf(stderr, "sent: %s \n", logged);
    
        logged_server(socket_desc,user);
        }
        else{ logged="0";
                            while ( (ret = spedisci(logged, 2,socket_desc)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot write to the socket");
    }
    fprintf(stderr, "sent: %s \n", logged);}
    }
    free(old_or_new);

    


    


    //libero la memoria allocata
    
    free(user->name);
    free(user->password);
    free(user);
    // close socket
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket for incoming connection");

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
