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
#define BASE_PATH        "/home/biar/Desktop/quaglia/"
typedef struct {
  char * name;
  char * password;
  int msg_cont;

}user_t;

typedef struct {
   int ID_mess;
   char destinatario[32];
   char oggetto[156];
   char testo[1024];
}messaggio_t;

int cercaFile(FILE * file,user_t user);
#endif
