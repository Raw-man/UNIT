#include "app.hpp"

#include <mutex>

#include "rang.hpp"

namespace unit {
std::mutex mut_cout;

static const CLI::App* GetCurrentSubcommand_(const CLI::App* app) {
  auto subcmd = app->get_subcommands();
  if (subcmd.empty()) {
    return app;
  }

  for (auto& sub : subcmd) return GetCurrentSubcommand_(sub);

  return nullptr;
}

void App::pre_callback() {
  auto current_sub = GetCurrentSubcommand_(this);

  if (current_sub != this && !this->input_paths.empty()) {
    throw CLI::ExtrasError("drag-and-drop", {(*input_paths.cbegin()).u8string()});
  }

  if (current_sub && this->print_config) PrintConfig(current_sub);
};

void App::Parse(int argc, char** argv) {
  if (argc < 2) {
    throw CLI::CallForHelp();
  }

  auto utf8_argv = this->ensure_utf8(argv);

  this->parse(argc, utf8_argv);

  if (this->get_option("drag-and-drop")->count() == 0 && this->get_subcommands().size() == 0) {
    throw CLI::RequiredError("A subcommand or a file path");
  }
}

void App::PrintConfig(const CLI::App* sub) {
  if (sub == nullptr) return;

  auto used_configs = this->config_ptr_->as<std::vector<std::string>>();
  std::string msg;
  if (!used_configs.empty()) msg += "used configs and ";

  msg += "current option values:\n";

  for (auto& config_path : used_configs) msg += config_path + "\n";

  msg += "\n" + sub->config_to_str(true);

  this->LogInfo(msg);
}

App::App() {
  SetUpGeneral();

  SetUpSubcmdExport();

  SetUpDragAndDrop();

  auto* sub = SetUpSubcmdImport();

  SetUpSubcmdImpMdl(sub);

  SetUpSubcmdImpCam(sub);

  SetUpSubcmdImpPos(sub);

  SetUpSubcmdImpMinimap(sub);

  SetUpSubcmdImpLit(sub);

  SetUpSubcmdImpText(sub);

  SetUpSubcmdImpSnd(sub);

  SetUpSubcmdImpImg(sub);

  SetUpSubcmdImpFog(sub);

  SetUpSubcmdImpDis(sub);

  SetUpSubcmdUnpack();

  auto* pack = SetUpSubcmdPack();

  SetUpSubcmdPackDig(pack);

  SetUpSubcmdPackDigEntry(pack);

  SetUpSubcmdPackCollection(pack);

  SetUpSubcmdPackAsset(pack);

  SetUpSubcmdMD5();

  auto* loc = SetUpSubcmdLocate();

  SetUpSubcmdLocFile(loc);

  SetUpSubcmdLocHex(loc);

  SetUpSubcmdLocStr(loc);

  SetUpSubcmdDetect();
}

bool App::IsLogLevelEnabled(App::LogType level) { return level >= log_lvl; }

void App::SetLogLevel(App::LogType level) {
  std::scoped_lock l(mut_cout);
  log_lvl = level;
}

void App::Log(std::string_view msg, App::LogType type) {
  if (!IsLogLevelEnabled(type)) return;

  std::scoped_lock l(mut_cout);

  switch (type) {
    case LogType::kError:
      std::cout << rang::fgB::red << "[ERROR]" << rang::fg::reset << " " << msg << std::endl;
      break;

    case LogType::kWarn:
      std::cout << rang::fgB::yellow << "[WARN]" << rang::fg::reset << " " << msg << std::endl;
      break;

    case LogType::kInfo:
      std::cout << rang::fgB::green << "[INFO]" << rang::fg::reset << " " << msg << std::endl;
      break;

    case LogType::kDebug:
      std::cout << rang::fgB::gray << rang::bg::yellow << "[DEBUG]" << rang::fg::reset << rang::bg::reset << " " << msg
                << std::endl;
      break;
  }
}

void App::LogDebug(std::string_view msg) { App::Log(msg, LogType::kDebug); }

void App::LogInfo(std::string_view msg) { App::Log(msg, LogType::kInfo); }

void App::LogWarn(std::string_view msg) { App::Log(msg, LogType::kWarn); }

void App::LogError(std::string_view msg) { App::Log(msg, LogType::kError); }

}  // namespace unit
