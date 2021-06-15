/* 
* Aurras Server File
*/

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h> 

#define NUM_FILTERS 5
#define BUF_SIZE 1024


/*
NAME      EXECUTAVEL(sem argumentos)    MAX INSTACIAS CONC      INDEX                                                 
alto       aurrasd-gain-double              2                    0
baixo      aurrasd-gain-half                2                    1
eco        aurrasd-echo                     1                    2
rapido     aurrasd-tempo-double             2                    3
lento      aurrasd-tempo-half               1                    4
*/


char* filters_name[NUM_FILTERS];
char* filters_location[NUM_FILTERS];
int filters_current[NUM_FILTERS]; //ISTO NÃO VAI PODER SER GUARDADO GLOBALMENTE PQ OS FILHOS NÃO TEM ACESSO A ESTE, SÓ O PAI
int filters_limit[NUM_FILTERS];


int fd_client;
int fd_server;
int val=0;
int * pids;
int pids_count=0;
int * processos;
int proc=0;



void graceful_handler(int signum){
    for(int i = 0 ; i< pids_count;i++)
    printf("Guardar dados");
}



char** parseComando_transform(char* input, char** task){
       int quantosTabs=0;;
       int quantos=0;

       
       for(int i =0; input[i]!='\0';i++){
		if(input[i]=='|') quantosTabs++;
	    }
        
        //does not matter
        char* ptr =strtok(input, "|");
        
        task = malloc(sizeof(char*) * quantosTabs);
        while (ptr != NULL)
        {
            
            task[quantos] = strdup(ptr);
            quantos++;
            ptr = strtok(NULL, "|");
            
        }
        task[quantos] = NULL;
        quantos=0;
        return task; //daqui vejo o numero de filtros
        
}




//bash$ bin/aurrasd-filters/aurrasd-echo < samples/samples-1.m4a > output.mp3
//              location                        input               output


int executa(char* arg, int servpid){
    
    char** task = parseComando_transform(arg,task); //fazer
    int quantos = 0;
    for(int i = 0; task[i]!=NULL;i++){
        quantos++;

    }
    
    int n_filters = quantos -3; //numero total de filtros
    pids_count = n_filters; //numero de processos igual a filtros
    int pipe_filters[n_filters-1][2]; //numero de pipes;
    int pid;

    if(signal(SIGTERM, graceful_handler)==SIG_ERR){
			perror("signal sigterm");
            exit(-1);	
	}
        // task[3]                          task[1]                  task[2]
    //../bin/aurrasd-filters/aurrasd-echo < ../samples/sample-1-so.m4a > output.mp3
    if(n_filters == 1){//sem pipes
            pids = (int*) malloc(sizeof(int)*n_filters);
            if((pid = fork()==0)){
                char* filter = strdup(task[3]);
                //printf("%s\n",task[1]); 
                //printf("%s\n",task[2]);


                int fi = open(task[1],O_RDONLY,S_IRWXU);
                int fo = open(task[2],O_CREAT | O_WRONLY,S_IRWXU);
                dup2(fi,1);
                dup2(fo,0);
                //criar string do comando para pudermos meter no execvp ou exclp
                execlp(filter,filter,task[1],task[2],NULL);
                exit(-1);
            }
            pids[0] = pid;
    }else{
        //bin/aurrasd-filters/aurrasd-echo < samples/sample-1-so.m4a | bin/aurrasd-filters/aurrasd-tempo-half > output.mp3
        pids = (int*) malloc(sizeof(int)*n_filters);
        for(int i=0; i< n_filters;i++){
            if(i==0){//primeiro comando
                if(pipe(pipe_filters[i]) == -1){ //criamos o pipe 
                   perror("pipe[0]");
                   exit(-1); 
                }
                if((pid = fork()) == 0){
                    close(pipe_filters[i][0]);
                    dup2(pipe_filters[i][1],1); //descritor de escrita para o stdout de grep
                    close(pipe_filters[i][1]);
                    //criar string do comando para pudermos meter no execp ou execlp
                    //execlp ou execvp
                    //_exit(1);
                }
                close(pipe_filters[i][1]);
    
    
            }

            if(i == n_filters-1){//último comando
                //não necessito de criar pipe, apenas dirigimos o descritoe de leitura do último pipe para stdin
                if((pid = fork()) == 0){
                    dup2(pipe_filters[i-1][0],0);
                    close(pipe_filters[i-1][0]);
                    //criar string do comando para pudermos meter no execp ou execlp
                    //execlp ou execvp
                    //_exit(1);
                }
                close(pipe_filters[i-1][0]);
            
            }else{
                //comandos intermédios
                if(pipe(pipe_filters[i]) == -1){ //criamos o pipe 
                   perror("pipe_filters[i]");
                   _exit(-1); 
                }


                if((pid = fork()) == 0){
                    close(pipe_filters[i][0]);
                    dup2(pipe_filters[i-1][0],0);
                    close(pipe_filters[i-1][0]);
                    dup2(pipe_filters[i][1],1);
                    close(pipe_filters[i][1]);
                    //criar string do comando para pudermos meter no execp ou execlp
                    //execlp ou execvp
                    //_exit(1);
                }

                close(pipe_filters[i-1][0]);
                close(pipe_filters[i][0]);

            }
            pids[i] = pid;

        }

        
    }
    int status;
	for(int i=0;i<n_filters;i++){
		wait(&status);
		if(WIFEXITED(status)==0) val=-1;
	}
    printf("CHEGOU AQUII\n");
    return val;
}

int main(int argc, char* argv[]) {
    // information needed : Name; Location; Limit; Current;
    // processing arguments
    // argv[1] -> config_filename
    // argv[2] -> filters_folder

    
    // parse configs
    char* path_conf = malloc(sizeof(char*));
    strcpy(path_conf,"../");
    strcat(path_conf,argv[1]);
    
    int fd = open(path_conf, O_RDONLY); 
    char* buf = malloc(sizeof(char)*BUF_SIZE);
    read(fd, buf, BUF_SIZE);
    
    for(int i=0;i<NUM_FILTERS;i++) {
        
        char* filter = strsep(&buf, "\n");
        
        // name
        char* name = strsep(&filter, " ");
        filters_name[i] = strdup(name);
        
        // location
        char* aux = strsep(&filter, " ");
        char* location = malloc(sizeof(char*));
        strcpy(location, "/");
        strcat(location,aux);
        filters_location[i] = strdup(location);

        // limit
        char* limit = strsep(&filter, " ");
        filters_limit[i] = atoi(limit);
        
        // current
        filters_current[i] = 0;
    }
    
    close(fd);
    
    //fill path on filters' array
    char* filters_folder = malloc(sizeof(char*));
    strcpy(filters_folder, "../");
    strcat(filters_folder,argv[2]);

    for(int i=0; i<NUM_FILTERS; i++) {
        char aux[128]; 
        strcpy(aux, filters_folder);
        strcat(aux, filters_location[i]);
        filters_location[i] = strdup(aux);
    }

    /*CRIAR HANDLER PARA NÃO DEIXAR QUE MAIS CLIENTES ACEDAM AO SISTEMA 
    GUARDA PIDS QUE ESTEJAM ATIVOS PARA OS DEIXAR ACABAR???
    */
    if(signal(SIGTERM, graceful_handler)==SIG_ERR){
        perror("signal SIGCHLDERRO");  
        exit(3);
    }
     
    



    //criar named pipe
    //client -> servidor
	mkfifo("/tmp/FIFO",0666);
    //servidor -> client
    mkfifo("/tmp/FIFO2",0666);
    char buffer[1024];
    
    int servidor_pid = getpid();
    int data;
    while (1){
        fd_client = open("/tmp/FIFO",O_RDONLY);
        while((data = read(fd_client,buffer,1024)) > 0) {
        
            //write(STDOUT_FILENO,buffer,data);
            char * guarda = strdup(buffer);
            pids_count++;
            
            int pipe_paiFilho[2]; // Mecanismo de comunicação entre processos
            if(pipe(pipe_paiFilho)<0){
                perror("pipe_paiFilho FAILED");
                exit(1);
            }

            //Um ouvido para cada cliente - processo para cada cliente
            switch(fork()){
             case -1:
                perror("fork FAILED");
                _exit(-1);
            
             case 0:
                close(pipe_paiFilho[0]); //fechamos de leitura 
    		    int pid;
    	        int status;
                printf("%s\n",buffer);
                if(strcmp(buffer,"status") == 0){
                    //ir buscar todas as task ainda em execucao em processos?
                    //write(pip_paiFilho[1],processos);
                    //ir buscar todos os filtros os valores de current e max
                    char** filtros_stat = malloc(sizeof(char*) * NUM_FILTERS);
                    for(int i=0; i< NUM_FILTERS ;i++){
                        char FILTER[1024];
                        char number1[10];
                        char number2[10];
                        strcat(FILTER,"filter ");
                        strcat(FILTER,filters_name[i]);
                        strcat(FILTER,": ");
                        sprintf(number1,"%d",filters_current[i]);
                        strcat(FILTER,number1);
                        strcat(FILTER,"/");
                        sprintf(number2,"%d",filters_limit[i]);
                        strcat(FILTER,number2);
                        strcat(FILTER,"\n");
                        strcpy(filtros_stat[i],FILTER);
                    }
                    //strcat(processos,filtros_stat);
                    //write(pip_paiFilho[1],processos);
                    //_exit(0);

                
                }else{//saber o numero de filtros - 

                    char  TASKPID[1024];
                    char  number[20];
                    strcat(TASKPID, "task #");
                    sprintf(number,"%d",pids_count);
                    strcat(TASKPID, number);
                    strcat(TASKPID,": ");
                    strcat(TASKPID,guarda);
                    strcat(TASKPID, "\n");    //SAVE TASK TO PAI  - EXEMPLO:task #3: transform|input-filename|output-filename|filter-id-1|filter-id-1
                    write(pipe_paiFilho[1],TASKPID,strlen(TASKPID));

                    close(pipe_paiFilho[1]);
                    int suc = executa(buffer,servidor_pid);
                    if(suc == 0) write(1,"Task realizada com sucesso\n",28);  //task com sucesso
                    if(suc == -1) write(1,"Task realizada com insucesso\n",30); //task sem sucesso
                    exit(1);
                }

            
             default:
                close(pipe_paiFilho[1]);
                char** tarefa = malloc(sizeof(char*)*pids_count); //vamos ter de multiplicar consoante o numero de processos que temos
                int i=0;
                char buf[1024];
                while(( data = read(pipe_paiFilho[0],&buf,1)) > 0){ //escrevo para o STDOUT os dados que os processos filhos lhe vão mandando
                    tarefa[i]=strdup(buf);
                    i++;
                }
                proc++;
                close(pipe_paiFilho[0]);
            
            }
            
         }
    }
    close(fd_client);


    for(int i=0;i<NUM_FILTERS;i++) free(filters_location[i]);
    for(int i=0;i<NUM_FILTERS;i++) free(filters_name[i]);
    return 0;


}