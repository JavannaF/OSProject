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


//funzione per ricevere (gestisce invii parziali)
int ricevi(char *buf, int dim, int socket_desc){
    int recv_bytes=0;
    int letti=0;
    int flag=1;
    while (flag) {
    letti = recv(socket_desc, buf + recv_bytes, dim - recv_bytes, 0);
    if (letti < 0 && errno == EINTR) continue;
    if (letti < 0) {fprintf(stderr,"ERRORE\n");return -1;}//error: return -1
    if(letti==0) {fprintf(stderr,"ERRORE\n");break;}
    recv_bytes += letti;
    if (recv_bytes > 0 && (buf[recv_bytes - 1] == '\0' || recv_bytes>32*sizeof(char))) {
    flag = 0;
    }
    if (recv_bytes == 0) {fprintf(stderr,"ERRORE\n");break;}
    }
    return recv_bytes;
    }
    
    
//funzione per spedire (gestisce invii parziali)
int spedisci(char*buf, int dim, int socket_desc){	
    int spediti;
    int recv_bytes=0;
    while (recv_bytes<dim) {
    spediti = send(socket_desc, buf+ recv_bytes, dim - recv_bytes, 0);
    if (spediti < 0 && errno == EINTR) continue;
    if (spediti < 0) {fprintf(stderr,"ERRORE\n");return -1;}//error: return -1
    recv_bytes += spediti;
    
    
    if (recv_bytes == 0){fprintf(stderr,"ERRORE\n"); return -1;}
}
 return spediti;   
}

void logged_client(int socket_desc){
    char * option=(char *) malloc(2*sizeof(char));
    int ret;
    int option_len=2*sizeof(char);
    while(1){
    fprintf(stderr, "Se vuoi scrivere un nuovo messaggio premi 1, se vuoi leggere un vecchio messaggio premi 2, se vuoi cancellarne uno premi 3,se vuoi uscire premi 0");
    *option=fgetc(stdin);
    fgetc(stdin);//pulisce stdin da /n
    if (!memcmp(option, "0", option_len)) break;//L'utente vuole uscire
    ret = spedisci(option, option_len, socket_desc);

    fprintf(stderr, "sent: %s \n", option);
        char* nome=(char*)malloc(DESTINATARIO_LEN);
        char* oggetto=(char*)malloc(OGGETTO_LEN);
        char* messaggio=(char*)malloc(TESTO_LEN);
        int optionint=atoi(option);
    switch (optionint){       
        case 1://voglio scrivere un nuovo messaggio
        fprintf(stderr, "A chi vuoi inviare il tuo messaggio?\n");//chiedo il nome
        fgets(nome,DESTINATARIO_LEN,stdin);
        ret=spedisci(nome,strlen(nome)+1,socket_desc);
        fprintf(stderr, "sent: %s quanti byte mando(teminatore + a capo)%d quanti byte è lungo il nome(a capo)%d\n", nome,ret,((int)(strlen(nome))));
        fprintf(stderr, "Quale è l'oggetto del tuo messaggio? \n");//chiedo l'oggetto
        fgets(oggetto,OGGETTO_LEN,stdin);
        ret = spedisci(oggetto, strlen(oggetto)+1, socket_desc);
    if (DEBUG) fprintf(stderr, "sent: %s \n", oggetto);   
    fprintf(stderr, "Scrivi il tuo messaggio qui: \n"); //chiedo il messaggio
    char * temp;
    fgets(messaggio,TESTO_LEN,stdin);  
    ret=spedisci(messaggio, strlen(messaggio)+1,socket_desc);
    if (DEBUG) fprintf(stderr, "sent: %s \n", messaggio);   
             
        break;
       case 2:

       fprintf(stderr,"Quale messsaggio desideri leggere? Digita qui l'id:");
       char id[5]={0};
       char presenza_id;
       fgets((char*)id,5*sizeof(char),stdin);
       ret = spedisci((char *)id, 6*sizeof(char), socket_desc);
        if(ret==-1) fprintf(stderr,"ERRORE\n");
        fprintf(stderr,"ID mandato= %s\n",(char *)id); 
        ret = ricevi(&presenza_id, 2*sizeof(char), socket_desc);
        if(ret<0) fprintf(stderr,"Errori col mittente\n");
        fprintf(stderr,"presenza id ricevuta %s\n",&presenza_id);
        if(!memcmp(&presenza_id,"1",2*sizeof(char))){
        ret = ricevi(nome , MITTENTE_LEN, socket_desc);
        if(ret<0 ) fprintf(stderr,"Errori col mittente\n");
        fprintf(stderr,"Mittente: %s\n",(char *)nome);  
        ret = ricevi(oggetto , OGGETTO_LEN,socket_desc);
        if(ret<0) fprintf(stderr,"Errori con l'oggetto\n");
        fprintf(stderr,"Oggetto:%s\n",oggetto);  
        ricevi(messaggio , TESTO_LEN,socket_desc);
        if(ret<0) fprintf(stderr,"Errori il Testo\n");            
        fprintf(stderr,"Testo:%s\n", messaggio );}
        else fprintf(stderr,"Siamo spiacenti ma il messaggio da lei cercato non è presente\n");
        break;
        
        case 3:
         
        fprintf(stderr,"Digita l'ID del messaggio che vuoi cancellare: ");
        char ID[5]={0};
        fgets((char *)ID,5,stdin);
        ret=spedisci((char *) ID,5,socket_desc);
        break;
        }
    return;
    
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
    ERROR_HELPER(socket_desc, "Non riesco a creare la socket\n");

    // set up parameters for the connection
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!
//FASE 1: Log_In e Registrazione 
    // initiate a connection on the socket
    ret = connect(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    ERROR_HELPER(ret, "Non riesco a stabilireuna connessione\n");


    fprintf(stderr, "Connessione stabilita!\n");    
    printf("Benvenuto! Se sei un nuovo utente premi '0', '1' altrimenti.\n");
    *old_or_new=fgetc(stdin);
    fgetc(stdin);//pulisce da /n
    
    //------------------AVVERTO SERVER SE REGISTRAZIONE O LOG IN------------------//
    if( spedisci(old_or_new, 2*sizeof(char), socket_desc) < 0) {
        
       fprintf(stderr,"Errore nell'invio dei dati al sever, Arrivederci\n"); return -1;
    }
    
    fprintf(stderr, "sent: %s \n", old_or_new);
    
    char* recv_buf=(char *) malloc(sizeof(char));
    size_t recv_buf_len = sizeof(char);
    int recv_bytes;

    /////////////HANDLE USERNAME AND PASSWORD////////////
    char* usr_name=(char*)malloc(32*sizeof(char));
    char* usr_pwd=(char*)malloc(32*sizeof(char));
    size_t usr_name_len = 32*sizeof(char);
    size_t usr_pwd_len = 32*sizeof(char);
    printf("Inserisci uno username (massimo 32 caratteri) \n"); 
    fgets(usr_name,DESTINATARIO_LEN,stdin); //---->INSERISCI CONTROLLI 
    char * PATH=(char*)malloc(strlen(BASE_PATH)+strlen(usr_name));
    strcpy(PATH,BASE_PATH);
   if(spedisci(usr_name, usr_name_len, socket_desc) <= 0) {
               fprintf(stderr,"Errore nell'invio dei dati al sever, Arrivederci \n"); return -1;
    }
    
   fprintf(stderr, "sent: %s \n", usr_name);
   printf("Inserisci una password (massimo 32 caratteri)\n"); 
   fgets(usr_pwd,DESTINATARIO_LEN,stdin);   //---->INSERISCI CONTROLLI
   if(spedisci(usr_pwd, usr_pwd_len, socket_desc) <= 0) {
       fprintf(stderr,"Errore nell'invio dei dati al sever, Arrivederci\n"); return -1;
    }
    
    fprintf(stderr, "sent: %s \n", usr_pwd);

    memset(recv_buf,0,recv_buf_len);
    recv_bytes=-1;
        // risposta sul nome---> dice se per lo user può fare il login se è disponibile per chi si registra
    if(ricevi(recv_buf, 2,socket_desc) <= 0 ) {
       fprintf(stderr,"Errore nella ricezione dei dati dal sever, Arrivederci \n"); return -1;
    }

    
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
                fprintf(stderr,"Bentornato %s\n",usr_name);
                logged_client(socket_desc);
                }
            }
            
    recv_buf[recv_bytes] = '\0'; // add string terminator manually!*/

    

    // close the socket
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Non riesco a chiudere la socket \n");

    //("Answer from server: %s", recv_buf);

    if (DEBUG) fprintf(stderr, "Arrivedercie Grazie di aver usato il nostro servizio di messaggistica!\n");

    exit(EXIT_SUCCESS);


}
