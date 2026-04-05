#include "locate.hpp"

#include <algorithm>
#include <functional>
#include <iostream>

#include "NNL/utility/string.hpp"
#include "utils.hpp"
namespace unit {

std::vector<std::size_t> SearchIn(const std::vector<unsigned char>& needle,
                                  const std::vector<unsigned char>& hay,
                                  std::size_t (*hash)(unsigned char c),
                                  bool (*pred)(unsigned char a,
                                               unsigned char b)) {
  if (needle.empty() || hay.empty()) return {};

  using res_iterator = std::vector<unsigned char>::const_iterator;

  std::vector<std::size_t> results;

  for (res_iterator hay_pos = hay.begin(); hay_pos != hay.end();) {
    if (pred == nullptr || hash == nullptr) {
      std::boyer_moore_horspool_searcher searcher(needle.begin(), needle.end());
      hay_pos = std::search(hay_pos, hay.end(), searcher);
    } else {
      std::boyer_moore_horspool_searcher searcher(needle.begin(), needle.end(),
                                                  hash, pred);
      hay_pos = std::search(hay_pos, hay.end(), searcher);
    }

    if (hay_pos != hay.end()) {
      std::size_t offset = std::distance(hay.begin(), hay_pos);
      results.push_back(offset);
      hay_pos += needle.size();
    }
  }

  return results;
}

void Traverse(const std::filesystem::path& input_path,
              std::function<void(const std::filesystem::path&)> callback) {
  using directory_iterator = std::filesystem::directory_iterator;

  auto dir_entries = directory_iterator(input_path);

  for (const std::filesystem::directory_entry& dir_entry : dir_entries) {
    if (dir_entry.is_directory()) {
      Traverse(dir_entry.path(), callback);
    } else {
      callback(dir_entry.path());
    }
  }
}

void Locate(const std::vector<unsigned char>& needle,
            const std::filesystem::path& search_path,
            std::size_t (*hash)(unsigned char c),
            bool (*compare)(unsigned char a, unsigned char b)) {
  std::stringstream stream;

  std::size_t num_matches = 0;

  const auto append_matches = [&](const std::filesystem::path& hay_path) {
    const auto haystack = utl::LoadFile(hay_path);

    const auto hits = SearchIn(needle, haystack, hash, compare);
    if (hits.empty()) return;
    num_matches += hits.size();
    std::string path_str = hay_path.u8string();

    stream << "\n" << path_str << ":\n";

    for (auto offset : hits) {
      std::string hex_offset = nnl::utl::string::IntToHex(offset, true, false);

      stream << "at " << hex_offset << "\n";
    }
  };

  if (std::filesystem::is_directory(search_path)) {
    Traverse(search_path, append_matches);

  } else if (std::filesystem::is_regular_file(search_path)) {
    append_matches(search_path);
  }

  std::cout << stream.str() << "\n" << std::endl;

  if (num_matches > 0)
    std::cout << num_matches << (num_matches > 1 ? " matches!" : " match!");
  else
    std::cout << "no matches!";

  std::cout << "\n\n";
}

}  // namespace unit
