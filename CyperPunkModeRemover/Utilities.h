#pragma once
#ifndef UTILITIES_H
#define UTILITIES_H

#include <sstream>
#include <string>
#include <vector>
#include <Windows.h>

std::vector<std::string> listFilesInZip(const std::string& zipFilePath);
std::vector<std::string> listFilesInRar(const std::string& rarFilePath);
std::vector<std::string> listFilesInArchive(const std::string& filePath);

std::string runCommand(const std::string& command);

void showError(const std::string& message);
void showInfo(const std::string& message);
std::vector<std::string> listFilesInZip(const std::string& zipFilePath);
bool deleteFiles(const std::vector<std::string>& files, const std::string& basePath);
std::wstring stringToWstring(const std::string& str);


#endif // UTILITIES_H