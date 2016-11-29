#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "parser.h"




int compruebaBGfin(int index, pid_t *lista, char**listaLineas,char**listaEstados,int isOn);

int compruebaIndexBG(int index,int *isOn);

int compruebaBGfin(int index, pid_t *lista, char**listaLineas,char**listaEstados,int isOn){
int kpid, newisOn, status;   
        newisOn = isOn;   
       
        if((kpid=waitpid(lista[index],&status,WNOHANG))>0){
                printf("[%d]    %d    HECHO          %s\n", index, lista[index],listaLineas[index]);
                
                newisOn = 0;
                        
        }              
        
        return newisOn;
}

int compruebaIndexBG(int index,int *isOn){
int k,maximo,numActivos;
maximo = index;
numActivos = 0;

        for(k=1;k<=index;k++){
                if(isOn[k]==1){
                        maximo = k;
                        numActivos++;
                        
                }else{
                
                }
        }
        
        if (numActivos == 0){
                maximo = 0;
        }
        
        return maximo;

}

int main(void) {
        //manejo de cmdline and command
	char buf[1024];
	tline * line;
       
        //variables auxiliares
	int i,j,k,jp,commandsNumber,status,kpid,auxComp;
	pid_t pid;	
        char **firstCommandArguments;

	// Para las redirecciones de entrada, salia y error
	FILE *file;
	int backup_in = dup(fileno(stdin));
	int backup_out = dup(fileno(stdout));
	int backup_err = dup(fileno(stderr));

	// Variables para el manejo del comando cd
	char *defaultDirectory = getenv("HOME");
	char currentDirectory[1024]; // para almacenar el directorio actual
	getcwd(currentDirectory, sizeof(currentDirectory));
	
        //para los pipes
        int *fd;

        //Procesos bg
        pid_t *lista;
        int pidBG;
        char **listaLineas;
        char **listaEstados;
        int *isOn;
        int maxOnBG,indexBG;
        
	//Procesos fg
	pid_t *listaNegra;
        
	//Ignoramos las señales SIGINT y SIGQUIT
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN); 

	
       
        indexBG = 0;
	printf("msh> ");
	lista = (pid_t*)malloc(2*sizeof(pid_t*));
	listaLineas = (char**)malloc(2*sizeof(char**));
        listaEstados = (char**)malloc(2*sizeof(char**));        
	listaNegra = (pid_t*)malloc(2*sizeof(pid_t*));
        isOn = (int*)malloc(2*sizeof(int*));
      
        
	while (fgets(buf, 1024, stdin)) {
		
                
		line = tokenize(buf);

		if (line==NULL) {
			fprintf(stderr, "Vacío\n");
			continue;
		}
                
                //COMPROBAR PROCESOS EN BACKGROUND (TERMINADOS ??)
                
                /*for(k=1;k<=indexBG;k++){
                        isOn[k] = compruebaBGfin(k,lista,listaLineas,listaEstados,isOn[k]);
                        
                }
                
                indexBG = compruebaIndexBG(indexBG,isOn);*/
                




		commandsNumber = line->ncommands;
		
                fd = (int*)malloc((commandsNumber*2)*sizeof(int*));
                lista = (pid_t*)realloc(lista,(indexBG+2)*sizeof(pid_t*));
                listaLineas = (char**)realloc(listaLineas,(indexBG+2)*sizeof(char**));
                listaEstados = (char**)realloc(listaEstados,(indexBG+2)*sizeof(char**));
                isOn = (int*)realloc(isOn,(indexBG+2)*sizeof(int*));                

                if(line->ncommands!=0){
			firstCommandArguments = line->commands[0].argv;
                        //comandos propios
		        if(strcmp(firstCommandArguments[0], "cd")==0){// si el primer comando de los que introduzco es "cd"

				fprintf(stderr,"Arg: %d\n", line->commands[0].argc);
		        	if(line->commands[0].argc < 2){// si no se especifica una ruta, se pone la ruta por defecto ("HOME")
					fprintf(stderr,"Dir: %s\n", defaultDirectory);
					chdir(defaultDirectory);
				}else{// si se especifica la ruta
					fprintf(stderr,"Dir: %s\n", firstCommandArguments[1]);
					chdir(firstCommandArguments[1]);
				}
                        	// cambiamos el valor del directorio actual por el que hemos cambiado (comprobar con pwd)
			        getcwd(currentDirectory, sizeof(currentDirectory));
		        }
			else if(strcmp(firstCommandArguments[0], "jobs")==0){// si el comando es jobs
                               
                                 
                                 for(k=1;k<=indexBG;k++){
                                         
                                         if(isOn[k]!=0){
                                                if(kpid=waitpid(lista[k],&status,WNOHANG)<=0){
                                                        printf("[%d]    %d   %s          %s\n", k, lista[k],listaEstados[k],listaLineas[k]);
                                                
                                                }else{
                                                        printf("[%d]    %d   HECHO (JOBS)         %s\n", k, lista[k],listaLineas[k]);
                                                        isOn[k]=0;
                                                        
                                                }
                                                
                                        }                
                        
                                }
                        }
			else if(strcmp(firstCommandArguments[0], "fg")==0){// si el comando es fg
				if(line->commands[0].argc < 2){// si no se especifica qué proceso se quiere reanudar
					if(indexBG > 0){
                                                printf("%s\n", listaLineas[indexBG]);// se ejecuta el último comando puesto en segundo plano
					        waitpid(lista[indexBG], NULL, 0);
					        // hay que borrarlo de la lista del jobs
					        isOn[indexBG] = 0;
                                                
                                                indexBG--;
                                        }else{

                                           printf("bash : fg :  actual: no existe ese trabajo\n");
                                        }
                                        
				}else{// si se especifica el pid del proceso
					k = atoi(firstCommandArguments[1]);
                                        if (k <= indexBG){
                                                printf("%s\n", listaLineas[k]);
					        // lo pasamos a primer plano (los demás procesos deben esperar por él)
					        waitpid(lista[k], NULL, 0);
					        // hay que borrarlo de la lista del jobs
					        isOn[k] = 0;
                                        }else{
                                                printf("bash : fg :  %d: no existe ese trabajo\n", k);
                                        }
					
				}
			
			//comandos ejecutables
			}else{
                                 
                		// si hay redicrección de entrada (<)
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
				// si hay redirección de salida (>)
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
				// si hay redirección de error (>&)
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


                                                                        





		                // creamos los pipes, el doble de comandos que haya (dos pipes por comando)
                                for (j=0;j<line->ncommands;j++){
                                        if(pipe(fd+j*2)<0){// comando[1] (pipe[1] y pipe[0]), comando[2] (pipe[3] y pipe[2])
                                                fprintf(stderr, "Imposible to pipe().\n%s\n", strerror(errno));
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
						// Si no es el primer comando
                                                if(i!=0){
                                                        // Escribe en el pipe
                                                        if(dup2(fd[jp - 2],0)<0){
                                                                 fprintf(stderr, "Imposible to dup2 no prim.\n%s\n", strerror(errno));                                                          exit(0);                 
                                                        }
                                                        close(fd[jp - 2]); 
				     
                                                }
						// Si no es el último comando
                                                if(i!=line->ncommands-1){
                                                         // Lee del pipe
                                                         if(dup2(fd[jp + 1], 1)<0){
                                                                 fprintf(stderr, "Imposible to dup2 no ult().\n%s\n", strerror(errno));                                                         exit(0);                 
                                                         }
				                       close(fd[jp + 1]);     
                                                }
                                                // Cerramos todos los pipes después de la lectura/escritura en el pipe de cada comando
                                                for (k=0; k<2*line->ncommands;k++){
                                                        close(fd[k]);
                                                }

					     
                                                
                                                
                                                    
					        if((execv(line->commands[i].filename, line->commands[i].argv))<0){
                                                       
                                                	fprintf(stderr, "COMANDO ERRÓNEO .\n%s\n", strerror(errno));
                                                	exit(status);
                                                }
                                               
					        //Si llega aquí es que se ha producido un error en el execvp
					        //printf("Error al ejecutar el comando: \n%s\n", strerror(errno));
					        //exit(status);//regresa al wait del padre 
                                                
		
				            }                              
                                       
                                                



				      	pidBG = pid; // nos sirve si vamos a ejecutar un comando en background

                                	jp+=2;
			        }
			        // Restauramos los ficheros de redirección (cerramos todos los pipes despué de leer todos los comandos)
                                for (k=0; k<2*line->ncommands;k++){
                                        close(fd[k]);
                                }
                                             
                                // Si hay algún comando a ejecutar en background
                                if (line->background != 1) {
                                	for (k=0; k<=line->ncommands;k++)                                            	
                                		wait(&status);
                                	
		                } else {
                                        
                                        indexBG++;        
                                        listaLineas[indexBG] =  (char*)malloc(1024*sizeof(char));
                                        listaEstados[indexBG] = (char*)malloc(1024*sizeof(char));
		                        lista[indexBG] = pidBG;
		                        strcpy(listaLineas[indexBG],strtok(buf,"&"));
                                        isOn[indexBG] = 1;                                                       
                                        strcpy(listaEstados[indexBG],"EJECUTANDO");
                                      		                        
		                        printf("[%d]    %d\n",indexBG,pidBG);
                                                 
		                      
                                }

			        dup2(backup_in, fileno(stdin));
			        dup2(backup_out, fileno(stdout));
			        dup2(backup_err, fileno(stderr));
                              
		     }// fin else (comandos ejecutables)
                     for(k=1;k<=indexBG;k++){
                        isOn[k] = compruebaBGfin(k,lista,listaLineas,listaEstados,isOn[k]);
                        
                        }
                
                indexBG = compruebaIndexBG(indexBG,isOn);   
                }else{// si no hay comandos
                       
                        for(k=1;k<=indexBG;k++){
                        isOn[k] = compruebaBGfin(k,lista,listaLineas,listaEstados,isOn[k]);
                        
                }
                
                indexBG = compruebaIndexBG(indexBG,isOn);
                        printf("msh> ");
                        continue;
                }// fin if(line->ncommands!=0)
	printf("msh> ");	
	}
	return 0;
}
