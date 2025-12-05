#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAXLINE 8192
#define MAXARGS 128

/* Function Prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

extern char **environ; // Defined by libc

int main(){
    char cmdline[MAXLINE];
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    while(1){
        printf("%% ");
        fgets(cmdline, MAXLINE, stdin);
        if(feof(stdin)){
            exit(0);
        }
        eval(cmdline);
    }
}

/* eval */
void eval(char *cmdline){
    char *argv[MAXARGS];// Argument list execve()
    char buf[MAXLINE];  // Holds modified command line
    int bg;             // Should the job run in bg or fg?
    pid_t pid;          // Process id

    strcpy(buf, cmdline);
    bg = parseline(buf, argv); // 명령어 라인에 &가 있으면 bg=1, 아니면 0
    if(argv[0] == NULL){
        return;
    }
    if(!builtin_command(argv)){
        if((pid = fork()) == 0){
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            if(execvp(argv[0], argv) < 0){
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }
        if(!bg){
            int status;
            if(waitpid(pid, &status, 0) < 0){ // 백그라운드 작업이 아닐경우, wait으로 child 실행이 끝날 때까지 기다려준다.
                perror("waitfg: waitpid error");
            }
        }
        else{
            printf("%d %s", pid, cmdline);
        }
    }
}

/* builtin - */
int builtin_command(char **argv){
    if(!strcmp(argv[0], "quit") || !strcmp(argv[0], "exit") || !strcmp(argv[0], "logout")){
        exit(0);
    }
    if(!strcmp(argv[0], "cd")){
        char *path = argv[1];
        if(path == NULL){
            path = getenv("HOME");
            chdir(path);
        }
        if(chdir(path) < 0){
            perror("cd_error");
        }
        return 1;
    }
    if(!strcmp(argv[0], "&")){
        return 1;
    }
    return 0;
}

/* Parseline -  */
int parseline(char *buf, char **argv){
    char *delim;
    int argc, bg;

    buf[strlen(buf)-1] = ' ';
    while(*buf && (*buf == ' ')){
        buf++;
    }

    argc = 0;
    while((delim = strchr(buf, ' '))){
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while(*buf && (*buf == ' ')){
            buf++;
        }
        
    }
    argv[argc] = NULL;
    if(argc == 0){
        return 1;
    }
    if((bg = (*argv[argc-1] == '&') )!= 0){
        argv[--argc] = NULL;
    }
    return bg;
}
