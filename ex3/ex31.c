// Yael Avioz 207237421

#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>

#define ERROR -1
#define EQUAL 1
#define DIFFERENT 2
#define SIMILAR 3
#define TRUE 1
#define FALSE 0
#define MAXLEN 150

int is_equal(int file1, int file2, long len)
{
   int i;
	 char buffer_file1[MAXLEN+1];
   char buffer_file2[MAXLEN+1];
   
   //get back to the begining of the file
   long location1 = lseek(file1, 0, SEEK_SET);
   long location2 = lseek(file2, 0, SEEK_SET);
    
    //read the data to the buffer
    int read1 =read(file1, buffer_file1, MAXLEN);
    int read2 =read(file2, buffer_file2, MAXLEN);
		if (read1 == -1 || read2 == -1) {
			return ERROR;
		}
    for(i=0; i<len; i++)
    {
     if(buffer_file1[i] != buffer_file2[i])
     {
       return FALSE;
     }
    }
    return TRUE;
}

int is_similar(int file1, int file2, long len1, long len2)
{
   int i;
   int counter1=0;
   int counter2=0;
   char buffer_file1[MAXLEN+1];
   char buffer_file2[MAXLEN+1];
    
   char temp_buffer1[MAXLEN+1];
   char temp_buffer2[MAXLEN+1];
    
   //get back to the begining of the file
   long location1 = lseek(file1, 0, SEEK_SET);
   long location2 = lseek(file2, 0, SEEK_SET);
    
   //read the data to the buffer
   int read1 =read(file1, buffer_file1, MAXLEN);
   int read2 =read(file2, buffer_file2, MAXLEN);
   if (read1 == -1 || read2 == -1) 
   {
		return ERROR;
   }
   
   //copy the data without whitespaces and swich to lowercase for the first file
   
   for( i=0; i<len1; i++)
   {
     if(buffer_file1[i] != '\n' && buffer_file1[i] != '\t' && buffer_file1[i] != '\r' && buffer_file1[i] != 32)
     {
     temp_buffer1[counter1] = tolower(buffer_file1[i]);
     counter1++;
     }
   }
   
   //copy the data without whitespaces and swich to lowercase for the second file
   for(i=0; i<len2; i++)
   {
     if(buffer_file2[i] != '\n' && buffer_file2[i] != '\t' && buffer_file2[i] != '\r' && buffer_file2[i] != 32)
     {
      temp_buffer2[counter2] = tolower(buffer_file2[i]);
      counter2++;
     }
   }

   //compare the files length
   if (counter1 != counter2)
   {
     return FALSE;
   }
   else{
   //check if the files are equal
   for(i=0; i<counter1; i++)
   {
     if(temp_buffer1[i] != temp_buffer2[i])
     {
       return FALSE;
     }
   }
  }
   return TRUE;
}

int main(int argc, char *argv[]) {

	//validated input
	if (argc != 3) {
        return ERROR;
    }
	
	char *path_file1 = argv[1];
  char *path_file2 = argv[2];
	
	int file1 = open(path_file1, 0);
	int file2 = open(path_file2, 0);
	
	if (file1 < 0 || file2 < 0) {
		close(file1);
		close(file2);
		return ERROR;
	}
	
	//get the file length
	long len1 = lseek(file1, 0, SEEK_END);
	long len2 = lseek(file2, 0, SEEK_END);
  
	if(len1 == len2)
	{
		if(is_equal(file1, file2, len1))
		{
			return EQUAL;
		}
	}
	
	if(is_similar(file1, file2, len1, len2))
	{
		return SIMILAR;
	}
	
	//in case the files are different
	return DIFFERENT;
}
