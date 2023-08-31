#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>



// Copyright (c) [2023] [Mike C. Vermeer]
// This software is licensed under the MIT License. See the LICENSE file for details.

// Global Variables
HANDLE threadHandles[10];  // Assuming a maximum of 10 simultaneous threads.
int threadIndex = 0;
HANDLE hMutex;

// Functions for presentation / look of the program

// Function to set console colors
void setConsoleColors() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    // Set the text and background colors
    SetConsoleTextAttribute(hConsole, 0x00 | BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED);

    // Clear the screen to apply the new background color over the entire console window
    system("cls");

    // Set Console Title
    SetConsoleTitle(L"Auto - Installer | by Mike C. Vermeer");
}

// Function to simulate typing effect
void typePrint(const char* str, unsigned int delay) {
    WaitForSingleObject(hMutex, INFINITE);
    for (size_t i = 0; i < strlen(str); i++) {
        putchar(str[i]);
        fflush(stdout); // Flush the output buffer to ensure the character is printed immediately
        Sleep(delay);
    }
    ReleaseMutex(hMutex);
}

// Start of program

// Check if the program is Elevated
BOOL IsElevated() {
    BOOL fIsElevated = FALSE;
    HANDLE hToken = NULL;

    // Try to open the process token
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD dwSize;

        // Retrieve the TokenElevation information
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
            fIsElevated = elevation.TokenIsElevated;
        }
    }

    // Clean up and close the handle
    if (hToken) {
        CloseHandle(hToken);
    }

    return fIsElevated;
}

// Function to download an installer using curl
void downloadInstaller(const char* url, const char* outputFilename) {
    WaitForSingleObject(hMutex, INFINITE);

    char command[1024];
    snprintf(command, sizeof(command), "curl -L -o %s %s", outputFilename, url); // -L to follow redirects
    system(command);

    ReleaseMutex(hMutex);
}

// Function to run an installer
void runInstaller(const char* installerPath) {
    system(installerPath);
}

void installSteam() {
    printf("Downloading and installing Steam...\n");
    downloadInstaller("https://cdn.cloudflare.steamstatic.com/client/installer/SteamSetup.exe", "steam_installer.exe");
    runInstaller("steam_installer.exe");
}

void installDiscord() {
    printf("Downloading and installing Discord...\n");
    downloadInstaller("https://dl.discordapp.net/apps/win/0.0.309/DiscordSetup.exe", "discord_installer.exe");
    runInstaller("discord_installer.exe");
}

void installUbisoft() {
    printf("Downloading and installing Ubisoft Connect...\n");
    downloadInstaller("https://ubi.li/4vxt9", "ubisoft_installer.exe");
    runInstaller("ubisoft_installer.exe");
}

void installSpotify() {
    printf("Downloading and installing Spotify...\n");
    downloadInstaller("https://download.scdn.co/SpotifySetup.exe", "spotify_installer.exe");
    runInstaller("spotify_installer.exe");
}

void installBattlenet() {
    printf("Downloading and installing Battle.net...\n");
    downloadInstaller("https://www.battle.net/download/getInstallerForGame?os=win&gameProgram=BATTLENET_APP&version=Live", "battlenet_installer.exe");
    runInstaller("battlenet_installer.exe");
}

void installEpiclauncher() {
    printf("Downloading and installing Epic Games Launcher...\n");
    downloadInstaller("https://launcher-public-service-prod06.ol.epicgames.com/launcher/api/installer/download/EpicGamesLauncherInstaller.msi", "epiclauncher_installer.msi");
    runInstaller("epiclauncher_installer.msi");
}

void programCleanup() {
    typePrint("\nRemoving files and cleaning up...", 25);

    remove("steam_installer.exe");
    remove("discord_installer.exe");
    remove("ubisoft_installer.exe");
    remove("spotify_installer.exe");
    remove("battlenet_installer.exe");
    remove("nvidia-drivers_installer.exe");
    remove("amd/ati-drivers_installer.exe");
    remove("dxdiag.txt");
    remove("epiclauncher_installer.msi");
}

int detectGraphicsCard() {
    typePrint("\n Scanning system for your Graphics Processing Unit...\n\n", 20);
    system("dxdiag /t dxdiag.txt");

    FILE* file = fopen("dxdiag.txt", "r");
    char line[512];
    if (!file) {
        printf("Failed to retrieve system information.\n");
        return -1;  // Unknown
    }

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "NVIDIA")) {
            fclose(file);
            return 1;  // NVIDIA
        }
        else if (strstr(line, "AMD") || strstr(line, "ATI")) {
            fclose(file);
            return 2;  // AMD
        }
    }

    fclose(file);
    return -1;  // Unknown
}

typedef struct {
    void (*installerFunc)();
} InstallerThreadData;

DWORD WINAPI runInstallerThread(LPVOID arg) {
    InstallerThreadData* data = (InstallerThreadData*)arg;
    data->installerFunc();
    free(data);
    return 0;
}

void spawnInstaller(void (*installerFunc)()) {
    InstallerThreadData* data = malloc(sizeof(InstallerThreadData));
    data->installerFunc = installerFunc;
    HANDLE threadHandle = CreateThread(NULL, 0, runInstallerThread, data, 0, NULL);
    threadHandles[threadIndex++] = threadHandle;
}

int main() {
    // Start of the main function
    hMutex = CreateMutex(NULL, FALSE, NULL);
    setConsoleColors();  // Set console colors to black text on white background

    if (!IsElevated()) {
        typePrint("This program needs to be run as administrator!\n", 2);
        typePrint("Press any key to exit...\n", 1);
        getch();  // waits for any keypress
        CloseHandle(hMutex);
        return 1;
    }

    typePrint("Welcome to the Auto - Installer Program!\n", 20);
    typePrint("This program allows you to automatically download and install popular software.\n", 20);
    typePrint("You can also detect your graphics card and install appropriate drivers.\n", 20);
    typePrint("Follow the prompts to select and install the software you need.\n\n", 20);

    // Copyright notice
    typePrint("Copyright (c) [2023]", 25);
    typePrint(" [Mike C. Vermeer] \n", 75);
    typePrint("This software is licensed under the MIT License. See the included LICENSE file for details.\n\n", 25);

    typePrint("\nDo you already have GPU drivers installed? (1 for Yes, 0 for No): ", 5);
    int skipDrivers;
    scanf("%d", &skipDrivers);

    typePrint("\n\nSelect software to install:\n", 5);
    typePrint("1. Steam\n", 5);
    typePrint("2. Discord\n", 5);
    typePrint("3. Ubisoft Connect\n", 5);
    typePrint("4. Spotify\n", 5);
    typePrint("5. Battle.net\n", 5);
    typePrint("6. Epic Games Launcher\n", 5);
    typePrint("7. Multiple selections\n", 5);
    typePrint("8. All software\n", 5);
    if (!skipDrivers) {
        typePrint("9. GPU Drivers only\n", 5);
    }
    typePrint("0. Exit\n", 10);
    typePrint("> ", 50);

    int choice;
    scanf("%d", &choice);

    switch (choice) {
    case 1:
        system("cls");
        installSteam();
        break;
    case 2:
        system("cls");
        installDiscord();
        break;
    case 3:
        system("cls");
        installUbisoft();
        break;
    case 4:
        system("cls");
        installSpotify();
        break;
    case 5:
        system("cls");
        installBattlenet();
        break;
    case 6:
        system("cls");
        installEpiclauncher();
        break;
    case 7:
        typePrint("\n\nEnter software numbers separated by space (end with 0): ", 5);
        int selection;
        while (scanf("%d", &selection) == 1 && selection != 0) {
            switch (selection) {
            case 1: spawnInstaller(installSteam); break;
            case 2: spawnInstaller(installDiscord); break;
            case 3: spawnInstaller(installUbisoft); break;
            case 4: spawnInstaller(installSpotify); break;
            case 5: spawnInstaller(installBattlenet); break;
            case 6: installEpiclauncher(); break;
            }
        }
        break;
    case 8:
        spawnInstaller(installSteam);
        spawnInstaller(installDiscord);
        spawnInstaller(installUbisoft);
        spawnInstaller(installSpotify);
        spawnInstaller(installBattlenet);
        spawnInstaller(installEpiclauncher);
        break;
    case 9:
        // GPU Drivers only
        break;
    case 0:
        // Exit
        programCleanup();
        system("cls");
        typePrint("\nExiting the program.", 50);
        Sleep(1250);
        CloseHandle(hMutex);
        return 0;
    }

    if (!skipDrivers) {
        int card = detectGraphicsCard();
        if (card == 1) {
            typePrint("NVIDIA graphics card detected. Installing drivers...\n", 5);
            downloadInstaller("https://uk.download.nvidia.com/GFE/GFEClient/3.27.0.112/GeForce_Experience_v3.27.0.112.exe", "nvidia-drivers_installer.exe");
            runInstaller("nvidia-drivers_installer.exe");
        }
        else if (card == 2) {
            typePrint("AMD graphics card detected. Installing drivers...\n", 5);
            downloadInstaller("https://drivers.amd.com/drivers/installer/23.10/whql/amd-software-adrenalin-edition-23.8.1-minimalsetup-230819_web.exe", "amd/ati-drivers_installer.exe");
            runInstaller("amd/ati-drivers_installer.exe");
        }
        else {
            typePrint("Could not detect graphics card or card not supported.\n", 5);
        }
    }

    WaitForMultipleObjects(threadIndex, threadHandles, TRUE, INFINITE);
    for (int i = 0; i < threadIndex; i++) {
        CloseHandle(threadHandles[i]);  // Close the thread handle after waiting
    }

    programCleanup();

    system("cls");
    typePrint("\nExiting the program.", 50);

    Sleep(1500);

    CloseHandle(hMutex);
    return 0;
}