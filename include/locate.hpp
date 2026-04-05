#pragma once

#include <filesystem>
#include <vector>
namespace unit {

void Locate(const std::vector<unsigned char>& needle,
            const std::filesystem::path& search_path,
            std::size_t (*hash)(unsigned char c) = nullptr,
            bool (*pred)(unsigned char a, unsigned char b) = nullptr);

}
