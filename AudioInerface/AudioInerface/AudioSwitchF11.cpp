
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#include "WindowsAudioOutput/WindowsAudioOutput.h"


void printAudioPlaybackDevice(LPWSTR id, LPWSTR name, BOOL def)
{
    printf("%d %ws %ws\n", def, name, id);
}

static std::wstring setId = L"{0.0.0.00000000}.{a0c1eb94-f9dd-492c-a688-a99dad311d97}";

WindowsAudioOutput* WAO;
void setDefault(LPWSTR id, LPWSTR name, BOOL def)
{
    if (std::wstring((WCHAR*)id) == setId)
    {
        WAO->SetDefaultAudioPlaybackDeviceById(setId);
    }
}

void enumAudioDevices(WCHAR* id, WCHAR* name, BOOL def)
{
    std::wstring n = std::wstring((WCHAR*)name);
    std::wstring i = std::wstring((WCHAR*)name);

    std::string active = "false";
    if (def)
    {
        active = "true";
    }

    std::cout << active << ";" << std::string(n.begin(), n.end()) << ";" << std::string(i.begin(), i.end()) << std::endl;
}


int main(int len, char** args)
{
    if (len < 2 || len > 3)
    {
        std::cout << "USAGE: -l (list) | -s deviceId";
        std::string a;
        std::cin >> a;
        system("pause");
        return 0;
    }

    std::string type = std::string(args[1]);
    

    if (type == "-l")
    {
        WindowsAudioOutput* windowsAudioOutput = new WindowsAudioOutput();
        windowsAudioOutput->EnumerateAudioPlaybackDevices(printAudioPlaybackDevice);
    }
    else if (type == "-s")
    {
        std::string id = std::string(args[2]);
        WindowsAudioOutput* windowsAudioOutput = new WindowsAudioOutput();
        WAO = windowsAudioOutput;
        windowsAudioOutput->EnumerateAudioPlaybackDevices(setDefault);

    }
    std::string a;
    std::cin >> a;
    system("pause");
    return 0;
}

