
#include "utils.hpp"

#include <fstream>
#include <set>

#include "NNL/utility/string.hpp"

namespace unit {

namespace utl {

template <typename TContainer = std::vector<unsigned char>>
TContainer LoadFile_(const std::filesystem::path& path) {
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

std::vector<unsigned char> LoadFile(const std::filesystem::path& path) { return LoadFile_(path); }

std::string LoadTextFile(const std::filesystem::path& path) { return LoadFile_<std::string>(path); }

template <typename TContainer>
void SaveFile_(const std::filesystem::path& path, const TContainer& buffer) {
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

void SaveFile(const std::filesystem::path& path, const std::vector<unsigned char>& buffer) { SaveFile_(path, buffer); }

void SaveFile(const std::filesystem::path& path, const std::string& buffer) { SaveFile_(path, buffer); }

void SaveFile(const std::filesystem::path& path, const nnl::BufferView& buffer) { SaveFile_(path, buffer); }

void CreateDir(const std::filesystem::path& output_path) {
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

std::vector<std::filesystem::path> GetSortedDirEntries(const std::filesystem::path& dir_path,
                                                       const std::vector<std::string>& allowed_ext, bool skip_hidden) {
  std::set<std::string, decltype(&nnl::utl::string::CompareNat)> sorted_entries(&nnl::utl::string::CompareNat);

  for (const fs::directory_entry& dir_entry : fs::directory_iterator(dir_path)) {
    const auto& path = dir_entry.path();

    if (!allowed_ext.empty()) {
      std::string ext = nnl::utl::string::ToLower(path.extension().u8string());

      auto itr_ext = std::find(allowed_ext.begin(), allowed_ext.end(), ext);

      if (itr_ext == allowed_ext.end()) continue;
    }

    std::string name = path.filename().u8string();
    if (!skip_hidden || !nnl::utl::string::StartsWith(name, ".")) sorted_entries.insert(name);
  }

  std::vector<std::filesystem::path> entries;

  entries.reserve(sorted_entries.size());

  for (const auto& entry : sorted_entries) {
    entries.push_back(fs::absolute(dir_path / fs::u8path(entry)).lexically_normal());
  }

  return entries;
};

std::vector<std::filesystem::path> GetDirEntries(const std::filesystem::path& dir_path,
                                                 const std::vector<std::string>& allowed_ext, bool skip_hidden) {
  std::vector<std::filesystem::path> entries;

  for (const fs::directory_entry& dir_entry : fs::directory_iterator(dir_path)) {
    const auto& path = dir_entry.path();

    if (!allowed_ext.empty()) {
      std::string ext = nnl::utl::string::ToLower(path.extension().u8string());

      auto itr_ext = std::find(allowed_ext.begin(), allowed_ext.end(), ext);

      if (itr_ext == allowed_ext.end()) continue;
    }

    if (!skip_hidden || !nnl::utl::string::StartsWith(path.filename().u8string(), "."))
      entries.push_back(fs::absolute(path).lexically_normal());
  }

  return entries;
};

std::filesystem::path GetConfigFile(const std::string& appname, const std::string& content) {
  std::filesystem::path config_path;

#ifdef linux
  char* home = getenv("XDG_CONFIG_HOME");
  if (!home) {
    home = getenv("HOME");
    if (!home) {
      return config_path;
    }
  }

  config_path = home;

  config_path = config_path / std::filesystem::u8path(".config/");
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

  config_path = config_path / std::filesystem::u8path(appname).stem();

  bool created = std::filesystem::create_directories(config_path);

  config_path = config_path / std::filesystem::u8path(appname);

  if (created) {
    std::ofstream out(config_path);

    if (!out.is_open()) return std::filesystem::path();

    out << content;

    out.close();
  }

  return config_path;
}

std::filesystem::path ReplaceExtensionFront(const std::filesystem::path& path,
                                            const std::filesystem::path& new_extension) {
  auto new_name = path.filename().u8string();
  auto dot = new_name.find(".");
  if (dot != std::string::npos) {
    new_name = new_name.substr(0, dot);
  }

  return (path.parent_path() / std::filesystem::u8path(new_name)).replace_extension(new_extension);
}

std::string BytesToMegabytes(std::size_t size) {
  double megabytes = (double)size / (double)1_MiB;
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << megabytes;
  return stream.str() + "MB";
}

}  // namespace utl

}  // namespace unit
