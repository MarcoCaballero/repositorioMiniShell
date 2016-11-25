#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "parser.h"



int main(void) {
	char buf[1024];
	tline * line;
	int i,j,k;
	pid_t pid;
	int status;
	// Para las redirecciones de entrada, salia o error
	FILE *file;
	int backup_in = dup(fileno(stdin));
	int backup_out = dup(fileno(stdout));
	int backup_err = dup(fileno(stderr));
	// Ruta por defecto al ejecutar el comando cd
	char *defaultDirectory = getenv("HOME");
	char auxDirectory[1024]; // Auxiliar para el manejo del cd
	char currentDirectory[1024]; // Para almacenar el directorio actual
	getcwd(currentDirectory, sizeof(currentDirectory));
	char **firstCommandArguments;
        //para los pipes
        
	//Ignoramos las señales SIGINT y SIGQUIT
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN); 


	printf("msh> ");
	
	while (fgets(buf, 1024, stdin)) {
		//printf("%s", currentDirectory);
		//printf("%s", defaultDirectory);
		

		line = tokenize(buf);

		if (line==NULL) {
			fprintf(stderr, "Vacío\n");
			continue;
		}
		int commandsNumber = line->ncommands;
		int fd[commandsNumber*2];// pipe1 [0][0-1] pipe2[1][0-1]....

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
			
                	//comandos ejecutables	

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
			                printf("comando a ejecutarse en background\n");
		                } 
                                
                               
                                for (j=0;j<line->ncommands;j++){
                                        if(pipe(fd+j*2)<0){
                                                fprintf(stderr, "Imposilbe to pipe().\n%s\n", strerror(errno));     
                                                exit(0);   
                                        } 
     
                                
                                }
                            
                                
                               
                                
                                int jp = 0;
                                
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
                                                exit(0);
                                                }
                                               
					        //Si llega aquí es que se ha producido un error en el execvp
					        //printf("Error al ejecutar el comando: \n%s\n", strerror(errno));
					        //exit(status);//regresa al wait del padre 
		
				        }
				       
				       
                                
                                jp+=2;
			        }
			        // Restauramos los ficheros de redirección
                                                                
                                for (k=0; k<2*line->ncommands;k++){
                                        close(fd[k]);
                                }
                                for (k=0; k<=line->ncommands;k++)
                                        wait(&status);
                                
                               
					        //exit(0);
			        dup2(backup_in, fileno(stdin));
			        dup2(backup_out, fileno(stdout));
			        dup2(backup_err, fileno(stderr));
                                printf("BACKUPEADO\n");
		     }
               
                }else{
                        printf("msh> ");
                        continue;
                }
	printf("msh> ");	
	}
	return 0;
}
