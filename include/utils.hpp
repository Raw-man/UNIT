#pragma once

#include <climits>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>


#ifdef linux
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <KnownFolders.h>
#include <Shlobj.h>
#include <initguid.h>
#endif

#include "NNL/common/io.hpp"
#include "exception.hpp" // IWYU pragma: export

namespace unit {

constexpr unsigned long long operator""_MiB(unsigned long long n) {
  return n * 1024 * 1024;
}

namespace utl {

namespace fs = std::filesystem;

std::string LoadTextFile(const std::filesystem::path& path);

std::vector<unsigned char> LoadFile(const std::filesystem::path& path);

void SaveFile(const std::filesystem::path& path, const std::vector<unsigned char>& buffer);

void SaveFile(const std::filesystem::path& path, const std::string& buffer);

void SaveFile(const std::filesystem::path& path, const nnl::BufferView& buffer);

void CreateDir(const std::filesystem::path& output_path);

std::vector<std::filesystem::path> GetSortedDirEntries(
    const std::filesystem::path& dir_path,
    const std::vector<std::string>& allowed_ext = {}, bool skip_hidden = true);

std::vector<std::filesystem::path> GetDirEntries(
    const std::filesystem::path& dir_path,
    const std::vector<std::string>& allowed_ext = {}, bool skip_hidden = true);

std::filesystem::path GetConfigFile(const std::string& appname,
                                    const std::string& content = "");

// extension is the substring after the first dot
std::filesystem::path ReplaceExtensionFront(
    const std::filesystem::path& path,
    const std::filesystem::path& new_extension);

std::string BytesToMegabytes(std::size_t size);

template <class Func>
struct Finally {
  explicit Finally(Func f) : act(f) {}
  ~Finally() { act(); }
  Func act;
};

}  // namespace utl
}  // namespace unit
