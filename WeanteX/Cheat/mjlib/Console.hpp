class Console {
public:
    static void Enable() {
        AllocConsole();
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
    }

    static void Close() {
        fclose(stdin);
        fclose(stdout);
        fclose(stderr);
        FreeConsole();
    }

    static void ExitError() {
        Sleep(5000);
        exit(1);
    }

    static void Clear() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        DWORD dwSize = csbi.dwSize.X * csbi.dwSize.Y;
        DWORD dwWritten;
        COORD coord = { 0, 0 };
        FillConsoleOutputCharacter(hConsole, ' ', dwSize, coord, &dwWritten);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwSize, coord, &dwWritten);
        SetConsoleCursorPosition(hConsole, coord);
    }
};