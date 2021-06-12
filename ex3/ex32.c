// Yael Avioz 207237421

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>
#include <ctype.h>

#define ERROR -1
#define TRUE 1
#define FALSE 0
#define MAXLEN 150

#define NO_C_FILE "0,NO_C_FILE"
#define COMPILATION_ERROR "10,COMPILATION_ERROR"
#define TIMEOUT "20,TIMEOUT"
#define WRONG "50,WRONG"
#define SIMILAR "75,SIMILAR"
#define EXCELLENT "100,EXCELLENT"

typedef struct config{
  char directory[MAXLEN];
  char input_file[MAXLEN];
  char output[MAXLEN];
  int res_fd;
}config;

typedef struct result{
  char name[MAXLEN];
  char gradeAndReason[MAXLEN];
  char toString[MAXLEN];
}result;

void res_to_string(result* result){
  strcpy(result->toString, result->name);
  strcat(result->toString, ",");
  strcat(result->toString, result->gradeAndReason);
  strcat(result->toString, "\0");
}

void write_res(char* user_name, char* gradeAndReason, const config* const config){
  result res;
  strcpy(res.name, user_name);
  strcpy(res.gradeAndReason, gradeAndReason);
  res_to_string(&res);
  write(config->res_fd, res.toString, strlen(res.toString));
  write(config->res_fd, "\n", strlen("\n"));
}

void compare(char* user_name, const config* const config){
  char correctOutput[MAXLEN];
  strcpy(correctOutput, config->output);
  int pid = fork();
  if(pid < 0) 
  {
  exit(ERROR);
  }
  if(pid == 0){
    char *args[] = {"./comp.out", "output.txt", correctOutput, NULL};
    if (execvp(args[0], args) < 0) {
      exit(ERROR);
    }
  } else {
    int status;
    if (waitpid(pid, &status, 0) < 0) {
      return;
    }
    if(WIFEXITED(status)){
      switch(WEXITSTATUS(status)){
        case 1:
          write_res(user_name, WRONG, config);
          break;
        case 2:
          write_res(user_name, SIMILAR, config);
          break;
        case 3:
          write_res(user_name, EXCELLENT, config);
          break;
        default:
          return;
      }
    }
  }
}

void run(char *user_name, const config *const config){
  int pid2 = fork();
  if(pid2 < 0){ exit(1); }
  if(pid2 == 0){
    int outFd;
    int inFd;
    if ((inFd = open(config->input_file, O_RDONLY)) < 0)
    {
      exit(ERROR);
    }
    if ((outFd = open("output.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
    {
      exit(ERROR);
    }
    if (dup2(inFd, 0) < 0) 
    {
      exit(ERROR);
    }
    if (dup2(outFd, 1) < 0) {
      exit(ERROR);
    }
    if (execlp("./compiled.out", "./compiled.out", NULL) < 0){
      exit(ERROR);
    }
  } else {
    int status;
    sleep(5);
    pid_t return_pid = waitpid(pid2, &status, WNOHANG);
    if (return_pid < 0) {
      return;
    } else if (return_pid == 0) {
      write_res(user_name, TIMEOUT, config);
      kill(pid2, SIGKILL);
      return;
    } else if (return_pid == pid2) {
      if(WIFEXITED(status)) {
        compare(user_name, config);
      }
    }
  }
}

void compile(char *filePath, char *user_name, const config *const config){
  int pid1 = fork();
  if(pid1 < 0)
  { 
  exit(ERROR);
   }
  if(pid1 == 0) {
    char *args[] = {"gcc", "-o", "compiled.out", filePath, NULL};
    execvp(args[0], args);
    exit(ERROR);
  } else {
    int returnStatus;
    if (waitpid(pid1, &returnStatus, WCONTINUED) < 0) {
      return;
    }
    if(returnStatus == 0){
      run(user_name, config);
    } else {
      write_res(user_name, COMPILATION_ERROR, config);
      return;
    }
  }
}

const char *GetFileExt(const char *filename) {
  const char *dot = strrchr(filename, '.');
  if(!dot || dot == filename) return "";
  return dot + 1;
}

int IsFileC(struct dirent *pDirent){
  if((pDirent->d_type == DT_REG || pDirent->d_type == DT_UNKNOWN)
          && strcmp(GetFileExt(pDirent->d_name), "c") == 0){
    return TRUE;
  }
}

void sub_dir(char *dirName, char *user_name, const config* const config, int* exist){
  DIR *dir;
  struct dirent *entry;
  if (!(dir = opendir(dirName))) return;
  while ((entry = readdir(dir)) != NULL) {
    if (IsFileC(entry) == TRUE) {
      *exist = TRUE;
      char filePath[MAXLEN];
      memset(filePath, 0, MAXLEN);
      snprintf(filePath, MAXLEN, "%s/%s", dirName, entry->d_name);
      compile(filePath, user_name, config);
      closedir(dir);
      return;
    } 
    else if(entry->d_type == DT_DIR){
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;
      char path[MAXLEN];
      memset(path, 0, MAXLEN);
      snprintf(path, MAXLEN, "%s/%s", dirName, entry->d_name);
      sub_dir(path, user_name, config, exist);
    }
  }
  closedir(dir);
}

int main(int argc, char *argv[])
{
	//validated input
	if (argc != 2) {
        return ERROR;
   }

  //open the file from the given path
  char *path = argv[1];
  int config = open(path,0);
	if (config < 0)
  {
    close(config);
	  return ERROR;
	}
 
  //read the date from the config file to the buffer
  char buffer[MAXLEN+1];
  int read_config =read(config, buffer, MAXLEN);
  if (read_config == -1 )
  {
		return ERROR;
	}
 
  //gets the data from the buffer to the struct Config_Data
  char *line;
  char lines[3][MAXLEN];
  int i = 0;
  line = strtok(buffer, "\n");
  while(line != NULL){
    strcpy(lines[i++], line);
    line = strtok(NULL, "\n");
  }
  strcpy(config->directory, lines[0]);
  strcpy(config->input_file, lines[1]);
  strcpy(config->output, lines[2]);

  if ((config->res_fd = open("results.csv", O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0) exit(1);
  
  //Check the input directory
  DIR *dir;
  struct dirent *entry;
  if (!(dir = opendir(directory)))
    return;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_DIR) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
      char dirPath[MAXLEN];
      
      //connect two strings to one
      memset(dirPath, 0, MAXLEN);
      snprintf(dirPath, MAXLEN, "%s/%s", dirName, entry->d_name);
      
      int exist = FALSE;
      
      sub_dir(dirPath, entry->d_name, config, &exist);  
      if (exist  == FALSE){
      write_res(entry->d_name, NO_C_FILE, config);
      }
    }
  }
  closedir(dir);
}
