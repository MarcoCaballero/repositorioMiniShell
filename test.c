#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "parser.h"




void compruebaBGfin(int index, pid_t *lista, char**listaLineas);

void compruebaBGfin(int index, pid_t *lista, char**listaLineas){
int kpid, status, k;      
        for(k=0;k<index;k++){
               if((kpid=waitpid(lista[k],&status,WNOHANG))>0){
                        printf("[%d] - %d             %s\n", k+1, lista[k],listaLineas[k]);
                }                
                        
        }


}

int main(void) {
        //manejo de cmdline and command
	char buf[1024];
	tline * line;
       
        //variables auxiliares
	int i,j,k,jp,commandsNumber,status, indexBG,kpid;
	pid_t pid;	
        char **firstCommandArguments;
	// Para las redirecciones de entrada, salia o error
	FILE *file;
	int backup_in = dup(fileno(stdin));
	int backup_out = dup(fileno(stdout));
	int backup_err = dup(fileno(stderr));
	// Variables para el manejo del comando cd
	char *defaultDirectory = getenv("HOME");
	char auxDirectory[1024]; // Auxiliar para el manejo del cd
	char currentDirectory[1024]; // Para almacenar el directorio actual
	getcwd(currentDirectory, sizeof(currentDirectory));
	
        //para los pipes
        int *fd;

        //Procesos bg
        pid_t *lista;
        int test;
        char **listaLineas;
        
	//Ignoramos las señales SIGINT y SIGQUIT
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN); 
        
        indexBG = 0;
	printf("-msh> ");
       lista = (pid_t*)malloc(sizeof(pid_t*));
       listaLineas = (char**)malloc(sizeof(char**));
	while (fgets(buf, 1024, stdin)) {
		
                
		line = tokenize(buf);

		if (line==NULL) {
			fprintf(stderr, "Vacío\n");
			continue;
		}

                //COMPROBAR PROCESOS EN BACKGROUND (TERMINADOS ??)
                compruebaBGfin(indexBG,lista,listaLineas);
		commandsNumber = line->ncommands;
		
                fd = (int*)malloc((commandsNumber*2)*sizeof(int*)); 
                lista = (pid_t*)realloc(lista,(indexBG+1)*sizeof(pid_t*));
                listaLineas = (char**)realloc(listaLineas,(indexBG+1)*sizeof(char**)); 
                
                if(line->ncommands!=0){
		firstCommandArguments = line->commands[0].argv;
                        //comandos propios
		        if(strcmp(firstCommandArguments[0], "cd")==0){

				fprintf(stderr,"Arg: %d\n", line->commands[0].argc);
		        	if(line->commands[0].argc < 2){
					fprintf(stderr,"Dir: %s\n", defaultDirectory);
					//strcpy(auxDirectory, defaultDirectory);
					chdir(defaultDirectory);
				}else{
					fprintf(stderr,"Dir: %s\n", firstCommandArguments[1]);
					//strcpy(auxDirectory, firstCommandArguments[1]);
					chdir(firstCommandArguments[1]);
				}
                        
			        getcwd(currentDirectory, sizeof(currentDirectory));
                	//comandos ejecutables	

		        }if(strcmp(firstCommandArguments[0], "jobs")==0){
                                

                                
                                 for(k=0;k<indexBG;k++){
                                         if((kpid=waitpid(lista[k],&status,WNOHANG))<=0){
                                                printf("[%d] - %d             %s\n", k+1, lista[k],listaLineas[k]);
                                        }                
                        
                                }
                        }else{
                
                                if (line->redirect_input != NULL) {
			                printf("redirección de entrada: %s\n", line->redirect_input);
			                char *fileName = line->redirect_input;
			                file = fopen(fileName, "r");
			                if(file == NULL){// Si el fichero está vacío
				                fprintf(stderr, "%s :File error.%s\n", fileName, strerror(errno));
				                printf("msh> ");
				                continue;
			                }else{
				                // Si no está vacío, el fichero de entrada es "file"
				                int fileNumberEnter = fileno(file);
				                //Duplica lo que hay dentro del archivo
				                dup2(fileNumberEnter, 0);
				                fclose(file);
			                }
		                }
		                if (line->redirect_output != NULL) {
			                printf("redirección de salida: %s\n", line->redirect_output);
			                char *fileName = line->redirect_output;			
			                file = fopen(fileName, "a");
			                if (file==NULL){
				                fprintf(stderr, "%s :File error.%s\n", fileName, strerror(errno));
				                printf("msh> ");	
				                continue;
			                }else{
				                // El fichero se salida será "file"
				                int fileNumberExit = fileno(file);
				                dup2(fileNumberExit, 1);
				                fclose(file);
			                }
		                }
		                if (line->redirect_error != NULL) {
			                printf("redirección de error: %s\n", line->redirect_error);
			                char *fileName = line->redirect_error;
			                file = fopen(fileName, "a");
			                if (file==NULL){
				                fprintf(stderr, "%s :File error.%s\n", fileName ,strerror(errno));
				                printf("msh> ");	
				                continue;
			                }else{
				                // El fichero de error es ahora "file"
				                int FileNumberError = fileno(file);
				                dup2(FileNumberError, 2); //Ahora la salida de error es por el descriptor del archivo.
				                fclose(file);
			                }
		                }
		                if (line->background) {
			                printf("comando a ejecutarse en background-%d\n",line->background);
		                } 
                                
                               
                                for (j=0;j<line->ncommands;j++){
                                        if(pipe(fd+j*2)<0){
                                                fprintf(stderr, "Imposilbe to pipe().\n%s\n", strerror(errno));     
                                                exit(0);   
                                        } 
     
                                
                                }
                            
                                
                               
                                
                                jp = 0;
                                
		                for (i=0; i<line->ncommands; i++) {
			                //printf("orden %d (%s):\n", i, line->commands[i].filename);
                                        
                                     
                                        	             

				        pid = fork();
                                        
				        if (pid < 0) { /* Error */
					        fprintf(stderr, "Falló el fork().\n%s\n", strerror(errno));
					        exit(1);
				        }
				        else if (pid == 0) { /* Proceso Hijo */
                                                                                                
                                                            
                                                if(i!=0){
                                                                                                                 
                                                        if(dup2(fd[jp - 2],0)<0){
                                                                 fprintf(stderr, "Imposilbe to dup2 no prim.\n%s\n", strerror(errno));                                                          exit(0);                 
                                                        }
                                                        close(fd[jp - 2]); 
				     
                                                }
                                                if(i!=line->ncommands-1){
                                                                                                                
                                                         if(dup2(fd[jp + 1], 1)<0){
                                                                 fprintf(stderr, "Imposilbe to dup2 no ult().\n%s\n", strerror(errno));                                                         exit(0);                 
                                                         }
				                       close(fd[jp + 1]);     
                                                }
                                                
                                              


                                                for (k=0; k<2*line->ncommands;k++){
                                                        close(fd[k]);
                                                }

					      
					        if(execv(line->commands[i].filename, line->commands[i].argv)<0){
                                                printf("Error al ejecutar el comando");
                                                exit(status);
                                                }
                                               
					        //Si llega aquí es que se ha producido un error en el execvp
					        //printf("Error al ejecutar el comando: \n%s\n", strerror(errno));
					        //exit(status);//regresa al wait del padre 
		
				        }
				     
                                        
				      test = pid; 
                                
                                jp+=2;
			        }
			        // Restauramos los ficheros de redirección
                                                                
                                for (k=0; k<2*line->ncommands;k++){
                                        close(fd[k]);
                                }
                                
                                
                                        if (line->background != 1) {
                                              
                                             for (k=0; k<=line->ncommands;k++){                                               	
                                                wait(&status);
                                             }
		                        } else {
                                               listaLineas[indexBG] =  (char*)malloc(1024*sizeof(char));
                                              lista[indexBG] = test;
                                              strcpy(listaLineas[indexBG],strtok(buf,"&"));
                                              indexBG++; 
                                              printf("[%d]-%d\n",indexBG, test);
                                                
                                                 
                                        }
                                
                                                              
                              
                                
                               
					        //exit(0);
			        dup2(backup_in, fileno(stdin));
			        dup2(backup_out, fileno(stdout));
			        dup2(backup_err, fileno(stderr));
                              
		     }
               
                }else{
                        printf("-msh> ");
                        continue;
                }
	printf("-msh> ");	
	}
	return 0;
}
