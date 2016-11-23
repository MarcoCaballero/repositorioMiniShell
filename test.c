#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "parser.h"
#define _cd "cd"


int main(void) {
	char buf[1024];
	tline * line;
	int i,j;
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
	//Ignoramos las señales SIGINT y SIGQUIT
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN); 


	printf("msh> ");	
	while (fgets(buf, 1024, stdin)) {
		

		line = tokenize(buf);

                int commandsNumber = line->ncommands;
		firstCommandArguments = line->commands[0].argv;

		if (line==NULL) {
			continue;
		}
                

                if(commandsNumber!=0){
                        //comandos propios
		        if(strcmp(firstCommandArguments[0], _cd)==0){
		        printf("HAS DADO CON LA PUTA CLAVE\n");
                        
			
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
				                dup2(fileNumberEnter, fileno(stdin));
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
				                dup2(fileNumberExit, fileno(stdout));
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
				                dup2(FileNumberError, fileno(stderr)); //Ahora la salida de error es por el descriptor del archivo.
				                fclose(file);
			                }
		                }
		                if (line->background) {
			                printf("comando a ejecutarse en background\n");
		                } 
		                for (i=0; i<line->ncommands; i++) {
			                printf("orden %d (%s):\n", i, line->commands[i].filename);
			        //for (j=0; j<line->commands[i].argc; j++) {
				        //printf("  argumento %d: %s\n", j, line->commands[i].argv[j]);
				        pid = fork();
				        if (pid < 0) { /* Error */
					        fprintf(stderr, "Falló el fork().\n%s\n", strerror(errno));
					        exit(1);
				        }
				        else if (pid == 0) { /* Proceso Hijo */
					        printf("  argumento %d: %s\n", 1, line->commands[i].argv[1]);
					        execv(line->commands[i].filename, line->commands[i].argv);
					        //Si llega aquí es que se ha producido un error en el execvp
					        printf("Error al ejecutar el comando: \n%s\n", strerror(errno));


					        exit(status);
		
				        }
				        //else if(pid > 0){
					        //printf("Volvemos al proceso padre\n");
				        //}
				        else {
					        wait(&status);
					        //waitpid(pid, NULL, 0);
					        if (WIFEXITED(status) != 0)
						        if (WEXITSTATUS(status) != 0)
							        printf("El comando no se ejecutó correctamente\n");
					        //exit(0);
				        }
			        //}
			        // Restauramos los ficheros de redirección
			        dup2(backup_in, fileno(stdin));
			        dup2(backup_out, fileno(stdout));
			        dup2(backup_err, fileno(stderr));
		                }
                        }
               
                }else{
                        printf("msh> ");
                        continue;
                }        
	printf("msh> ");	
	}
	return 0;
}
