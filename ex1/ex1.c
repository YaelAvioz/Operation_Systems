#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <libgen.h>

#define TRUE 1
#define FALSE 0
#define MAXLEN 100

struct JobInfo
{
    char job_name[MAXLEN];
    char is_background;
    pid_t pid;
} typedef JobInfo;

int initialize_command(JobInfo jobs_info[], int curr_job_index, char input[], char *command_args[])
{
    char *argument;
    int args_count = -1;

    int input_length = strlen(input);

    if (input[input_length - 1] == '&')
    {
        input[input_length - 2] = '\0';
        jobs_info[curr_job_index].is_background = TRUE;
    }

    strcpy(jobs_info[curr_job_index].job_name, input);

    argument = strtok(input, " ");
	int i = 0;
    for (i; argument != NULL; i++)
    {
        command_args[i] = argument;
        argument = strtok(NULL, " ");
        args_count++;
    }
    command_args[args_count + 1] = NULL;

    return args_count;
}

void change_directory(char *new_path, char *prev_path, char *curr_wd)
{
    int chdir_result = chdir(new_path);

    if (chdir_result == -1)
    {
        printf("chdir failed\n");
        fflush(stdout);
    }
    else
    {
        strcpy(prev_path, curr_wd);
    }
}

void remove_apostrophes(char *str)
{
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++)
    {
        *dst = *src;
        if (*dst != '"')
        {
            dst++;
        }
    }
    *dst = '\0';
}

char is_running(pid_t pid)
{
    if (waitpid(pid, NULL, WNOHANG) == 0)
    {
        return TRUE;
    }
    return FALSE;
}

void cd(int args_count, char prev_path[], char *command_args[])
{
    char modified_path[MAXLEN];
    char current_working_dir[MAXLEN];

    getcwd(current_working_dir, MAXLEN);
    if (args_count > 1)
    {
        printf("Too many arguments\n");
        fflush(stdout);
    }
    else if (args_count == 0)
    {
        change_directory(getenv("HOME"), prev_path, current_working_dir);
    }
    else
    {
        if (strcmp(command_args[1], "-") == 0)
        {
            change_directory(prev_path, prev_path, current_working_dir);
        }
        else
        {
            if (strncmp(command_args[1], "~", 1) == 0)
            {
                strcpy(modified_path, getenv("HOME"));
                strcat(modified_path, command_args[1] + 1);
                change_directory(modified_path, prev_path, current_working_dir);
            }
            else
            {
                change_directory(command_args[1], prev_path, current_working_dir);
            }
        }
    }
}

void echo(int args_count, char *command_args[])
{
    char modified_string[MAXLEN];

	int i = 1;
    for (i; i < args_count; i++)
    {
        strcpy(modified_string, command_args[i]);
        remove_apostrophes(modified_string);
        printf("%s ", modified_string);
    }
    strcpy(modified_string, command_args[args_count]);
    remove_apostrophes(modified_string);
    printf("%s\n", modified_string);
    fflush(stdout);
}

void history(int jobs_count, JobInfo jobs_info[])
{
	int i = 0;
    for (i; i < jobs_count; i++)
    {
        printf("%s", jobs_info[i].job_name);
        if (jobs_info[i].is_background && is_running(jobs_info[i].pid))
        {
            printf(" RUNNING\n");
        }
        else
        {
            printf(" DONE\n");
        }
        fflush(stdout);
    }
    printf("%s RUNNING\n", jobs_info[jobs_count].job_name);
}

void jobs(int jobs_count, JobInfo jobs_info[])
{
	int j = 0;
    for (j; j <= jobs_count; j++)
    {
        if (jobs_info[j].is_background == TRUE)
        {
            printf("%s\n", jobs_info[j].job_name);
            fflush(stdout);
        }
    }
}

void exeternal_command(char command[], JobInfo jobs_info[], int curr_job_index, char *command_args[])
{
    int exec_result;
    pid_t pid;

    pid = fork();

    if (pid < 0)
    {
        printf("fork failed\n");
    }
    if (pid == 0)
    {
        exec_result = execvp(command, command_args);
        if (exec_result == -1)
        {
            printf("exec failed\n");
            exit(-1);
        }
    }
    else
    {
        if (jobs_info[curr_job_index].is_background)
        {
            jobs_info[curr_job_index].pid = pid;
        }
        else
        {
            waitpid(pid, NULL, 0);
        }
    }
}

void execute_single_command(JobInfo jobs_info[], int curr_job_index, char prev_path[])
{
    char input[MAXLEN];
    int args_count;
    char *command_args[MAXLEN];

    printf("$ ");
    fflush(stdout);
    scanf(" %[^\n]s", input);

    args_count = initialize_command(jobs_info, curr_job_index, input, command_args);

    if (strcmp(command_args[0], "cd") == 0)
    {
        cd(args_count, prev_path, command_args);
    }
    else if (strcmp(command_args[0], "echo") == 0)
    {
        echo(args_count, command_args);
    }
    else if (strcmp(command_args[0], "exit") == 0)
    {
        exit(0);
    }
    else if (strcmp(command_args[0], "history") == 0)
    {
        history(curr_job_index, jobs_info);
    }
    else if (strcmp(command_args[0], "jobs") == 0)
    {
        jobs(curr_job_index, jobs_info);
    }
    else
    {
        exeternal_command(command_args[0], jobs_info, curr_job_index, command_args);
    }
}

int main()
{
    JobInfo jobs_info[MAXLEN];
    char prev_path[MAXLEN];
    int jobs_count = 0;

    while (TRUE)
    {
        execute_single_command(jobs_info, jobs_count, prev_path);
        jobs_count++;
    }

    return 0;
}