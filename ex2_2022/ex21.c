// Yael Avioz 207237421

#include <fcntl.h>
#include <stdio.h>

#define MAXLEN 150

//check if the given char is a whitespaces
int is_whitespaces (char c)
{
    if ((c == ' ') || (c == '\n') || (c == '\t') || (c == '\r') || (c == '\v') || (c == '\f'))
    {
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{

    // validated input
    if (argc != 3)
    {
        return -1;
    }

    // open the files with the flag read only
    int file1 = open(argv[1], O_RDONLY);
    int file2 = open(argv[2], O_RDONLY);

    // in case there was a failiure
    if (file1 < 0 || file2 < 0)
    {
        close(file1);
        close(file2);
        return -1;
    }

    // save the length for each file
    long file1_len = lseek(file1, 0, SEEK_END);
    long file2_len = lseek(file2, 0, SEEK_END);

    // create buffer to each file
    char buffer1[MAXLEN];
    char buffer2[MAXLEN];

    // return to begining
    long file1_beg = lseek(file1, 0, SEEK_SET);
    long file2_beg = lseek(file2, 0, SEEK_SET);

    // read the data of each file to the buffer
    int read_file1 = read(file1, buffer1, file1_len + 1);
    int read_file2 = read(file2, buffer2, file2_len + 1);

    // in case there was a failiure
    if (read_file1 == -1 || read_file2 == -1)
    {
        return -1;
    }

    // create flags to sign if the files are equals or simulars
    int equal_flag = 1;
    int similar_flag = 1;
    int i = 0;

    // compare the given files
    if (file1_len == file2_len)
    {
        for (i = 0; i < file1_len; i++)
        {
            if (buffer1[i] != buffer2[i])
            {
                equal_flag = 0;
            }
        }
    }
    //in case the file length is diffrent they are not equal
    else
    {
        equal_flag = 0;
    }

    if (equal_flag == 0)
    {
        // create buffers and copy the data without whitespaces
        char tmp_buffer1[MAXLEN];
        char tmp_buffer2[MAXLEN];

        int i, j, n = 0;

        for (i = 0; i < file1_len; i++)
        {
            if (is_whitespaces(buffer1[i]) == 1)
            {
                tmp_buffer1[j] = tolower(buffer1[i]);
                j++;
            }
        }

        for (i = 0; i < file2_len; i++)
        {
            if (is_whitespaces(buffer2[i]) == 1)
            {
                tmp_buffer2[n] = tolower(buffer2[i]);
                n++;
            }
        }

        // if we get 2 buffers with diffrent size the files are not simulars
        if (j != n)
        {
            similar_flag = 0;
        }

        // check if the files are simulars
        if (similar_flag != 0)
        {
            for (i = 0; i < j; i++)
            {
                if (tmp_buffer1[i] != tmp_buffer2[i])
                {
                    similar_flag = 0;
                }
            }
        }
    }

    // the files are equals
    if (equal_flag == 1)
    {
        return 1;
    }

    // the files are simulars
    if (similar_flag == 1)
    {
        return 3;
    }

    // the files are diffrent
    return 2;
}