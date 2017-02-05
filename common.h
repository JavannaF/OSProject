#ifndef COMMON_H
#define COMMON_H

// macro to simplify error handling
#define ERROR_HELPER(ret, message)  do {                                \
            if (ret < 0) {                                              \
                fprintf(stderr, "%s: %s\n", message, strerror(errno));  \
                exit(EXIT_FAILURE);                                     \
            }                                                           \
        } while (0)

/* Configuration parameters */
#define DEBUG           1   // display debug messages
#define MAX_CONN_QUEUE  3   // max number of connections the server can queue
#define SERVER_ADDRESS  "127.0.0.1"
#define SERVER_COMMAND  "0"
#define SERVER_PORT     2015
#define BASE_PATH        "/home/biar/Desktop/OSProject/messaggi/"
#define DESTINATARIO_LEN  32*sizeof(char)
#define MITTENTE_LEN  32*sizeof(char)
#define OGGETTO_LEN       64
#define TESTO_LEN         256
#define MAX_INBOX         10
typedef struct {
  char * name;
  char * password;

}user_t;

typedef struct {
   char* destinatario;
   char* oggetto;
   char* testo;
   int ID;
}messaggio_t;

int cercaFile(FILE * file,user_t user, short int pwd);
#endif
