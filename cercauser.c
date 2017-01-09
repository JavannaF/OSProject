#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct{
  char * name;
  char * password;
  int cont_msg;
}user_t;

int cercaFile(FILE * file,user_t user){
    
        char* letto=malloc(32*sizeof(char));
        char* pass=malloc(32*sizeof(char));
        char* contachar=(char *)malloc(sizeof(char));
        int usrname_len= strlen(user.name);
        int pass_len= strlen(user.password);
        fgets(letto,32*sizeof(char), file);
	    while(!feof(file)) {
            if(memcmp(letto,user.name, strlen(letto)-1)==0){
		        fgets(pass,32*sizeof(char), file);
                if(memcmp(pass,user.password, strlen(pass)-1)==0){                   
                                fgets(contachar,sizeof(char), file);
                user.cont_msg= atoi((const char *)contachar);
			 printf("Benvenuto %s e int %d\n",user.name, user.cont_msg); 
                         return 1;
                }
                else {
                         printf("Password sbagliata\n");
                         return -1;
                }
            }
           fgets(pass,32*sizeof(char),file);
           fprintf(file,"zucchero");
           fgets(contachar,sizeof(char), file);
           fgets(letto,32*sizeof(char), file);   
           
         }
    printf("Non ho trovato corrispondenza col tuo username %s \n",user.name);
        
        
    return 0;
}


int main(int argc,int argv[]){
    /*FILE * file=fopen("user_data","r+w");
    if(file==NULL){
	 printf("impossibile aprire il file!\n");
         exit(1);
	}
    user_t user;
    user.name= "prov";
    user.password="cosa";
    cercaFile(file,user);*/
    printf("%li",sizeof(char));
    return 0;
}
