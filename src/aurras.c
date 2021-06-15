/* 
* Aurras Client File
*/

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h> 

//RETURNS: buf com memória realocada
void * xrealloc(void *buf,size_t len){

    buf = realloc(buf,len);

    if (buf == NULL) {
        perror("realloc");
        exit(1);
    }

    return buf;
}



// RETURNS: pointer para string totalmente concatenada dos argumentos do stdin
char * strcat_agrv(int argc, char *argv[]){
    char *buf = NULL;
    int dstlen = 0;
    char * src;
    int srclen;
    int seplen = 0;

    // process all arguments
    for (int i = 1; i < argc;  ++i) {
        // point to current argument
        src = strdup(argv[i]);

        // get its length
        srclen = strlen(src);

        // grow the output buffer:
        // output length + space for separator + arg length + space for EOS
        buf = xrealloc(buf,dstlen + seplen + srclen + 1);

        // add the separator [for _subsequent_ arguments]
        if(seplen){
             buf[dstlen++] = '|';

        }
        seplen = 1;

        // append the current argument
        
        strcpy(&buf[dstlen],src);

        // increase output length to account for current argument
        dstlen+= srclen;
    }

    // add EOS string terminator
    buf[dstlen] = 0;

    return buf;
}


int main(int argc, char* argv[]) {
    // decide what to do based on argc
    
    //client -> servidor
    int fd_client;
    
    //servidor -> client
    int fd_server;
    int data;
    char *output_type;
    char buffer[1024];

    if (argc==1) { 
        printf(">> ./aurras status\n");
        printf(">> ./aurras transform input-filename output-filename filter-id-1 filter-id-2 ...\n");
        
    
    } else {
        
        if(fork() == 0){
            if((fd_client=open("/tmp/FIFO",O_WRONLY))<0){
						perror("open");
		    }
            char* output_type = strcat_agrv(argc,argv);	
            //printf("%s\n",output_type);	
            write(fd_client,output_type,strlen(output_type)+1);
        }
        //vou escrever para o servidor que necessito : se status ou um transform
         // usage historic information  
        fd_server = open("FIFO2",O_RDONLY);
    	char * resposta = (char*) malloc(sizeof(1024));
    	while(1){
			while((data = read(fd_server,resposta,1024)) > 0) {
                    write(1,resposta,data);
			}
    	}
        

    }
    return 0;
}