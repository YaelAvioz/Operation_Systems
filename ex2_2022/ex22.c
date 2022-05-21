// Yael Avioz 207237421

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#define NO_C_FILE "0,NO_C_FILE"
#define COMPILATION_ERROR "10,COMPILATION_ERROR"
#define TIMEOUT "20,TIMEOUT"
#define WRONG "50,WRONG"
#define SIMILAR "75,SIMILAR"
#define EXCELLENT "100,EXCELLENT"
#define MAXLEN 150

// write the result
void write_output(char *user_name, char *gradeAndReason, int results_file)
{
    char result[MAXLEN];
    memset(result, 0, MAXLEN);
    snprintf(result, MAXLEN, "%s,%s\0", user_name, gradeAndReason);
    write(results_file, result, strlen(result));
    write(results_file, "\n", strlen("\n"));
}

void compile(char *filePath, char *user_name, char *input, char *output, int results_file)
{
    int pid1 = fork();

    // in case there was a failiure
    if (pid1 < 0)
    {
        exit(-1);
    }

    // child case
    if (pid1 == 0)
    {
        char *args[] = {"gcc", "-o", "compiled.out", filePath, NULL};
        execvp(args[0], args);
        exit(-1);
    }

    // main process
    else
    {
        int compile_res;
        pid_t waitpid1_res = waitpid(pid1, &compile_res, WCONTINUED);

        // in case there was a failiure
        if (waitpid1_res < 0)
        {
            return;
        }

        if (compile_res == 0)
        {
            int pid2 = fork();

            // in case there was a failiure
            if (pid2 < 0)
            {
                exit(1);
            }

            if (pid2 == 0)
            {
                int input_fd = open(input, O_RDONLY);
                int output_fd = open("output.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);

                if ((input_fd < 0) || (output_fd < 0) || (dup2(input_fd, 0) < 0) || (dup2(output_fd, 1) < 0) || (execlp("./compiled.out", "./compiled.out", NULL) < 0))
                {
                    exit(-1);
                }
            }
            else
            {
                int status;
                sleep(5);
                pid_t waitpid2_res = waitpid(pid2, &status, WNOHANG);

                // in case there was a failiure
                if (waitpid2_res < 0)
                {
                    return;
                }

                else if (waitpid2_res == 0)
                {
                    write_output(user_name, TIMEOUT, results_file);
                    kill(pid2, SIGKILL);
                    return;
                }

                else if (waitpid2_res == pid2)
                {
                    if (WIFEXITED(status))
                    {
                        char correctOutput[MAXLEN];
                        strcpy(correctOutput, output);
                        int pid = fork();

                        // in case there was a failiure
                        if (pid < 0)
                        {
                            exit(-1);
                        }

                        if (pid == 0)
                        {
                            char *args[] = {"./comp.out", "output.txt", correctOutput, NULL};

                            // in case there was a failiure
                            if (execvp(args[0], args) < 0)
                            {
                                exit(-1);
                            }
                        }

                        else
                        {
                            int status;
                            if (waitpid(pid, &status, 0) < 0)
                            {
                                return;
                            }

                            // returns true if the child terminated normally
                            if (WIFEXITED(status))
                            {
                                // returns the exit status of the child
                                int child_status = WEXITSTATUS(status);

                                // equal
                                if (child_status == 1)
                                {
                                    write_output(user_name, EXCELLENT, results_file);
                                }

                                // diffrant
                                else if (child_status == 2)
                                {
                                    write_output(user_name, WRONG, results_file);
                                }

                                // similar
                                else if (child_status == 3)
                                {
                                    write_output(user_name, SIMILAR, results_file);
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            write_output(user_name, COMPILATION_ERROR, results_file);
            return;
        }
    }
}

int main(int argc, char *argv[])
{
    // validated input
    if (argc != 2)
    {
        return -1;
    }

    // open the file
    int file = open(argv[1], O_RDONLY);

    // in case there was a failiure
    if (file < 0)
    {
        return -1;
    }

    // create buffer
    char buffer[MAXLEN + 1];

    // read the data to the buffer
    int read_file = read(file, buffer, MAXLEN);

    // in case there was a failiure
    if (read_file == -1)
    {
        return -1;
    }

    // create the errors file
    int errors_file = open("errors.txt", O_WRONLY | O_CREAT, 0666);

    // in case there was a failiure
    if (errors_file < 0)
    {
        return -1;
    }

    dup2(errors_file, STDERR_FILENO);
    close(errors_file);

    // initalized the data from buffer
    char directory[MAXLEN];
    char input[MAXLEN];
    char output[MAXLEN];

    char *line;

    line = strtok(buffer, "\n");
    strcpy(directory, line);

    line = strtok(NULL, "\n");
    strcpy(input, line);

    line = strtok(NULL, "\n");
    strcpy(output, line);

    // create the results file
    int results_file = open("results.csv", O_CREAT | O_TRUNC | O_WRONLY, 0644);

    // in case there was a failiure
    if (results_file < 0)
    {
        return -1;
    }

    // open directory
    DIR *dir_fd;
    dir_fd = opendir(directory);

    // in case there was a failiure
    if (dir_fd < 0)
    {
        return -1;
    }

    // iterate over the directory entries
    struct dirent *entry;
    for (; entry = readdir(dir_fd); entry != NULL)
    {
        // skip non-directry entries
        if (entry->d_type != DT_DIR)
        {
            continue;
        }

        // skip . entry
        if (strcmp(entry->d_name, ".") == 0)
        {
            continue;
        }

        // skip .. entry
        if (strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // get subdir full path
        char dirPath[MAXLEN];
        memset(dirPath, 0, MAXLEN);
        snprintf(dirPath, MAXLEN, "%s/%s", directory, entry->d_name);

        DIR *subdir_fd;
        subdir_fd = opendir(dirPath);

        // in case there was a failiure
        if (subdir_fd < 0)
        {
            return -1;
        }

        int is_found = 0;
        struct dirent *subdir_entry;

        for (; subdir_entry = readdir(subdir_fd); subdir_entry != NULL)
        {
            if ((subdir_entry->d_type == DT_REG) || (subdir_entry->d_type == DT_UNKNOWN))
            {
                // checks if the file ends with .c
                char *dot;
                dot = strrchr(subdir_entry->d_name, '.');
                if ((!(!dot || dot == subdir_entry->d_name)) && (strcmp((dot + 1), "c") == 0))
                {
                    // get subdir full path
                    char filePath[MAXLEN];
                    memset(filePath, 0, MAXLEN);
                    snprintf(filePath, MAXLEN, "%s/%s", dirPath, subdir_entry->d_name);
                    compile(filePath, entry->d_name, input, output, results_file);
                    is_found = 1;
                    break;
                }
            }
        }
        closedir(subdir_fd);

        if (is_found == 0)
        {
            write_output(entry->d_name, NO_C_FILE, results_file);
        }
    }

    unlink("output.txt");
    unlink("compiled.out");
    closedir(dir_fd);
}