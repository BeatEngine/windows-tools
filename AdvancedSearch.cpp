#include <windows.h>

#include <stdio.h>
#include <string.h>

void fsearch(char* root, char* fname, long size_min, FILE* csvOut = 0)
{
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;
    char sPath[2048];
    bool isNameNotRelevant = strcmp(fname,"*") == 0;
    sprintf(sPath, "%s\\*.*", root);
    if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return;
    }
    do
    {
        if(strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0 && (isNameNotRelevant || strstr(fdFile.cFileName, fname) != 0 || (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)))
        {
            sprintf(sPath, "%s\\%s", root, fdFile.cFileName);
            if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
            {
                if(size_min == 0 && (isNameNotRelevant || strstr(fdFile.cFileName, fname) != 0))
                {
                    if(csvOut)
                    {
                        fprintf(csvOut, "Directory;%s;0\n", sPath);
                    }
                    else
                    {
                        printf("Directory: %s\n", sPath);
                    }
                }
                fsearch(sPath, fname, size_min, csvOut); //Recursion, I love it!
            }
            else if(fdFile.nFileSizeLow > size_min && (isNameNotRelevant || strstr(fdFile.cFileName, fname) != 0)){

                if(csvOut)
                {
                    int tfls[2] = {fdFile.nFileSizeLow, fdFile.nFileSizeHigh};
                    unsigned long tfsz = *((unsigned long*)(tfls));
                    if(tfsz < 0)
                    {
                        tfsz = 0;
                    }
                    fprintf(csvOut, "File;%s;%lu\n", sPath, tfsz);
                }
                else
                {
                    printf("File: %s\n", sPath);
                }
            }
        }
    }
    while(FindNextFile(hFind, &fdFile)); //Find the next file.
    FindClose(hFind); //Always, Always, clean things up!
}





int main(int args, char** argv)
{
    if(args < 3)
    {
        printf("Usage:\n %s -d ROOT -n NAME -s MIN_SIZE_BYTES -csv OUTPUT_NAME\n", argv[0]);
        return 0;
    }
    char root[1024] = "";
    char name[1024] = "*";
    char csvName[256] = "";
    long minSize = 0;
    bool csv = false;

    for(int i = 1; i < args; i++)
    {
        if(strcmp(argv[i], "-d") == 0 && i+1 < args)
        {
            memcpy(root, argv[i+1], strlen(argv[i+1]));
        }
        else if(strcmp(argv[i], "-n") == 0 && i+1 < args)
        {
            memcpy(name, argv[i+1], strlen(argv[i+1]));
        }
        else if(strcmp(argv[i], "-s") == 0 && i+1 < args)
        {
            sscanf (argv[i+1],"%ld",&minSize);
        }
        else if(strcmp(argv[i], "-csv") == 0 && i+1 < args)
        {
            csv = true;
            memcpy(csvName, argv[i+1], strlen(argv[i+1]));
        }
    }
    if(strlen(root) > 0 && strlen(name) > 0)
    {
        if(csv)
        {
            FILE* f = fopen(csvName, "w");
            fprintf(f, "Type;Path;Size in MB\n");
            fsearch(root, name, minSize, f);
            fclose(f);
        }
        else
        {
            fsearch(root, name, minSize, 0);
        }
    }
    return 0;
}






