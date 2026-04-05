#include "NNL/common/logger.hpp"
#include "app.hpp"
#include "logger.hpp"
#include "unit_version.hpp"
#include "utils.hpp"
#include "validators.hpp"
namespace unit {

class Formatter : public CLI::Formatter {
  CLI11_INLINE std::string make_group(std::string group, bool is_positional,
                                      std::vector<const CLI::Option *> opts) const override {
    std::stringstream out;

    out << "\n" << group << ":\n";
    for (const CLI::Option *opt : opts) {
      out << (is_positional ? make_positional_option(opt) : make_option(opt, false));
    }

    return out.str();
  }

  std::string make_positional_option(const CLI::Option *opt) const {
    std::stringstream out;

    auto fn = opt->get_fnames();
    auto sn = opt->get_snames();
    auto ln = opt->get_lnames();
    bool is_positional_only = fn.empty() && sn.empty() && ln.empty();

    const std::string left = "  " + make_option_name(opt, true) + make_option_opts(opt);
    const std::string desc = is_positional_only ? make_option_desc(opt) : "";

    out << std::setw(static_cast<int>(column_width_)) << std::left << left;

    if (!desc.empty()) {
      bool skipFirstLinePrefix = true;
      if (left.length() >= column_width_) {
        out << '\n';
        skipFirstLinePrefix = false;
      }
      CLI::detail::streamOutAsParagraph(out, desc, right_column_width_, std::string(column_width_, ' '),
                                        skipFirstLinePrefix);
    }

    out << '\n';
    return out.str();
  }
};

void App::SetUpGeneral() {
#ifdef _WIN32
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
#endif

  auto formatter = std::make_shared<unit::Formatter>();

  this->formatter(formatter);

  this->description(
      "Ultimate Ninja Impact Toolbox: a CLI program for modding NSUNI and "
      "NSLAR\n(" UNIT_BUILD_STR ")");

  this->require_subcommand(0, 1);  // Require 0 or 1 subcommands

  this->set_version_flag("-v,--version", UNIT_BUILD_STR);

  this->usage(
      "\nunit <SUBCOMMAND> [<SUBCOMMAND>] <INPUT> <OUTPUT> [OPTIONS]\n"
      "\nunit <FILE1> [<FILE2>...] [OPTIONS]\n\n"
      "Use --help with a subcommand to get more info\n");

  std::string config_path_global =
      unit::utl::GetConfigFile("unit.toml",
                               "#This is a global config."
                               "\n#A local config can be placed into the program's current folder. "
                               "In that case this one will be ignored."
                               "\n#Uncomment the following to set new global settings"
                               "\n#print-config=true"
                               "\n#log-lvl=\"debug\""
                               "\n#[imp.mdl]"
                               "\n#merge-mat=true"
                               "\n#indexed=false"
                               "\n#sort=true"
                               "\n#blend=true"
                               "\n#[exp]"
                               "\n#visibility=true")
          .parent_path()
          .u8string();

  std::string config_path_local = std::filesystem::current_path().u8string();

  this->set_config("--config", "unit.toml",
                   "Read options from a toml config. The default config can be "
                   "placed here:\n" +
                       config_path_global + "\n" + config_path_local)
      ->transform(CLI::FileOnDefaultPath(config_path_global, false))
      ->capture_default_str();

  this->footer(
      "\nhttps://bit.ly/rcjn-cli\n"
      "\nhttps://www.youtube.com/RomanFirst\n"
      "\n1romanfirst@gmail.com\n"
      "\nhttps://github.com/Raw-man/UNIT");

  auto default_formatter = this->get_formatter();

  default_formatter->enable_default_flag_values(false);

  const std::vector<std::pair<std::string, App::LogType>> log_type_str = {{"debug", App::LogType::kDebug},
                                                                          {"info", App::LogType::kInfo},
                                                                          {"warn", App::LogType::kWarn},
                                                                          {"error", App::LogType::kError}};

  this->add_option("--log-lvl", this->log_lvl, "Set logging level")
      ->transform(EnumTransformer(log_type_str))
      ->capture_default_str();

  this->add_flag("--print-config,!--no-print-config", this->print_config, "Print options and their current values")
      ->default_val(false);

  this->fallthrough(true);

  nnl::SetGlobalLogCB([](const std::string &msg, nnl::LogLevel lvl) {
    switch (lvl) {
      case nnl::LogLevel::kWarn:
        UNIT_LOG_WARN(msg);
        break;
      case nnl::LogLevel::kError:
        UNIT_LOG_ERROR(msg);
        break;
    }
  });
}

}  // namespace unit
