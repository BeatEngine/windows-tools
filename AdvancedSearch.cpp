#include <windows.h>

#include <stdio.h>
#include <string.h>

unsigned long fileSize(char* path)
{
    FILE* f = fopen(path, "rb");
    if(f == 0)
    {
        return 0;
    }
    fseek(f, 0, SEEK_END);
    unsigned long sz = ftell(f);
    fclose(f);
    return sz;
}

#define largeToInteger(low, high) (static_cast<long long>(high) << 32) | low


double powul(long x, int p)
{
    long res = x;
    for(int i = 1; i < p; i++)
    {
        res *= x;
    }
    return (double)res;
}

long countSubs(char* path)
{
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;
    long result = 0;
    if((hFind = FindFirstFile(path, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    do
    {
        if(strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, ".."))
        {
            result++;
        }
    }
    while(FindNextFile(hFind, &fdFile));
    FindClose(hFind);
    return result;
}

long long fsearch(char* root, char* fname, long size_min, FILE* csvOut = 0, double* total = 0, int deepth = 1, double partial = 1)
{
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;
    char sPath[2048];
    bool isNameNotRelevant = strcmp(fname,"*") == 0;
    sprintf(sPath, "%s\\*.*", root);
    long long sum = 0;
    bool newed = false;

    if(total == 0)
    {
        total = new double;
        *total = 0;
        newed = true;
    }

    if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    long csubs = countSubs(sPath);
    double nprt = partial;
    do
    {
        if(strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0 && (isNameNotRelevant || strstr(fdFile.cFileName, fname) != 0 || (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)))
        {
            sprintf(sPath, "%s\\%s", root, fdFile.cFileName);
            if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
            {
                if(!csvOut &&size_min == 0 && (isNameNotRelevant || strstr(fdFile.cFileName, fname) != 0))
                {
                    printf("Directory: %s\n", sPath);
                }
                long long dirSum = fsearch(sPath, fname, size_min, csvOut, total, deepth+1, partial/csubs); //Recursion, I love it!
                nprt -= partial/csubs;
                if(csvOut && size_min == 0 && (isNameNotRelevant || strstr(fdFile.cFileName, fname) != 0))
                {
                    fprintf(csvOut, "Directory;\"%s\";%lld;%lld\n", sPath, dirSum, dirSum/1000000);
                }
                sum += dirSum;
            }
            else if(fdFile.nFileSizeLow > size_min && (isNameNotRelevant || strstr(fdFile.cFileName, fname) != 0)){

                //unsigned int tfls[2] = {fdFile.nFileSizeLow, fdFile.nFileSizeHigh};
                long long tfsz = largeToInteger(fdFile.nFileSizeLow, fdFile.nFileSizeHigh);//fileSize(sPath);//*((unsigned long*)(tfls));
                sum += tfsz;
                if(csvOut)
                {
                    if(tfsz < 0)
                    {
                        tfsz = 0;
                    }
                    fprintf(csvOut, "File;\"%s\";%lld;%lld\n", sPath, tfsz, tfsz/1000000);
                }
                else
                {
                    if(tfsz > 1000000000)
                    {
                        printf("File: %s  %d GB\n", sPath, tfsz/1000000000);
                    }
                    else if(tfsz > 1000000)
                    {
                        printf("File: %s  %d MB\n", sPath, tfsz/1000000);
                    }
                    else if(tfsz > 1000)
                    {
                        printf("File: %s  %d KB\n", sPath, tfsz/1000);
                    }
                    else
                    {
                        printf("File: %s  %d Bytes\n", sPath, tfsz);
                    }
                }
            }

        }
    }
    while(FindNextFile(hFind, &fdFile)); //Find the next file.
    FindClose(hFind); //Always, Always, clean things up!
    *total += nprt;

    if(*total>1)
    {
        *total = 1.0;
    }
    if(csvOut && deepth < 5)
    {
        printf("\r%f% ", (float)(*total*100));
    }
    if(newed)
    {
        delete total;
    }
    return sum;
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
            fprintf(f, "Type;Path;Size in Bytes;Size in MB\n");
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






