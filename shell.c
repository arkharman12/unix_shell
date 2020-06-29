// required header files
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>


// constant size
#define CMD_HISTORY_SIZE 10

// declare the variables
char *history_cmd[CMD_HISTORY_SIZE];
int history_count = 0;

// execute each command entered
static void execute_command(const char * inputline) {
      char * command = strdup(inputline);
      char *args[10];
      int argc = 0;
      char* token = strtok(command, "|");
      args[argc++] = strtok(command, " ");
      while (args[argc - 1] != NULL) {
            args[argc++] = strtok(NULL, " ");
      }

      argc--;
      int background = 0;
      // parent process wait for the child to exit
      if (strcmp(args[argc - 1], "&") == 0) {
            background = 1;
            args[--argc] = NULL;
      }

      int fd[2] = { -1, -1 };
      // input/ouput redirection
	  char pi[] = ">";
      while (argc >= 3) {
            if (strcmp(args[argc - 2], ">") == 0) {
                fd[1] = open(args[argc - 1], O_CREAT | O_WRONLY |
                    O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
                if (fd[1] == -1) {
                    perror("open");
                    free(command);
                    return;
                }
                args[argc - 2] = NULL;
                argc -= 2;

            }
            else if (strcmp(args[argc - 2], "<") == 0) {
                fd[0] = open(args[argc - 1], O_RDONLY);
                if (fd[0] == -1) {
                    perror("open");
                    free(command);
                    return;
                }
                args[argc - 2] = NULL;
                argc -= 2;
            }
	    // else if(memchr(inputline, '|', sizeof(inputline))){
		// Read to pipe
	        else if (strcmp(args[argc - 2], "|") == 0) {
                while (token != NULL){
			    printf("%s\n", token);
			    token = strtok(NULL,"|");
			    return;
                }
            }
            else {
                  break;
            }
    }

    /*
        printf("%d\n", argc);
        if(memchr(args, '|', sizeof(args))){
            printf("PIPE MAN");

    }
    */

    int status;
    // child process for executing commands
    pid_t pid = fork();
    switch (pid) {
    case -1:
        perror("fork");
        break;
    case 0:
        if (fd[0] != -1) {
            if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO) {
                perror("dup2");
                exit(1);
            }
        }
        if (fd[1] != -1) {
            if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO) {
                perror("dup2");
                exit(1);
            }
        }
        execvp(args[0], args);
        perror("execvp");
        exit(0);
      default:
            close(fd[0]); close(fd[1]);
            if (!background)
                waitpid(pid, &status, 0);
                break;
    }
    free(command);
}

// add the command to the history after each run
static void add_to_history(const char * command) {
      if (history_count == (CMD_HISTORY_SIZE - 1)) {
            int i;
            free(history_cmd[0]);
            for (i = 1; i < history_count; i++)
                  history_cmd[i - 1] = history_cmd[i];
            history_count--;
      }
      history_cmd[history_count++] = strdup(command);
}

// run each command from the history
static void find_in_history(const char * command) {
      int count = 0;
      // if there are no commands in the history
      if (history_count == 0) {
            printf("No commands in history\n");
            return;
      }
      if (command[1] == '!')
            count = history_count - 1;
      else {
            count = atoi(&command[1]) - 1;
            if ((count < 0) || (count > history_count)) {
                  fprintf(stderr, "No such command in history.\n");
                  return;
            }
      }
      printf("%s\n", command);
      execute_command(history_cmd[count]);
}

// list of command history
static void list_history() {
      int i;
      for (i = history_count - 1; i >= 0; i--) {
            printf("%i %s\n", i + 1, history_cmd[i]);
      }
}


/*

static void runPipe (const char* args) {

pid_t pid;
int fpipe[2];


// create pipe
if (pipe(fpipe)) {
	fprintf(stderr, "Pipe failed \n");
	exit(EXIT_FAILURE);

}

//pid = fork();

if (fork()==0) {
	
	dup2(fpipe[1], STDOUT_FILENO);
	close(fpipe[0]);
	close(fpipe[1]);
	
	execvp(args[0],args );
	exit(1);
}

if (fork() == 0){
	dup2(fpipe[0], STDIN_FILENO);
	close(fpipe[1]);
	close(fpipe[0]);
	
	execvp(args, args+k+1);
	exit(1);
}

close (fpipe[0]);
close (fpipe[1]);
wait(0); // first child
wait(0); // second child

}

*/

// driver or main method
int main(int argc, char *argv[]) {
    size_t line_size = 100;
    char * line = (char*)malloc(sizeof(char)*line_size);
    // char *options[1] = {"|"};
    // int k = 0, option, found;
    // char * command = strdup(line);
    // char* token = strtok(command, "|");
    // char* token = strtok(line, "|");
    // printf("\n Token:  %s",token);
    if (line == NULL) {
        perror("malloc");
        return 1;
    }
    int flag = 0;
    int should_run = 1;
    while (should_run) {
        if (!flag)
            printf("CSCI403 > ");
            fflush(stdout);
        if (getline(&line, &line_size, stdin) == -1) {
            if (errno == EINTR) {
                clearerr(stdin);
                flag = 1;
                continue;
            }
            perror("getline");
            break;
        }
        flag = 0;
        int line_len = strlen(line);
        if (line_len == 1) {
            continue;
        }
        line[line_len - 1] = '\0';
        if (strcmp(line, "exit") == 0) {
            break;
        }
        else if (strcmp(line, "history") == 0) {
            list_history();
        }
        else if (line[0] == '!') {
            find_in_history(line);
        }
        else if (line[0] == '>'){
		printf("Left");
	    }
        /*
        else if(memchr(line, '|', sizeof(line))){
     	char * command2 = strdup(line);
		char* token2 = strtok(command2, "|");

		while (token2 != NULL){
			printf(" YO TOEKN: %s\n", token2);
			token = strtok(NULL,"|");

			break;
		}

	    }*/
        else {
	    /*
		k =1;
		while(line[k] != NULL){
			if(option = 0; option < 1; option++){
				if(strcmp(args[k],options[option]) == 0)
				break;
			}
		
		if(option < 1){
			found =1;
			if(option == 0){
				prinf("YES");
			}
		}
		k++;
	    }*/
	
			
            add_to_history(line);
		    // printf("THE LINE IS: %s\n",line);
            execute_command(line);
	        // runPipe(line);
            }
      }
      free(line);
      return 0;
}

