// Utilities.cpp : Defines shared functions which are used across the program
#include "Utilities.h"
#include <Windows.h>
#include <zip.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>
namespace fs = std::filesystem;

void showError(const std::string& message) {
    std::wstring wideMessage(message.begin(), message.end());
    MessageBox(NULL, wideMessage.c_str(), L"Error", MB_ICONERROR | MB_OK);
}

void showInfo(const std::string& message) {
    std::wstring wideMessage(message.begin(), message.end());
    MessageBox(NULL, wideMessage.c_str(), L"Info", MB_ICONINFORMATION | MB_OK);
}

std::wstring stringToWstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], sizeNeeded);
    return result;
}

// Executes a command and captures its output
std::string runCommand(const std::string& command) {
    HANDLE hPipeRead, hPipeWrite;
    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    // Create a pipe for the child process's STDOUT
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        throw std::runtime_error("Failed to create pipe.");
    }

    // Ensure the read handle to the pipe is not inherited
    SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, 0);

    // Create the child process
    PROCESS_INFORMATION piProcInfo = { 0 };
    STARTUPINFOA siStartInfo = { 0 }; // Use STARTUPINFOA for ANSI version
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hPipeWrite;
    siStartInfo.hStdOutput = hPipeWrite;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    std::vector<char> cmd(command.begin(), command.end());
    cmd.push_back('\0'); // Null-terminate the command

    if (!CreateProcessA(NULL, cmd.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &siStartInfo, &piProcInfo)) {
        CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        throw std::runtime_error("Failed to create process.");
    }

    CloseHandle(hPipeWrite);

    // Read the output from the child process
    char buffer[128];
    std::string result;
    DWORD bytesRead;
    while (ReadFile(hPipeRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result += buffer;
    }

    CloseHandle(hPipeRead);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    return result;
}

std::vector<std::string> listFilesInZip(const std::string& zipFilePath) {
    const std::vector<std::string> gameFolders = { "bin", "JSGME-MODS", "r6", "red4ext", "archive", "engine" };
    std::vector<std::string> filteredFiles;

    int error;
    zip_t* archive = zip_open(zipFilePath.c_str(), ZIP_RDONLY, &error);
    if (!archive) {
        showError("Unable to open ZIP file: " + zipFilePath);
        return filteredFiles;
    }

    zip_int64_t numEntries = zip_get_num_entries(archive, 0);
    for (zip_int64_t i = 0; i < numEntries; ++i) {
        const char* name = zip_get_name(archive, i, 0);
        if (!name) continue;

        std::string filePath(name);
        std::replace(filePath.begin(), filePath.end(), '\\', '/'); // Normalize path separators

        // Skip directories
        if (filePath.back() == '/') continue;

        // Extract the folder structure
        size_t firstSlash = filePath.find('/');
        if (firstSlash != std::string::npos) {
            std::string topFolder = filePath.substr(0, firstSlash);

            // If the top-level folder is valid, keep the path as-is
            if (std::find(gameFolders.begin(), gameFolders.end(), topFolder) != gameFolders.end()) {
                filteredFiles.push_back(filePath);
            }
            // If the top-level folder is invalid, adjust the path to start from a valid folder
            else {
                // Find the first valid folder in the path
                size_t validPos = filePath.find_first_of("/");
                while (validPos != std::string::npos) {
                    std::string remainingPath = filePath.substr(validPos + 1);
                    size_t nextSlash = remainingPath.find('/');
                    std::string nextTopFolder = remainingPath.substr(0, nextSlash);

                    if (std::find(gameFolders.begin(), gameFolders.end(), nextTopFolder) != gameFolders.end()) {
                        filteredFiles.push_back(remainingPath);
                        break;
                    }
                    validPos = filePath.find_first_of("/", validPos + 1);
                }
            }
        }
    }

    zip_close(archive);

    //// Debug output
    //std::wstring debugMessage = L"Filtered Files:\n";
    //if (filteredFiles.empty()) {
    //    debugMessage += L" - No valid files found or invalid ZIP structure\n";
    //}
    //else {
    //    for (const auto& file : filteredFiles) {
    //        debugMessage += stringToWstring(file) + L"\n";
    //    }
    //}
    //MessageBox(NULL, debugMessage.c_str(), L"Debug: Filtered Files", MB_OK);

    return filteredFiles;
}

std::vector<std::string> listFilesInRar(const std::string& rarFilePath) {
    std::vector<std::string> fileList;

    // Command to list files in the .rar archive
    std::string command = "7z l \"" + rarFilePath + "\"";

    // Open a pipe to read the command output
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: Could not open pipe for command: " << command << std::endl;
        return fileList;
    }

    char buffer[256];
    bool fileSection = false;

    // Read the output of 7z.exe
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);

        // Detect start and end of file list section
        if (line.find("-------------------") != std::string::npos) {
            fileSection = !fileSection; // Toggle the flag
            continue;
        }

        // Parse file lines
        if (fileSection && !line.empty() && line.find("D....") == std::string::npos) {
            size_t lastSpace = line.find_last_of(' ');
            if (lastSpace != std::string::npos) {
                std::string fileName = line.substr(lastSpace + 1);
                fileList.push_back(fileName);
            }
        }
    }

    _pclose(pipe);
    return fileList;
}
std::vector<std::string> listFilesInArchive(const std::string& archivePath) {
    const std::vector<std::string> gameFolders = { "bin", "JSGME-MODS", "r6", "red4ext", "archive", "engine" };
    std::vector<std::string> filteredFiles;

    try {
        if (fs::path(archivePath).extension() == ".zip") {
            // Defer to listFilesInZip for ZIP files
            return listFilesInZip(archivePath);
        }
        else if (fs::path(archivePath).extension() == ".rar") {
            // Process RAR file (requires 7-Zip to be installed)
            std::string command = "7z l \"" + archivePath + "\"";
            std::string output = runCommand(command);

            if (output.find("7-Zip") == std::string::npos) {
                throw std::runtime_error("7-Zip is not installed or could not process RAR file.");
            }

            std::istringstream stream(output);    // Assuming `output` is a std::string
            std::string line;
            while (std::getline(stream, line)) {
                // Process the line
            }
            bool readingFiles = false;

            while (std::getline(stream, line)) {
                // Start reading files after the header
                if (line.find("-------------------") != std::string::npos) {
                    readingFiles = !readingFiles;
                    continue;
                }

                if (readingFiles) {
                    std::string filePath = line.substr(line.find_first_not_of(" "));
                    std::replace(filePath.begin(), filePath.end(), '\\', '/'); // Normalize path separators

                    // Skip directories
                    if (filePath.back() == '/') continue;

                    // Extract the folder structure
                    size_t firstSlash = filePath.find('/');
                    if (firstSlash != std::string::npos) {
                        std::string topFolder = filePath.substr(0, firstSlash);

                        // If the top-level folder is valid, keep the path as-is
                        if (std::find(gameFolders.begin(), gameFolders.end(), topFolder) != gameFolders.end()) {
                            filteredFiles.push_back(filePath);
                        }
                        else {
                            // Adjust the path to remove invalid top-level folders
                            size_t validPos = filePath.find_first_of("/");
                            while (validPos != std::string::npos) {
                                std::string remainingPath = filePath.substr(validPos + 1);
                                size_t nextSlash = remainingPath.find('/');
                                std::string nextTopFolder = remainingPath.substr(0, nextSlash);

                                if (std::find(gameFolders.begin(), gameFolders.end(), nextTopFolder) != gameFolders.end()) {
                                    filteredFiles.push_back(remainingPath);
                                    break;
                                }
                                validPos = filePath.find_first_of("/", validPos + 1);
                            }
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception& e) {
        showError("Error reading archive: " + std::string(e.what()));
    }

    return filteredFiles;
}
bool deleteFiles(const std::vector<std::string>& files, const std::string& basePath) {
    std::vector<std::string> deletedFiles;
    bool anyFileExists = false;

    try {
        for (const std::string& file : files) {
            fs::path filePath = fs::path(basePath) / fs::path(file);
            if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
                anyFileExists = true; // At least one file exists
                fs::remove(filePath);
                deletedFiles.push_back(file);
            }
        }

        if (!anyFileExists) {
            // No matching files were found
            return false;
        }

        return true;
    }
    catch (const std::exception& e) {
        // Rollback deleted files if an error occurs
        for (const std::string& file : deletedFiles) {
            fs::path filePath = fs::path(basePath) / fs::path(file);
            std::ofstream recoveryFile(filePath.string());
        }
        showError("Error during file deletion: " + std::string(e.what()));
        return false;
    }
}

//bool deleteFiles(const std::vector<std::string>& files, const std::string& basePath) {
//    std::vector<std::string> deletedFiles;
//
//    try {
//        bool filesDeleted = false;
//
//        for (const std::string& file : files) {
//            fs::path filePath = fs::path(basePath) / file;
//            if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
//                fs::remove(filePath);
//                deletedFiles.push_back(file);
//                filesDeleted = true;
//            }
//        }
//
//        if (!filesDeleted) {
//            showInfo("No files from the ZIP archive exist in the specified directory.");
//            return true; // Not an error; simply no files to delete
//        }
//
//        return true; // Files deleted successfully
//    }
//    catch (const std::exception& e) {
//        // Rollback on error
//        for (const std::string& file : deletedFiles) {
//            fs::path filePath = fs::path(basePath) / file;
//            std::ofstream recoveryFile(filePath.string());
//        }
//        showError("Error during file deletion: " + std::string(e.what()));
//        return false;
//    }
//}
