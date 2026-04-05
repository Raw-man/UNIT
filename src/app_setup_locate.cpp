#include <CLI/CLI.hpp>
#include <string>

#include "NNL/common/io.hpp"
#include "NNL/utility/string.hpp"
#include "NNL/utility/utf8.hpp"
#include "app.hpp"
#include "exception.hpp"
#include "locate.hpp"
#include "logger.hpp"
#include "validators.hpp"
namespace unit {

void App::RunSubcmdLocStr() {
  using namespace nnl;

  std::vector<unsigned char> needle(loc_opt.input_string.begin(), loc_opt.input_string.end());
  const auto CaselessComp = [](const unsigned char a, const unsigned char b) -> bool {
    return std::tolower(a) == std::tolower(b);
  };

  auto CaselessHash = [](unsigned char c) -> std::size_t {
    return std::hash<unsigned char>{}(std::tolower(static_cast<unsigned char>(c)));
  };

  if (loc_opt.caseless && utl::utf8::IsASCII(loc_opt.input_string)) {
    Locate(needle, loc_opt.search_path, CaselessHash, CaselessComp);
  } else {
    if (loc_opt.caseless) UNIT_LOG_WARN("--caseless not supported for UTF-8 strings");

    Locate(needle, loc_opt.search_path);
  }
}

void App::RunSubcmdLocHex() {
  std::vector<unsigned char> needle;

  needle.reserve(loc_opt.input_string.size());

  const auto hex_char_to_int = [](char c) -> unsigned char {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
  };

  auto tokens = nnl::utl::string::Split(loc_opt.input_string, " ");

  for (auto& token : tokens) {
    if (token.size() % 2 != 0) token = "0" + token;

    for (std::size_t i = 0; i < token.size(); i += 2) {
      char c0 = token[i];
      char c1 = token[i + 1];

      int n = hex_char_to_int(c0);
      int n1 = hex_char_to_int(c1);

      if (n == -1 || n1 == -1) throw unit::RuntimeError("invalid hex pattern");

      unsigned char byte = ((n & 0xF) << 4) | (n1 & 0xF);

      needle.push_back(byte);
    }
  }

  Locate(needle, loc_opt.search_path);
}

void App::RunSubcmdLocFile() {
  nnl::FileReader f{loc_opt.input_path};

  std::size_t needle_size = loc_opt.needle_size == 0 ? f.Len() : std::min<std::size_t>(f.Len(), loc_opt.needle_size);

  const auto needle = f.ReadArrayLE<unsigned char>(needle_size);

  f.Close();

  Locate(needle, loc_opt.search_path);
}

static void SetUpSubcmdLocShared(CLI::App* loc, std::filesystem::path& search_path) {
  loc->add_option("-p,--search-path,search-path", search_path, "A path to a file or a folder to search in")
      ->required()
      ->transform(NormalizePath)
      ->check(CLI::ExistingPath);
}

void App::SetUpSubcmdLocHex(CLI::App* loc) {
  auto sub_hex = loc->add_subcommand("hex", "Search for the hex pattern");

  sub_hex->add_option("-i,--input,input", loc_opt.input_string, "A hex pattern to search for")
      ->required()
      ->check(ValidHex);

  SetUpSubcmdLocShared(sub_hex, loc_opt.search_path);

  sub_hex->callback([this, sub_hex]() {
    if (this->print_config) this->LogInfo("\n\n" + sub_hex->config_to_str(true));
    this->RunSubcmdLocHex();
  });
}

void App::SetUpSubcmdLocStr(CLI::App* loc) {
  auto sub_str = loc->add_subcommand("str", "Search for the string");

  sub_str->add_option("-i,--input,input", loc_opt.input_string, "A string to search for")->required();

  SetUpSubcmdLocShared(sub_str, loc_opt.search_path);

  sub_str->add_flag("--caseless,!--no-caseless", loc_opt.caseless, "Ignore character case")->default_val(false);

  sub_str->callback([this, sub_str]() {
    if (this->print_config) this->LogInfo("\n\n" + sub_str->config_to_str(true));
    this->RunSubcmdLocStr();
  });
}

void App::SetUpSubcmdLocFile(CLI::App* loc) {
  auto sub_file = loc->add_subcommand("file", "Search for the file");

  sub_file->add_option("-i,--input,input", loc_opt.input_path, "A path to the file to search for")
      ->required()
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  SetUpSubcmdLocShared(sub_file, loc_opt.search_path);

  std::map<std::string, int> mapping{
      {"b", 1},            // bytes
      {"k", 1024},         // kilobytes
      {"kb", 1024},        // kilobytes
      {"m", 1024 * 1024},  // megabytes
      {"mb", 1024 * 1024}  // megabytes
  };

  sub_file
      ->add_option("--size", loc_opt.needle_size,
                   "Use only the first n bytes (or kb, mb) of the input file "
                   "for matching.")
      ->transform(CLI::AsNumberWithUnit(mapping, {}, "DATA UNIT"))
      ->check(CLI::PositiveNumber);

  sub_file->callback([this, sub_file]() {
    if (this->print_config) this->LogInfo("\n\n" + sub_file->config_to_str(true));
    this->RunSubcmdLocFile();
  });
}

CLI::App* App::SetUpSubcmdLocate() {
  auto* sub = this->add_subcommand("loc", "Locate a file, hex pattern, string within a file or a folder");

  sub->group("ADVANCED SUBCOMMANDS");

  sub->alias("locate");

  sub->usage("\nunit loc <SUBCOMMAND> <INPUT> <SEARCH_PATH> [OPTIONS]");

  sub->require_subcommand(1);

  auto hex_pattern = std::make_shared<std::string>();

  sub->fallthrough(true);

  // block propagation
  sub->add_option("export-files-block-3", this->input_paths)
      ->group("")
      ->check([](const std::string& str) { return "The following argument was not expected: " + str; })
      ->configurable(false);

  return sub;
}

}  // namespace unit
