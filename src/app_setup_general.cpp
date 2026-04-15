#include "NNL/common/logger.hpp"
#include "app.hpp"
#include "app_formatter.hpp"
#include "logger.hpp"
#include "unit_version.hpp"
#include "utils.hpp"
#include "validators.hpp"

namespace unit {

void App::SetUpGeneral() {
#ifdef _WIN32
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
#endif

  auto formatter = std::make_shared<unit::Formatter>();

  formatter->label("TEXT:FILE", "PATH:FILE");

  formatter->label("TEXT:DIR", "PATH:DIR");

  formatter->label("TEXT:PATH", "PATH: DIR or FILE");

  formatter->label("TEXT:PATH(existing)", "PATH: DIR or FILE");

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

  nnl::SetGlobalLogCB([](const std::string_view msg, nnl::LogLevel lvl) {
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
