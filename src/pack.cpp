#include "pack.hpp"

#include "logger.hpp"
#include "threadpool.hpp"
#include "utils.hpp"
namespace unit {

nnl::Buffer PackRecursively(const fs::path& conf_path) {
  std::string toml_str = utl::LoadTextFile(conf_path);
  CLI::ConfigTOML toml_conf;
  std::istringstream strm{toml_str};
  auto items = toml_conf.from_config(strm);
  fs::path input_path;
  std::string cmd;
  bool recursive = false;
  for (const auto& item : items) {
    // get command and subcommand e.g. [pac.ast]
    if (item.name == "++" && item.parents.size() >= 2 && (item.parents[0] == "pac" || item.parents[0] == "pack")) {
      cmd = item.parents[1];
    }
    // get the input option and its value (the dir path)
    if (item.name == "input" && !item.inputs.empty() && !item.parents.empty() &&
        (item.parents[0] == "pac" || item.parents[0] == "pack"))
      input_path = fs::u8path(item.inputs[0]);

    if (item.name == "recursive" && !item.inputs.empty() && !item.parents.empty() &&
        (item.parents[0] == "pac" || item.parents[0] == "pack"))
      recursive = (item.inputs[0] == "1" || item.inputs[0] == "true");
  }

  if (!input_path.empty() && !cmd.empty() && fs::is_directory(input_path)) {
    PackOpt sub_opt;
    sub_opt.recursive = recursive;

    if (input_path.is_relative()) {
      input_path = conf_path.parent_path() / input_path;
    }

    input_path = input_path.lexically_normal();

    sub_opt.input_path = input_path;
    if (cmd == "ast" || cmd == "asset") {
      return PackAsset(sub_opt);
    } else if (cmd == "col" || cmd == "collection") {
      return PackCollection(sub_opt);
    } else if (cmd == "ent" || cmd == "entry") {
      return PackDigEntry(sub_opt);
    }
  }

  throw unit::RuntimeError(
      "failed to pack recursively. The config does not provide a valid "
      "subcommand or an input "
      "directory path " +
      conf_path.u8string());

  return {};
}

std::pair<bool, nnl::Buffer> ProcessEntry(const fs::path& input_file_path, const PackOpt& opt) {
  if (fs::is_directory(input_file_path)) {
    if (!opt.recursive) {
      UNIT_LOG_WARN("the directory was ignored since --recursive was not provided:" + input_file_path.u8string());
      return {false, {}};
    }

    fs::path conf_path = input_file_path / fs::u8path(".toml");

    if (fs::exists(conf_path)) {
      nnl::Buffer buf = PackRecursively(conf_path);
      return {true, std::move(buf)};
    } else {
      UNIT_LOG_WARN("\".toml\" config was not found, ignored the folder " + input_file_path.u8string());
      return {false, {}};
    }

  } else if (fs::is_regular_file(input_file_path)) {
    nnl::Buffer buf = utl::LoadFile(input_file_path);

    return {true, std::move(buf)};
  }
  UNIT_LOG_WARN("something went wrong " + input_file_path.u8string());
  return {false, {}};
}

nnl::Buffer PackAsset(const PackOpt& opt) {
  auto entry_files = utl::GetSortedDirEntries(opt.input_path);

  if (entry_files.empty()) throw unit::RuntimeError("no files found in the directory " + opt.input_path.u8string());

  if (!opt.output_path.empty()) UNIT_LOG_INFO("num entries: " + std::to_string(entry_files.size()));

  asset::Asset s;

  std::size_t i = 0;

  for (const auto& input_file_path : entry_files) {
    char* posa_p = nullptr;

    std::size_t k = i;

    auto entry_name = input_file_path.filename().u8string();

    if (entry_name[0] >= '0' && entry_name[0] <= '9') k = std::strtoul(entry_name.c_str(), &posa_p, 10);

    auto [success, buf] = ProcessEntry(input_file_path, opt);

    if (success) s[k] = std::move(buf);
  }

  auto bin_asset = asset::Export(s);

  if (bin_asset.size() > 30_MiB)
    UNIT_LOG_WARN("the output file is too big (" + utl::BytesToMegabytes(bin_asset.size()) +
                  ") and might not be loaded");

  return bin_asset;
}

nnl::Buffer PackCollection(const PackOpt& opt) {
  auto entry_files = utl::GetSortedDirEntries(opt.input_path);

  if (entry_files.empty()) throw unit::RuntimeError("no files found in the directory " + opt.input_path.u8string());

  if (!opt.output_path.empty()) UNIT_LOG_INFO("num entries: " + std::to_string(entry_files.size()));

  collection::Collection s;

  s.reserve(entry_files.size());

  for (const auto& input_file_path : entry_files) {
    auto [success, buf] = ProcessEntry(input_file_path, opt);

    if (success) s.push_back(std::move(buf));
  }

  return collection::Export(s);
}

nnl::Buffer PackDigEntry(const PackOpt& opt) {
  auto entry_files = utl::GetSortedDirEntries(opt.input_path);

  if (entry_files.empty()) throw unit::RuntimeError("no files found in the directory " + opt.input_path.u8string());

  if (!opt.output_path.empty()) UNIT_LOG_INFO("num entries: " + std::to_string(entry_files.size()));

  dig_entry::DigEntry s;

  s.reserve(entry_files.size());

  for (const auto& input_file_path : entry_files) {
    auto [success, buf] = ProcessEntry(input_file_path, opt);

    if (success) s.push_back(std::move(buf));
  }

  return dig_entry::Export(s);
}

nnl::Buffer PackDig(const PackOpt& opt) {
  auto entry_files = utl::GetSortedDirEntries(opt.input_path);

  if (entry_files.empty()) throw unit::RuntimeError("no files found in the directory " + opt.input_path.u8string());

  if (entry_files.size() < 32)
    throw unit::RuntimeError("not enough entries for a dig archive " + opt.input_path.u8string());

  UNIT_LOG_INFO("num entries: " + std::to_string(entry_files.size()));

  dig::Dig s;

  s.reserve(entry_files.size());

  for (const auto& input_file_path : entry_files) {
    auto [success, buf] = ProcessEntry(input_file_path, opt);

    if (!success) {
      continue;
    }

    dig::FileRecord rec;

    if (dig_entry::IsOfType(buf)) {
      auto res = dig_entry::ImportView(buf);
      rec.num_entries = static_cast<u16>(res.size());
    } else if (!buf.empty()) {
      throw unit::RuntimeError("not a dig entry archive: " + input_file_path.u8string());
    }

    rec.decompressed_size = buf.size();

    rec.buffer = std::move(buf);

    s.push_back(std::move(rec));
  }

  if (opt.compress) {
    std::vector<std::future<void>> tasks;

    ThreadPool pool(std::max(std::thread::hardware_concurrency(), 1U));

    UNIT_LOG_DEBUG("threads: " + std::to_string(pool.GetNumThreads()));

    for (std::size_t i = 0; i < s.size(); i++) {
      auto& rec = s[i];
      if (!rec.buffer.empty()) {
        tasks.push_back(pool.AddTask([&rec, i]() {
          auto comp_buf = dig::Compress(rec.buffer);

          if (comp_buf.size() < rec.buffer.size()) {
            rec.is_compressed = true;
            rec.buffer = std::move(comp_buf);
          } else {
            UNIT_LOG_DEBUG("compression was ignored for the entry " + std::to_string(i));
          }
        }));
      }
    }

    UNIT_LOG_DEBUG("tasks: " + std::to_string(tasks.size()));

    for (std::size_t i = 0; i < tasks.size(); i++) {
      tasks[i].get();
      UNIT_LOG_DEBUG("task completed: " + std::to_string(i));
    }
  }

  return dig::Export(s);
}

}  // namespace unit
