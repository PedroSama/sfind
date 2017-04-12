#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>

//decidi ir por uma struct se quiseres mudar depois força
struct Flags{

  char* name;
  char* type;
  int mode;

  int print;
  int delete;
  
  //falta o exec

} flags;

//para a interrupção do CTRL C
//aceita o CTRL C a meio do processo mas não faz o que devia
void sigint_handler(int signo){

  char terminate;

  kill(getppid(),SIGSTOP);

  printf("\n> Are you sure you want to terminate(Y/N)? \n");
  scanf("%s",&terminate);

  terminate = (char)toupper(terminate);

  if(terminate == 'N'){
    kill(getppid(),SIGCONT);
    
  }
  else{
    kill(getppid(),SIGQUIT);
    kill(getpid(),SIGQUIT);
  }

}

void subscribe_SIGINT(){

  struct sigaction act;
  act.sa_handler = sigint_handler;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);

  sigaction(SIGINT,&act,NULL);

}


//processa os argumentos da linha de comandos
void processArgs(int argc, char* argv[]){

  flags.name = "";
  flags.type = "";
  flags.mode = 0;
  flags.print = 0;
  flags.delete = 0;

  int i = 0;

  for(i; i < argc; i++){

    if(strcmp(argv[i],"-name") == 0)
      flags.name = argv[i+1];

    if(strcmp(argv[i],"-type") == 0)
      flags.type = argv[i+1];

    if(strcmp(argv[i],"-perm") == 0)
      flags.mode = strtoul(argv[i+1],NULL,8);

    if(strcmp(argv[i],"-print") == 0)
      flags.print = 1;

    if(strcmp(argv[i],"-delete") == 0)
      flags.delete = 1;
  }
//falta o tratamento para o exec aqui também
}

// -name funciona
// -type funciona (falta processamento de ligações simbólicas (-l)
// -print funciona (apesar de imprimir mais vezes do que é suposto

//falta -perm -delete e -exec


void searchDir(char* path){

  struct stat fileStatus;
  struct dirent *directory;
  DIR* dp;
  pid_t pid;

  subscribe_SIGINT(); //ctrl+C interruption
  sleep(4);

  if(stat(path,&fileStatus) != 0)
    printf("Error status\n");

  if(S_ISDIR(fileStatus.st_mode)){

    if((dp = opendir(path)) == NULL)
      printf("Error opendir\n");

    char newPath[256];

    while((directory = readdir(dp)) != NULL){

      if(strcmp(directory->d_name,".") == 0 || strcmp(directory->d_name, "..") == 0){
        continue;
      }

      strcpy(newPath,path);
      strcat(newPath,"/");
      strcat(newPath,directory->d_name);
      stat(newPath,&fileStatus);

      if(S_ISREG(fileStatus.st_mode)){

        if(flags.print == 1){
          if(*(flags.type) == 'f'){
            printf("%s\n", newPath);
          }
          else{
            if(strcmp(flags.name, directory->d_name) == 0){
                printf("%s\n",newPath);
            }
          }
        }
      }

      if(S_ISDIR(fileStatus.st_mode)){

        if(flags.print == 1){
          if(*(flags.type) == 'd')
            printf("%s\n", newPath);
        }

        switch(pid = fork()){
          case -1:
            printf("Error fork()...\n");
            break;

          case 0: //child process
            searchDir(newPath);
            break;

        }
      }
    }
    closedir(dp);
  }
}

int main(int argc, char* argv[]){

  char* path = getcwd(NULL,256);

  processArgs(argc,argv);

  printf("Starting on path:  %s\n\n", path);
  searchDir(path);

  return 0;
}


//help from http://stackoverflow.com/questions/4989431/how-to-use-s-isreg-and-s-isdir-posix-macros and similars.
