#include "NNL/common/logger.hpp"
#include "app.hpp"
#include "app_formatter.hpp"
#include "logger.hpp"
#include "unit_version.hpp"
#include "utils.hpp"
#include "validators.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace unit {

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

  this->footer(
      "\nhttps://bit.ly/rcjn-cli\n"
      "\nhttps://www.youtube.com/RomanFirst\n"
      "\n1romanfirst@gmail.com\n"
      "\nhttps://github.com/Raw-man/UNIT");

  auto config_path_global =
      unit::utl::GetConfigFile("unit.toml",
                               "# These are global configuration settings.\n"
                               "# You can provide additional configuration files using the --config option.\n"
                               "# Uncomment any lines below to apply new values.\n"
                               "#print-config=true\n"
                               "#log-lvl=\"debug\"\n"
                               "#[imp.mdl]\n"
                               "#merge-mat=true\n"
                               "#indexed=false\n"
                               "#sort=true\n"
                               "#blend=true\n"
                               "#[exp]\n"
                               "#visibility=true");

  const std::vector<std::pair<std::string, App::LogType>> log_type_str = {{"debug", App::LogType::kDebug},
                                                                          {"info", App::LogType::kInfo},
                                                                          {"warn", App::LogType::kWarn},
                                                                          {"error", App::LogType::kError}};

  this->add_option("--log-lvl", this->log_lvl, "Set logging level")
      ->transform(EnumTransformer(log_type_str))
      ->capture_default_str();

  auto conf_opt = this->set_config("--config", "",
                                   "Read options from a toml config. The default config can be "
                                   "placed here:\n" +
                                       config_path_global.parent_path().u8string())
                      ->capture_default_str()
                      ->multi_option_policy(CLI::MultiOptionPolicy::Reverse)
                      ->expected(1, -1);

  this->add_flag(
          "--global-config,!--no-global-config",
          [conf_opt, config_path_global](std::int64_t count_true) {
            if (count_true >= 0 && std::filesystem::is_regular_file(config_path_global)) {
              auto results = conf_opt->reduced_results();

              conf_opt->clear();

              conf_opt->add_result(config_path_global.u8string());

              for (auto res = std::rbegin(results); res != std::rend(results); ++res) {
                conf_opt->add_result(std::move(*res));
              }
            }
          },
          "Use or ignore the global config")
      ->callback_priority(CLI::CallbackPriority::First)
      ->force_callback(true)
      ->default_val(true)
      ->configurable(false);

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
