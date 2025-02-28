/*********************************************************************
** Program Filename: main.c
** Author: Aiden Reedy
** Date: 05/22/24
** Description: Solution for HW3 for cs374
** Input: depends on how it's used
** Output: depends on how it's used
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_KIDS 9999
static int FOREGROUND_ONLY = 0;  //global variable for SIGSTP
static int bg = 0;              //global variable for a process, it's parent process, and SIGINT handle to see if its a back ground process
static int status = 0;          //global variable for exit / term status of last child to exit / be terminated
static pid_t child_arr[MAX_KIDS]; //global variable for an array to hold child pids for exit command
static int numKids = 0;         //global variable for an int to keep track of how many kids are in the child_arr
static pid_t childPid = -1;     //global variable for the childPid

//struct to hold the command, args, input file etc...
struct command {
    char *comm;
    char **args;
    char *inp;
    char *outp;
    int arg_count;
};

/*********************************************************************
** Function: handle_SIGINT()
** Description: SIGINT handler to have it only kill background children
** Parameters: int signo
** Pre-Conditions: none
** Post-Conditions: none
*********************************************************************/
void handle_SIGINT(int signo) {
    if (childPid == 0 && bg == 0) { //check if it's a child and in the background
        status = signo;
        kill(getpid(), SIGTERM);  //kill it
    }else{
        return;
    }
}

/*********************************************************************
** Function: handle_SIGSTP()
** Description: SIGSTP handler to toggle forground only mode and say so
** Parameters: int signo
** Pre-Conditions: none
** Post-Conditions: flips FOREGROUND_ONLY variable
*********************************************************************/
void handle_SIGTSTP(int signo) {
    if(childPid != 0) {         //if it's the parent process
        if (FOREGROUND_ONLY) {  //if we are already in forground only mode
            printf("Exiting foreground-only mode\n"); //say we are turning it off
            fflush(stdout);
            FOREGROUND_ONLY = 0;        //turn it off
        } else {
            printf("Entering foreground-only mode (& is now ignored)\n"); //otherwise say its starting
            fflush(stdout);
            FOREGROUND_ONLY = 1;    //turn it on
        }
    }
}

/*********************************************************************
** Function: handle_SIGCHLD()
** Description: SIGCHLD handler to have the parent log the exit/ signal that killed the kid
** Parameters: int signo
** Pre-Conditions: none
** Post-Conditions: updates status global variable
*********************************************************************/
void handle_SIGCHLD(int signo) {
    int childStatus;
    pid_t kidPid;
    while ((kidPid = waitpid(-1, &childStatus, WNOHANG)) > 0) { //while loop to wait for a child to die
        if (WIFEXITED(childStatus)) { //if it exited
            printf("Child %d exited with status: %d\n", kidPid, WEXITSTATUS(childStatus)); //say it was an exit
            fflush(stdout);
            status = WEXITSTATUS(childStatus);  //update status
        } else if (WIFSIGNALED(childStatus)) { //if it was a signal
            printf("Child %d was terminated by signal %d\n", kidPid, WTERMSIG(childStatus)); //say it was a signal
            fflush(stdout);
            status = WTERMSIG(childStatus); //update status
        }
    }
    numKids--; //since someone died lower the amount of kids
    child_arr[numKids] = 0;  //set the lst
}

/*********************************************************************
** Function: parse()
** Description: fills out the command struct with the info from the input
** Parameters: char* input: the input from the user
** Pre-Conditions: none
** Post-Conditions: none
*********************************************************************/
struct command* parse(char* input) {
    struct command *cmd = malloc(sizeof(struct command));   //allocate memory for the struct
    if (!cmd) {
        perror("malloc");
        exit(1);
    }

    //make them all null to be safe
    cmd->comm = NULL;
    cmd->args = NULL;
    cmd->inp = NULL;
    cmd->outp = NULL;
    cmd->arg_count = 0;


    char* backup = strdup(input); //make a backup
    int i = 0;
    while(strstr(input, "$$")){ //while there is "$$" in the command
        while (backup[i]) {  //while we aren't at the last index of the backup
            if (backup[i] == '$' && backup[i + 1] == '$') {   //check if there's two $ next to each other
                //replace them with %d
                backup[i] = '%';
                backup[i + 1] = 'd';
                sprintf(input, backup, getpid()); //replace %d with the pid
            }
            i++; //go up another index
        }
    }
    free(backup);   //free the backup

    char *token = strtok(input, " ");   //start tokenizing
    cmd->comm = strdup(token);      //make the first token the command since it always is

    //while we aren't at the end of the input
    i = 0;
    while (token) {
        i++;
        //if there is a "<" (input redirection)
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");  //strtok again to get the file name
            if (token) { //check to make sure it wasn't the end of the file
                cmd->inp = strdup(token); //update the input in the struct
            }

            //same for output
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token) {
                cmd->outp = strdup(token);
            }

            //if its a &
        } else if (strcmp(token, "&") == 0) {
            bg = i; //make bg = i (we'll check later to see if i changed (ran the loop again))

            //otherwise its an argument
        } else {
            cmd->args = realloc(cmd->args, sizeof(char *) * (cmd->arg_count + 1)); //make it one arg bigger
            if (!cmd->args) {
                perror("realloc");
                exit(1);
            }
            cmd->args[cmd->arg_count] = strdup(token); //add the new argument
            if (!cmd->args[cmd->arg_count]) {
                perror("strdup");
                exit(1);
            }
            cmd->arg_count++;   //increase the count of arguments
        }
        token = strtok(NULL, " ");  //grab the next token
    }

    //check to see if i hasn't changed (if it did that means there was another token after the &)
    if(i == bg){
        bg = 1; //if its the same then make it a background process
    }else{
        bg = 0; //otherwise its still a foreground process
    }

    //update args once again to add the null at the end for execv()
    cmd->args = realloc(cmd->args, sizeof(char *) * (cmd->arg_count + 1));
    if (!cmd->args) {
        perror("realloc");
        exit(1);
    }
    cmd->args[cmd->arg_count] = NULL;

    if (FOREGROUND_ONLY) { //if we are in foreground only mode then change it back to a foregound process
        bg = 0;
    }

    return cmd;
}

/*********************************************************************
** Function: free_command()
** Description: free's the command struct
** Parameters: struct command *cmd: the command struct we are freeing
** Pre-Conditions: none
** Post-Conditions: none
*********************************************************************/
void free_command(struct command *cmd) {
    if (cmd) {  //check if it exists
        if(cmd->comm){ //if theres a command in it
            free(cmd->comm); //free it
        }
        if (cmd->args) { //if there are arguments
            int i = 0;
            for (i = 0; i < cmd->arg_count; i++) {  //free all of them
                free(cmd->args[i]);
            }
            free(cmd->args);    //free args itself
        }
        if(cmd->inp){ //if theres an input file
            free(cmd->inp); //free it
        }
        if(cmd->outp){ //if theres an output file
            free(cmd->outp);    //free it
        }
        free(cmd);  //free the struct
    }
}

/*********************************************************************
** Function: execute_command()
** Description: makes a child to use execv() and makes the parent wait if its a foreground process
** Parameters: struct command *cmd: the command struct we are freeing
** Pre-Conditions: none
** Post-Conditions: none
*********************************************************************/
void execute_command(struct command *cmd) {
    numKids++;  //we are making a child so increase the count of them
    childPid = fork();  //make the child

    switch (childPid) {
        //if something went wrong say so;
        case -1:
            perror("fork()");
            status = 1;
            break;

            //for the child
        case 0:

            if (cmd->inp) { //if theres an input redirection
                int new_in = open(cmd->inp, O_RDONLY); //open the new input
                if (new_in == -1) {
                    perror("open()");
                    exit(1);
                }
                dup2(new_in, 0); //make it stdin
                close(new_in); //close the new fd
            }

            if (cmd->outp) { //same as inout just with different flags and makes it stdout
                int new_out = open(cmd->outp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (new_out == -1) {
                    perror("open()");
                    exit(1);
                }
                dup2(new_out, 1);
                close(new_out);
            }

            if (execvp(cmd->comm, cmd->args) == -1) { //calls execvp with our command and args
                perror("execvp()");
                exit(1);
            }
            break;

        default: //for the parent
            child_arr[numKids-1] = childPid; //add the new pid to the array
            if (bg == 0) {  //if its a foreground process
                waitpid(childPid, &status, 0);  //wait for it to be done
            } else {
                printf("background pid is %d\n", childPid); //otherwise say its pid
                fflush(stdout);
            }
            break;
    }
}

/*********************************************************************
** Function: process()
** Description: takes the filled out cmd struct and runs the command (or passes it to execv)
** Parameters: struct command *cmd: the command struct we are running
** Pre-Conditions: none
** Post-Conditions: none
*********************************************************************/
void process(struct command *cmd) {
    if (strcmp(cmd->comm, "exit") == 0) {//if the command is exit
        int i = 0;
        for (i = 0; i < numKids; i++) { //kill all the children
            kill(child_arr[i], SIGTERM);
        }
        free_command(cmd);
        exit(0);    //exit the shell

    } else if (strcmp(cmd->comm, "cd") == 0) { //if it says cd
        if (cmd->args[1] == NULL) { //if there's no arguments other than the command
            chdir(getenv("HOME")); //go to home
        } else {
            if (chdir(cmd->args[1]) != 0) { //otherwise go to the desired directory
                perror("chdir()");
            }
        }

    } else if (strcmp(cmd->comm, "status") == 0) { //if the command says status

        if (WIFEXITED(status)) { //if the last child exited
            printf("Exit status: %d\n", WEXITSTATUS(status)); //print that
            fflush(stdout);

        } else if (WIFSIGNALED(status)) { //if the last child was singled to terminate
            printf("Terminated by signal: %d\n", WTERMSIG(status)); //say so
            fflush(stdout);
        }
    } else {
        execute_command(cmd); //otherwise call execute_command() to pass it to execv()
    }
}

int main() {
    //assign all needed signals their handler
    struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0}, SIGCHLD_action = {0};
    SIGINT_action.sa_handler = handle_SIGINT;
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    SIGCHLD_action.sa_handler = handle_SIGCHLD;

    sigaction(SIGINT, &SIGINT_action, NULL);
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
    sigaction(SIGCHLD, &SIGCHLD_action, NULL);

    char input[2048]; //make the input variable
    printf("$ smallsh\n");  //print the "intro"
    fflush(stdout);
    while (1) { //forever loop
        bg = 0; //reset bg to be safe
        printf(": "); //prompt for an iput
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) != NULL) { //get the input
            input[strcspn(input, "\n")] = '\0';  // take out the \n
            if (strlen(input) == 0 || input[0] == '#') { //if its a comment
                continue;  //resart the loop from the top
            }
            struct command *cmd = parse(input); //get the filled out struct
            process(cmd); //process it
            free_command(cmd); //free it
        } else { //if there was an error
            clearerr(stdin); //clear the errors in stdin
        }

    }
    return 0;
}
