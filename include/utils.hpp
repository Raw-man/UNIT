#pragma once

#include <climits>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>


#include "NNL/common/io.hpp"
#include "exception.hpp" // IWYU pragma: export

namespace unit {

constexpr unsigned long long operator""_MiB(unsigned long long n) {
  return n * 1024 * 1024;
}

namespace fs = std::filesystem;

namespace utl {

fs::path NormalizePath(const fs::path& path);

std::string LoadTextFile(const fs::path& path);

std::vector<unsigned char> LoadFile(const fs::path& path);

void SaveFile(const fs::path& path, const std::vector<unsigned char>& buffer);

void SaveFile(const fs::path& path, const std::string& buffer);

void SaveFile(const fs::path& path, const nnl::BufferView& buffer);

void CreateDir(const fs::path& output_path);

std::vector<fs::path> GetDirEntries(const fs::path& dir_path, bool skip_hidden = true);

std::vector<fs::path> GetSortedDirEntries(const fs::path& dir_path, bool skip_hidden = true);

std::vector<fs::path> GetDirFiles(const fs::path& dir_path,
                                               const std::vector<std::string>& allowed_ext = {}, bool skip_hidden = true );

std::vector<fs::path> GetSortedDirFiles(const fs::path& dir_path,
                                                     const std::vector<std::string>& allowed_ext = {}, bool skip_hidden = true);

fs::path GetConfigFile(const std::string& appname,
                                    const std::string& content = "");

// extension is the substring after the first dot
fs::path ReplaceExtensionFront(
    const fs::path& path,
    const fs::path& new_extension);

std::string BytesToMegabytes(std::size_t size);

template <class Func>
struct Finally {
  explicit Finally(Func f) : act(f) {}
  ~Finally() { act(); }
  Func act;
};

}  // namespace utl
}  // namespace unit
