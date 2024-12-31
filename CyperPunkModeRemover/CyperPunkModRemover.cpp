// CyperPunkModeRemover.cpp : Defines the entry point for the application.
#include "Utilities.h"
#include "framework.h"
#include "CyperPunkModRemover.h"
#include <commdlg.h>                // Common Dialog for file selection
#include <filesystem>               // Filesystem utilities
#include <vector>                    // Standard vector container
#include <string>                   // String utilities
#include <fstream>                  // File operations
#include <zip.h>                // libzip library for handling ZIP files
#include <Windows.h>            // For Windows GUI functions and MessageBox
#include <RichEdit.h>       //Support rich textbox to include colors and fonts

#define MAX_LOADSTRING 100  // Maximum string size for resources
#define PROGRAM_VERSION L"1.0.0"

namespace fs = std::filesystem; // Alias for filesystem namespace

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Utility function to convert std::wstring to std::string
std::string wstringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], sizeNeeded, NULL, NULL);
    return result;
}
// Function declarations for initialization and message handling
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

// Entry point for the application
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    // Load application title and window class name
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CYPERPUNKMODEREMOVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    // Initialize and display the application window
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CYPERPUNKMODEREMOVER));

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//  FUNCTION: MyRegisterClass()
//  PURPOSE: Registers the main window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CYPERPUNKMODEREMOVER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CYPERPUNKMODEREMOVER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

// Creates and initializes the main application window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, L"Cyberpunk Zip Archive Mod Remover", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 900, 700, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

// Saves the main directory path to a file
void saveMainDirectory(const std::wstring& directory) {
    std::ofstream settingsFile("settings.ini");
    if (settingsFile.is_open()) {
        settingsFile << std::string(directory.begin(), directory.end());
        settingsFile.close();
    }
}
// Loads the main directory path from a file
std::wstring loadMainDirectory() {
    std::ifstream settingsFile("settings.ini");
    if (settingsFile.is_open()) {
        std::string directory;
        std::getline(settingsFile, directory);
        settingsFile.close();
        return std::wstring(directory.begin(), directory.end());
    }
    // Return default directory if settings file is not found
    return L"D:\\Program Files (x86)\\GOG Galaxy\\Games\\Cyberpunk 2077"; // Default value
}
//Function to color text for Rich textbox
void AppendColoredText(HWND hwndRichEdit, const std::wstring& text, COLORREF color) {
    CHARFORMAT2 cf = { 0 };
    cf.cbSize = sizeof(CHARFORMAT2);
    cf.dwMask = CFM_COLOR | CFM_FACE;
    cf.crTextColor = color;
    wcscpy_s(cf.szFaceName, L"Consolas");

    SendMessage(hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    // Append the text
    int len = GetWindowTextLength(hwndRichEdit);
    SendMessage(hwndRichEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessage(hwndRichEdit, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
}

void appendToRichText(HWND richTextHandle, const std::wstring& text, COLORREF color = RGB(0, 0, 0), bool bold = false) {
    // Move caret to the end of the rich text box
    CHARRANGE cr;
    cr.cpMin = -1;
    cr.cpMax = -1;
    SendMessage(richTextHandle, EM_EXSETSEL, 0, (LPARAM)&cr);

    // Set character formatting
    CHARFORMAT2 cf = { 0 };
    cf.cbSize = sizeof(CHARFORMAT2);
    cf.dwMask = CFM_COLOR | CFM_BOLD;
    cf.crTextColor = color;
    cf.dwEffects = bold ? CFE_BOLD : 0;

    // Apply formatting to the selection
    SendMessage(richTextHandle, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    // Insert the text
    SendMessage(richTextHandle, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
}


// Main window procedure for handling messages
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND btnSelectZip, btnRemoveFiles, btnCheckInstall, editOutput, editDirectory;
    static std::vector<std::wstring> selectedZipFilePaths; // Store selected ZIP file paths
    static std::wstring selectedDirectory = L"D:\\Program Files (x86)\\GOG Galaxy\\Games\\Cyberpunk 2077"; // Default directory
    wchar_t dirBuffer[MAX_PATH] = { 0 }; // Buffer for directory path

    const std::vector<std::wstring> gameFolders = { L"bin", L"JSGME-MODS", L"r6", L"red4ext", L"archive", L"engine" };

    switch (uMsg) {
    case WM_CREATE:
        //Dynamically load library for rich textbox
        //LoadLibrary(TEXT("Msftedit.dll"));
        if (!LoadLibrary(TEXT("Msftedit.dll"))) {
            MessageBox(NULL, L"Rich Edit Control could not be loaded. Ensure your system supports it.", L"Error", MB_ICONERROR);
            PostQuitMessage(1);
            return -1;
        }
        // Load previously saved directory
        selectedDirectory = loadMainDirectory(); // Load saved directory
        // Create UI components
        btnCheckInstall = CreateWindow(L"BUTTON", L"Check Install", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 25, 120, 30, hwnd, (HMENU)3, NULL, NULL);
        
        btnSelectZip = CreateWindow(L"BUTTON", L"Select ZIP(s)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 65, 120, 30, hwnd, (HMENU)1, NULL, NULL);

        btnRemoveFiles = CreateWindow(L"BUTTON", L"Remove Files", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            200, 65, 120, 30, hwnd, (HMENU)2, NULL, NULL);

        // Initially disable the Remove Files button
        EnableWindow(btnRemoveFiles, FALSE);

        editDirectory = CreateWindow(L"EDIT", selectedDirectory.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            50, 105, 800, 25, hwnd, NULL, NULL, NULL);

        //Rich textbox
        editOutput = CreateWindowEx(WS_EX_CLIENTEDGE, MSFTEDIT_CLASS, L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
            50, 150, 800, 450, hwnd, NULL, hInst, NULL);
        return 0;
        
        //Standard textbox
        //editOutput = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
        //    50, 125, 600, 300, hwnd, NULL, NULL, NULL);
        //return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) { // Select ZIP(s)
            // Clear the rich text box before appending new data
            SetWindowText(editOutput, L"");

            // File selection logic
            wchar_t zipFiles[MAX_PATH * 100] = { 0 };
            OPENFILENAME ofn = { 0 };
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFile = zipFiles;
            ofn.nMaxFile = sizeof(zipFiles) / sizeof(wchar_t);
            ofn.lpstrFilter = L"Archive Files\0*.zip;\0";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

            if (GetOpenFileName(&ofn)) {
                selectedZipFilePaths.clear(); // Clear previous selections
                std::wstring directory = zipFiles;
                wchar_t* file = zipFiles + directory.length() + 1;

                // Handle single or multiple file selections
                if (*file == '\0') {
                    selectedZipFilePaths.push_back(directory); // Single file selected
                }
                else {
                    while (*file) {
                        selectedZipFilePaths.emplace_back(directory + L"\\" + file);
                        file += wcslen(file) + 1;
                    }
                }

                if (!selectedZipFilePaths.empty()) {
                    // Enable the Remove Files button after successful ZIP selection
                    EnableWindow(btnRemoveFiles, TRUE);
                }

                // Process and display selected ZIPs
                for (const auto& filePath : selectedZipFilePaths) {
                    std::wstring archiveName = fs::path(filePath).filename().wstring();
                    appendToRichText(editOutput, L"Archive: " + archiveName + L"\r\n", RGB(0, 0, 0), true); // Bold and black for archive names

                    auto files = listFilesInArchive(wstringToString(filePath));
                    if (files.empty()) {
                        if (fs::path(filePath).extension() == L".rar") {
                            appendToRichText(editOutput, L" - Could not process RAR file. Ensure 7-Zip is installed.\r\n", RGB(255, 0, 0)); // Red for error
                        }
                        else {
                            appendToRichText(editOutput, L" - No files found or unsupported archive format.\r\n", RGB(255, 0, 0)); // Red for error
                        }
                    }
                    else {
                        for (const auto& file : files) {
                            appendToRichText(editOutput, L" - " + stringToWstring(file) + L"\r\n", RGB(0, 0, 0)); // Black for file names
                        }
                    }
                }
            }
        }
        else if (LOWORD(wParam) == 2) { // Remove Files
            // File removal logic
            if (selectedZipFilePaths.empty()) {
                showError("Please select one or more archive files first.");
                break;
            }

            // Disable the Remove Files button
            EnableWindow(btnRemoveFiles, FALSE);

            // Clear the rich text box before appending new data
            SetWindowText(editOutput, L"");

            wchar_t dirBuffer[MAX_PATH] = { 0 };
            GetWindowText(editDirectory, dirBuffer, MAX_PATH);
            selectedDirectory = dirBuffer; // Update the directory path from the textbox

            std::vector<std::wstring> successful, notFound, errors;

            // Iterate through selected archives
            for (const auto& archivePath : selectedZipFilePaths) {
                std::wstring archiveName = fs::path(archivePath).filename().wstring();
                auto files = listFilesInArchive(wstringToString(archivePath));

                if (files.empty()) {
                    notFound.push_back(archiveName);
                    continue;
                }

                bool atLeastOneFileFound = false;
                bool deletionErrorOccurred = false;

                // Attempt to remove files
                for (const auto& file : files) {
                    std::wstring filePath = selectedDirectory + L"\\" + stringToWstring(file);

                    if (fs::exists(filePath)) {
                        atLeastOneFileFound = true;
                        try {
                            fs::remove(filePath);
                        }
                        catch (const std::exception&) {
                            deletionErrorOccurred = true;
                        }
                    }
                }

                // Categorize results
                if (!atLeastOneFileFound) {
                    notFound.push_back(archiveName);
                }
                else if (deletionErrorOccurred) {
                    errors.push_back(archiveName);
                }
                else {
                    successful.push_back(archiveName);
                }
            }

            // Display summary results in the rich text box
            if (!successful.empty()) {
                appendToRichText(editOutput, L"Successfully removed files from Cyberpunk directory:\r\n", RGB(0, 100, 0), true); // Bold and green for success
                for (const auto& archive : successful) {
                    appendToRichText(editOutput, L" - " + archive + L"\r\n", RGB(0, 0, 0)); // Black for file names
                }
            }
            if (!notFound.empty()) {
                appendToRichText(editOutput, L"Files were not found in theCyberpunk directory:\r\n", RGB(255, 0, 0), true); // Bold and red for missing files
                for (const auto& archive : notFound) {
                    appendToRichText(editOutput, L" - " + archive + L"\r\n", RGB(0, 0, 0)); // Black for file names
                }
            }
            if (!errors.empty()) {
                appendToRichText(editOutput, L"Errors occurred during file removal:\r\n", RGB(255, 0, 0), true); // Bold and red for errors
                for (const auto& archive : errors) {
                    appendToRichText(editOutput, L" - " + archive + L"\r\n", RGB(0, 0, 0)); // Black for file names
                }
            }
        }
        else if (LOWORD(wParam) == 3) { // Check Installation
            wchar_t zipFiles[MAX_PATH * 100] = { 0 };
            OPENFILENAME ofn = { 0 };
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFile = zipFiles;
            ofn.nMaxFile = sizeof(zipFiles) / sizeof(wchar_t);
            ofn.lpstrFilter = L"ZIP Files\0*.zip\0";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

            selectedZipFilePaths.clear(); // Reset selected paths
            // Clear the rich text box before appending new data
            SetWindowText(editOutput, L"");
            EnableWindow(btnRemoveFiles, FALSE);

            if (GetOpenFileName(&ofn)) {
                std::wstring directory = zipFiles;
                std::vector<std::wstring> selectedFiles;

                wchar_t* file = zipFiles + directory.length() + 1;
                if (*file == '\0') { // Single file selected
                    selectedFiles.push_back(directory);
                }
                else { // Multiple files selected
                    while (*file) {
                        selectedFiles.emplace_back(directory + L"\\" + file);
                        file += wcslen(file) + 1;
                    }
                }

                // Categorize files as installed or not installed
                std::vector<std::wstring> installedFiles;
                std::vector<std::wstring> notInstalledFiles;

                for (const auto& filePath : selectedFiles) {
                    std::wstring archiveName = fs::path(filePath).filename().wstring();
                    auto files = listFilesInArchive(wstringToString(filePath));
                    bool allFilesInstalled = true;

                    for (const auto& file : files) {
                        std::wstring fullFilePath = selectedDirectory + L"\\" + stringToWstring(file);
                        if (!fs::exists(fullFilePath)) {
                            allFilesInstalled = false;
                            break;
                        }
                    }

                    if (allFilesInstalled) {
                        installedFiles.push_back(archiveName);
                    }
                    else {
                        notInstalledFiles.push_back(archiveName);
                    }
                }

                // Display results in the rich text box
                if (!installedFiles.empty()) {
                    appendToRichText(editOutput, L"Installed Archives:\r\n", RGB(0, 100, 0), true); // Bold and green for installed
                    for (const auto& archive : installedFiles) {
                        appendToRichText(editOutput, L" - " + archive + L"\r\n", RGB(0, 0, 0)); // Black for archive names
                    }
                }
                if (!notInstalledFiles.empty()) {
                    appendToRichText(editOutput, L"Not Installed Archives:\r\n", RGB(255, 0, 0), true); // Bold and red for not installed
                    for (const auto& archive : notInstalledFiles) {
                        appendToRichText(editOutput, L" - " + archive + L"\r\n", RGB(0, 0, 0)); // Black for archive names
                    }
                }
            }
        }
        if (LOWORD(wParam) == IDM_ABOUT) {
            std::wstring aboutText = L"Cyberpunk Zip Archive Mode Remover\nVersion: " + std::wstring(PROGRAM_VERSION);
            MessageBox(hwnd, aboutText.c_str(), L"About My Program", MB_OK | MB_ICONINFORMATION);
        }
        break;

    case WM_DESTROY:
        // Save directory when application exits
        GetWindowText(editDirectory, dirBuffer, MAX_PATH);
        saveMainDirectory(dirBuffer); // Save directory to file
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}



//case WM_COMMAND:
//    if (LOWORD(wParam) == 1) { // Select ZIP(s)
//        SendMessage(editOutput, EM_REPLACESEL, FALSE, (LPARAM)L"Appending Text Works\n");
//
//        // File selection logic
//        wchar_t zipFiles[MAX_PATH * 100] = { 0 };
//        OPENFILENAME ofn = { 0 };
//        ofn.lStructSize = sizeof(ofn);
//        ofn.lpstrFile = zipFiles;
//        ofn.nMaxFile = sizeof(zipFiles) / sizeof(wchar_t);
//        ofn.lpstrFilter = L"Archive Files\0*.zip;*.rar\0";
//        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
//
//        // Clear the rich text box before appending new data
//        SetWindowText(editOutput, L"");
//
//        if (GetOpenFileName(&ofn)) {
//            selectedZipFilePaths.clear(); // Clear previous selections
//            std::wstring directory = zipFiles;
//            wchar_t* file = zipFiles + directory.length() + 1;
//            if (*file == '\0') { // Single file selected
//                selectedZipFilePaths.push_back(directory);
//            }
//            else { // Multiple files selected
//                while (*file) {
//                    selectedZipFilePaths.emplace_back(directory + L"\\" + file);
//                    file += wcslen(file) + 1;
//                }
//            }
//
//            if (!selectedZipFilePaths.empty()) {
//                // Enable the Remove Files button after successful ZIP selection
//                EnableWindow(btnRemoveFiles, TRUE);
//            }
//
//            std::wstring output;
//            for (const auto& filePath : selectedZipFilePaths) {
//                std::wstring archiveName = fs::path(filePath).filename().wstring();
//                output += L"Archive: " + archiveName + L"\r\n";
//                appendToRichText(editOutput, L"Archive: " + archiveName + L"\r\n", RGB(0, 0, 0), true); // Bold archive name
//
//                auto files = listFilesInArchive(wstringToString(filePath));
//                if (files.empty()) {
//                    if (fs::path(filePath).extension() == L".rar") {
//                        output += L" - Could not process RAR file. Ensure 7-Zip is installed.\r\n";
//                        appendToRichText(editOutput, L" - Could not process RAR file. Ensure 7-Zip is installed.\r\n", RGB(255, 0, 0));
//                    }
//                    else {
//                        output += L" - No files found or unsupported archive format.\r\n";
//                        appendToRichText(editOutput, L" - No files found or unsupported archive format.\r\n", RGB(255, 0, 0));
//                    }
//                }
//                else {
//                    for (const auto& file : files) {
//                        output += L" - " + stringToWstring(file) + L"\r\n";
//                        appendToRichText(editOutput, L" - " + stringToWstring(file) + L"\r\n", RGB(0, 0, 0));
//                    }
//                }
//            }
//            //SetWindowText(editOutput, output.c_str());
//        }
//    }
//    else if (LOWORD(wParam) == 2) { // Remove Files
//        // File removal logic
//        if (selectedZipFilePaths.empty()) {
//            showError("Please select one or more archive files first.");
//            break;
//        }
//
//        //Disable remove button
//        EnableWindow(btnRemoveFiles, FALSE);
//
//        wchar_t dirBuffer[MAX_PATH] = { 0 };
//        GetWindowText(editDirectory, dirBuffer, MAX_PATH);
//        selectedDirectory = dirBuffer; // Update the directory path from the textbox
//
//        std::vector<std::wstring> successful, notFound, errors;
//
//        for (const auto& archivePath : selectedZipFilePaths) {
//            std::wstring archiveName = fs::path(archivePath).filename().wstring();
//            auto files = listFilesInArchive(wstringToString(archivePath));
//
//            if (files.empty()) {
//                notFound.push_back(archiveName);
//                continue;
//            }
//
//            bool atLeastOneFileFound = false;
//            bool deletionErrorOccurred = false;
//
//            for (const auto& file : files) {
//                std::wstring filePath = selectedDirectory + L"\\" + stringToWstring(file);
//
//                if (fs::exists(filePath)) {
//                    atLeastOneFileFound = true;
//                    try {
//                        fs::remove(filePath);
//                    }
//                    catch (const std::exception&) {
//                        deletionErrorOccurred = true;
//                    }
//                }
//            }
//
//            if (!atLeastOneFileFound) {
//                notFound.push_back(archiveName);
//            }
//            else if (deletionErrorOccurred) {
//                errors.push_back(archiveName);
//            }
//            else {
//                successful.push_back(archiveName);
//            }
//        }
//
//        // Generate summary output
//        std::wstring output;
//        if (!successful.empty()) {
//            output += L"Successfully removed files from:\r\n";
//            for (const auto& archive : successful) {
//                output += L" - " + archive + L"\r\n";
//            }
//        }
//        if (!notFound.empty()) {
//            output += L"Files were not found in the directory for:\r\n";
//            for (const auto& archive : notFound) {
//                output += L" - " + archive + L"\r\n";
//            }
//        }
//        if (!errors.empty()) {
//            output += L"Errors occurred during file removal for:\r\n";
//            for (const auto& archive : errors) {
//                output += L" - " + archive + L"\r\n";
//            }
//        }
//
//        SetWindowText(editOutput, output.c_str());
//    }
//    else if (LOWORD(wParam) == 3) { // Check Installation
//        wchar_t zipFiles[MAX_PATH * 100] = { 0 };
//        OPENFILENAME ofn = { 0 };
//        ofn.lStructSize = sizeof(ofn);
//        ofn.lpstrFile = zipFiles;
//        ofn.nMaxFile = sizeof(zipFiles) / sizeof(wchar_t);
//        ofn.lpstrFilter = L"ZIP Files\0*.zip\0";
//        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
//
//        selectedZipFilePaths.clear(); // Reset selected paths
//        // Clear the rich text box before appending new data
//        SetWindowText(editOutput, L"");
//        EnableWindow(btnRemoveFiles, FALSE);
//
//        if (GetOpenFileName(&ofn)) {
//            std::wstring directory = zipFiles;
//            std::vector<std::wstring> selectedFiles;
//
//            wchar_t* file = zipFiles + directory.length() + 1;
//            if (*file == '\0') { // Single file selected
//                selectedFiles.push_back(directory);
//            }
//            else { // Multiple files selected
//                while (*file) {
//                    selectedFiles.emplace_back(directory + L"\\" + file);
//                    file += wcslen(file) + 1;
//                }
//            }
//
//            // Categorize files as installed or not installed
//            std::vector<std::wstring> installedFiles;
//            std::vector<std::wstring> notInstalledFiles;
//
//            for (const auto& filePath : selectedFiles) {
//                std::wstring archiveName = fs::path(filePath).filename().wstring();
//                auto files = listFilesInArchive(wstringToString(filePath));
//                bool allFilesInstalled = true;
//
//                for (const auto& file : files) {
//                    std::wstring fullFilePath = selectedDirectory + L"\\" + stringToWstring(file);
//                    if (!fs::exists(fullFilePath)) {
//                        allFilesInstalled = false;
//                        break;
//                    }
//                }
//
//                if (allFilesInstalled) {
//                    installedFiles.push_back(archiveName);
//                }
//                else {
//                    notInstalledFiles.push_back(archiveName);
//                }
//            }
//
//            // Build grouped output
//            std::wstring output;
//            if (!installedFiles.empty()) {
//                output += L"Installed Archives:\r\n";
//                for (const auto& archive : installedFiles) {
//                    output += L" - " + archive + L"\r\n";
//                }
//                output += L"\r\n"; // Add spacing between groups
//            }
//            if (!notInstalledFiles.empty()) {
//                output += L"Not Installed Archives:\r\n";
//                for (const auto& archive : notInstalledFiles) {
//                    output += L" - " + archive + L"\r\n";
//                }
//            }
//
//            //// Update the output window (Basic)
//            //SetWindowText(editOutput, output.c_str());
//            //Update the output window (Rich)
//            if (!installedFiles.empty()) {
//                AppendColoredText(editOutput, L"Installed Archives:\r\n", RGB(0, 128, 0)); // Green
//                for (const auto& archive : installedFiles) {
//                    AppendColoredText(editOutput, L" - " + archive + L"\r\n", RGB(0, 128, 0));
//                }
//            }
//            if (!notInstalledFiles.empty()) {
//                AppendColoredText(editOutput, L"Not Installed Archives:\r\n", RGB(255, 0, 0)); // Red
//                for (const auto& archive : notInstalledFiles) {
//                    AppendColoredText(editOutput, L" - " + archive + L"\r\n", RGB(255, 0, 0));
//                }
//            }
//        }
//    }
//    break;