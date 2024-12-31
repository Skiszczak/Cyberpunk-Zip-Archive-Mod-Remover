// Main.cpp : Defines...
#include "Utilities.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <zip.h> // libzip library for handling ZIP files
#include <Windows.h> // For GUI and MessageBox

namespace fs = std::filesystem;

int main() {
    // Constants
    const std::string baseDir = "D:\\Program Files (x86)\\GOG Galaxy\\Games\\Cyberpunk 2077";

    // GUI setup
    HWND hwnd = CreateWindow(L"STATIC", L"ZIP File Operation", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 100, 100, NULL, NULL, NULL, NULL);

    // Main loop
    while (true) {
        int choice = MessageBox(hwnd, L"Choose an operation:\n1. Select ZIP\n2. Remove Files", L"Operation", MB_ICONQUESTION | MB_YESNOCANCEL);
        if (choice == IDCANCEL) break;

        if (choice == IDYES) {
            // Select ZIP
            wchar_t zipFilePath[MAX_PATH] = { 0 };
            OPENFILENAME ofn = { 0 };
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFile = zipFilePath;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"ZIP Files\0*.zip\0";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

            if (GetOpenFileName(&ofn)) {
                auto files = listFilesInZip(std::string(zipFilePath, zipFilePath + wcslen(zipFilePath)));

                if (files.empty()) {
                    showInfo("No files found in the ZIP or invalid file structure.");
                }
                else {
                    std::string fileList;
                    for (const auto& file : files) fileList += file + "\n";
                    showInfo("Files found:\n" + fileList);
                }
            }
        }
        else if (choice == IDNO) {
            // Remove Files
            wchar_t zipFilePath[MAX_PATH] = { 0 };
            OPENFILENAME ofn = { 0 };
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFile = zipFilePath;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"ZIP Files\0*.zip\0";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

            if (GetOpenFileName(&ofn)) {
                auto files = listFilesInZip(std::string(zipFilePath, zipFilePath + wcslen(zipFilePath)));
                if (files.empty()) {
                    showInfo("No files found in the ZIP or invalid file structure.");
                }
                else {
                    if (deleteFiles(files, baseDir)) {
                        showInfo("Files deleted successfully.");
                    }
                    else {
                        showError("File deletion failed and changes were reverted.");
                    }
                }
            }
        }
    }

    DestroyWindow(hwnd);
    return 0;
}
