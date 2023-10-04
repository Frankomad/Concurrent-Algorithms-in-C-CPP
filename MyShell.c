/*
This code implements a basic Unix-like shell, "MyShell," that lets users execute commands,
change directories, and exit. It includes signal handling for Ctrl+C (SIGINT) to
terminate running commands. The shell parses user input, handles built-in commands like "cd" and "exit,"
and executes external commands by searching for them in the system's directories. Error handling is in
 place for command not found and directory non-existence.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define MAX_P_L 1024
pid_t child_pid;
char *variable_path(char *command);
void handle_sigint(int sig)
{
    if (child_pid > 0)
    {
        kill(child_pid, SIGKILL);
        wait(NULL);
    }
    else
    {
        printf("\nMyShell>");
    }
}

int main()
{
    char inputString[100];
    char *command[50];

    struct sigaction act;
    act.sa_handler = handle_sigint;
    sigaction(SIGINT, &act, NULL);

    while (1)
    {

        printf("MyShell> ");
        child_pid = -1;
        fgets(inputString, 100, stdin);
        if (strlen(inputString) == 1)
        {
            exit(0);
        }
        inputString[strcspn(inputString, "\n")] = 0;
        char *spliter;
        spliter = strtok(inputString, " ");
        int numOfCommands = 0;
        while (spliter != NULL)
        {
            command[numOfCommands++] = spliter;
            spliter = strtok(NULL, " ");
        }
        command[numOfCommands] = NULL;
        char *sadrzi1 = strchr(command[0], '/');
        char *sadrzi2 = strchr(command[0], '.');

        if (numOfCommands == 0)
        {
            printf("zajeb");
            continue;
        }
        else if (command[0] != NULL && strcmp(command[0], "cd") != 0 && strcmp(command[0], "exit") != 0)
        {
            if (sadrzi1 != NULL || sadrzi2 != NULL)
            {
                pid_t pid = child_pid = fork();
                if (pid == -1)
                {
                    printf("Error");
                    exit(1);
                }
                else if (pid == 0)
                {
                    setpgid(0, 0);
                    int returnValue = execve(command[0], command, NULL);
                    if (returnValue == -1)
                    {
                        printf("MyShell: Unknown command: %s\n", command[0]);
                        exit(1);
                    }
                }
                else
                {
                    int status;
                    sigset_t sigset;
                    waitpid(pid, &status, 0);
                }
            }
            else
            {
                char *pathOfCommand = variable_path(command[0]);
                if (pathOfCommand == NULL)
                {
                    printf("MyShell: Unknown command: %s\n", command[0]);
                }
                else
                {
                    pid_t pid = child_pid = fork();
                    if (pid == -1)
                    {
                        printf("Error");
                        exit(1);
                    }
                    else if (pid == 0)
                    {
                        int returnValue;
                        setpgid(0, 0);

                        returnValue = execve(pathOfCommand, command, NULL);

                        if (returnValue == -1)
                        {
                            printf("MyShell: Unknown command: %s\n", command[0]);
                            exit(1);
                        }
                    }
                    else
                    {
                        int status;
                        waitpid(pid, &status, 0);
                    }
                    free(pathOfCommand);
                }
            }
        }
        else if (strcmp(command[0], "cd") == 0)
        {
            int returnValue = chdir(command[1]);
            if (returnValue == -1)
            {
                printf("cd: The directory %s does not exist\n", command[1]);
            }
        }
        else if ((strcmp(command[0], "exit") == 0))
        {
            exit(0);
        }
    }

    return 0;
}

char *variable_path(char *command)
{
    char *content = getenv("PATH");
    if (content == NULL)
    {
        return NULL;
    }

    char *contentCopy = strdup(content);
    char *path = strtok(contentCopy, ":");
    while (path != NULL)
    {
        char pathOfCommand[MAX_P_L];
        snprintf(pathOfCommand, MAX_P_L, "%s/%s", path, command);
        if (access(pathOfCommand, X_OK) == 0)
        {
            free(contentCopy);
            return strdup(pathOfCommand);
        }
        path = strtok(NULL, ":");
    }
    free(contentCopy);
    return NULL;
}
