
#include "utils.hpp"

#ifdef linux
#include <unistd.h>
#endif

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <initguid.h>
#include <KnownFolders.h>
#include <Shlobj.h>
// clang-format on
#endif

#include <fstream>
#include <set>

#include "NNL/utility/string.hpp"

namespace unit {

namespace utl {

fs::path NormalizePath(const fs::path& path) {
  if (path.empty()) return path;

  auto full_path = std::filesystem::absolute(path);

  full_path = full_path.lexically_normal();

  // remove trailing slash
  if (!full_path.has_filename()) full_path = full_path.parent_path();

  return full_path;
}

template <typename TContainer = std::vector<unsigned char>>
TContainer LoadFile_(const fs::path& path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) return {};

  auto size = file.tellg();
  if (size <= 0) return {};

  TContainer buffer;
  buffer.resize(size);

  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char*>(buffer.data()), size);

  return buffer;
}

std::vector<unsigned char> LoadFile(const fs::path& path) { return LoadFile_(path); }

std::string LoadTextFile(const fs::path& path) { return LoadFile_<std::string>(path); }

template <typename TContainer>
void SaveFile_(const fs::path& path, const TContainer& buffer) {
  if (fs::is_directory(path))
    throw unit::RuntimeError(
        "failed to create the file since a directory with the same name "
        "already "
        "exists: " +
        path.u8string());

  std::ofstream file(path, std::ios::binary);

  if (!file.is_open()) {
    return;
  }

  file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

void SaveFile(const fs::path& path, const std::vector<unsigned char>& buffer) { SaveFile_(path, buffer); }

void SaveFile(const fs::path& path, const std::string& buffer) { SaveFile_(path, buffer); }

void SaveFile(const fs::path& path, const nnl::BufferView& buffer) { SaveFile_(path, buffer); }

void CreateDir(const fs::path& output_path) {
  bool success = false;
  std::string err_msg = "failed to create the directory";
  if (fs::exists(output_path)) {
    success = fs::is_directory(output_path);
    err_msg += " since a file with the same name already exists";
  } else {
    success = fs::create_directory(output_path);
  }

  if (!success) {
    throw unit::RuntimeError(err_msg + ": " + output_path.u8string());
  }
}

template <bool sort, bool files_only>
std::vector<fs::path> GetDirEntries_(const fs::path& dir_path, bool skip_hidden = true,
                                     const std::vector<std::string>& allowed_ext = {}) {
  std::vector<fs::path> entries;

  const fs::path dir_path_abs = NormalizePath(dir_path);

  for (const fs::directory_entry& dir_entry : fs::directory_iterator(dir_path_abs)) {
    fs::path path = dir_entry.path();

    if (files_only && !fs::is_regular_file(dir_entry.status())) continue;

    if (!allowed_ext.empty()) {
      std::string ext = nnl::utl::string::ToLower(path.extension().u8string());

      auto itr_ext = std::find(allowed_ext.begin(), allowed_ext.end(), ext);

      if (itr_ext == allowed_ext.end()) continue;
    }

    std::string name = path.filename().u8string();

    if (!skip_hidden || !nnl::utl::string::StartsWith(name, ".")) entries.push_back(std::move(path));
  }

  if constexpr (sort) {
    std::set<std::string, decltype(&nnl::utl::string::CompareNat)> sorted_entries(&nnl::utl::string::CompareNat);

    for (const auto& path : entries) sorted_entries.insert(path.filename().u8string());

    entries.clear();

    for (const auto& entry : sorted_entries) {
      entries.push_back(dir_path_abs / fs::u8path(entry));
    }
  }

  return entries;
};

std::vector<fs::path> GetDirEntries(const fs::path& dir_path, bool skip_hidden) {
  return GetDirEntries_<false, false>(dir_path, skip_hidden);
};

std::vector<fs::path> GetSortedDirEntries(const fs::path& dir_path, bool skip_hidden) {
  return GetDirEntries_<true, false>(dir_path, skip_hidden);
};

std::vector<fs::path> GetDirFiles(const fs::path& dir_path, const std::vector<std::string>& allowed_ext,
                                  bool skip_hidden) {
  return GetDirEntries_<false, true>(dir_path, skip_hidden, allowed_ext);
};

std::vector<fs::path> GetSortedDirFiles(const fs::path& dir_path, const std::vector<std::string>& allowed_ext,
                                        bool skip_hidden) {
  return GetDirEntries_<true, true>(dir_path, skip_hidden, allowed_ext);
};

fs::path GetConfigFile(const std::string& appname, const std::string& content) {
  fs::path config_path;

#ifdef linux
  char* home = getenv("XDG_CONFIG_HOME");
  if (!home) {
    home = getenv("HOME");
    if (!home) {
      return config_path;
    }
  }

  config_path = home;

  config_path = config_path / fs::u8path(".config/");
#endif

#ifdef _WIN32
  PWSTR home = nullptr;
  HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &home);

  if (hr != S_OK) return config_path;
  config_path = home;
#elif defined(CFGPATH_MAC)
  return config_path;
#endif

  if (config_path.empty()) return config_path;

  config_path = config_path / fs::u8path("unit");

  bool created = fs::create_directories(config_path);

  config_path = config_path / fs::u8path(appname);

  if (created || !fs::exists(config_path)) {
    std::ofstream out(config_path);

    if (!out.is_open()) return fs::path();

    out << content;

    out.close();
  }

  return config_path;
}

fs::path ReplaceExtensionFront(const fs::path& path, const fs::path& new_extension) {
  auto new_name = path.filename().u8string();
  auto dot = new_name.find(".");
  if (dot != std::string::npos) {
    new_name = new_name.substr(0, dot);
  }

  return (path.parent_path() / fs::u8path(new_name)).replace_extension(new_extension);
}

std::string BytesToMegabytes(std::size_t size) {
  double megabytes = (double)size / (double)1_MiB;
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << megabytes;
  return stream.str() + "MB";
}

}  // namespace utl

}  // namespace unit
