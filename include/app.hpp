#pragma once
#include <CLI/CLI.hpp>
#include <cassert>

#include <string>

#include "app_options.hpp"
namespace fs = std::filesystem;

namespace unit {

class App : private CLI::App {
 public:
  static App& GetInstance() {
    static App instance{};
    return instance;
  }

  void Parse(int argc, char** argv);

  int GetExitErrCode(const CLI::Error& e) { return this->exit(e); }

  App(const App&) = delete;

  App& operator=(const App&) = delete;

  enum class LogType {
    kDebug,
    kInfo,
    kWarn,
    kError,
  };

  void Log(std::string_view msg, App::LogType type);

  void LogDebug(std::string_view msg);

  void LogInfo(std::string_view msg);

  void LogWarn(std::string_view msg);

  void LogError(std::string_view msg);

  void SetLogLevel(App::LogType level);

  bool IsLogLevelEnabled(App::LogType level);

 private:
  App();

  void SetUpGeneral();

  void SetUpSubcmdExport();

  void SetUpDragAndDrop();

  CLI::App* SetUpSubcmdImport();

  void SetUpSubcmdImpText(CLI::App* sub);

  void SetUpSubcmdImpDis(CLI::App* sub);

  void SetUpSubcmdImpFog(CLI::App* sub);

  void SetUpSubcmdImpImg(CLI::App* sub);

  void SetUpSubcmdImpSnd(CLI::App* sub);

  void SetUpSubcmdImpLit(CLI::App* sub);

  void SetUpSubcmdImpPos(CLI::App* sub);

  void SetUpSubcmdImpMinimap(CLI::App* sub);

  void SetUpSubcmdImpCam(CLI::App* sub);

  void SetUpSubcmdImpMdl(CLI::App* sub);

  void SetUpSubcmdUnpack();

  CLI::App* SetUpSubcmdPack();

  void SetUpSubcmdPackAsset(CLI::App* pack);

  void SetUpSubcmdPackCollection(CLI::App* pack);

  void SetUpSubcmdPackDigEntry(CLI::App* pack);

  void SetUpSubcmdPackDig(CLI::App* pack);

  void SetUpSubcmdMD5();

  CLI::App* SetUpSubcmdLocate();

  void SetUpSubcmdLocFile(CLI::App* loc);

  void SetUpSubcmdLocHex(CLI::App* loc);

  void SetUpSubcmdLocStr(CLI::App* loc);

  void SetUpSubcmdDetect();

  void RunSubcmdExport();

  void RunDragAndDropExport();

  void RunSubcmdImpMdl();

  void RunSubcmdImpCam();

  void RunSubcmdImpPos();

  void RunSubcmdImpMinimap();

  void RunSubcmdImpLit();

  void RunSubcmdImpSnd();

  void RunSubcmdImpImg();

  void RunSubcmdImpFog();

  void RunSubcmdImpDis();

  void RunSubcmdImpText();

  void RunSubcmdUnpack();

  void RunSubcmdPackAsset();

  void RunSubcmdPackCollection();

  void RunSubcmdPackDigEntry();

  void RunSubcmdPackDig();

  void RunSubcmdMD5();

  void RunSubcmdLocFile();

  void RunSubcmdLocStr();

  void RunSubcmdLocHex();

  void RunSubcmdDetect();

  bool print_config = false;

  LogType log_lvl = LogType::kInfo;

  std::set<std::string> input_paths;

  ExportOptions exp_opt;

  ImportMdlOpt imp_mdl_opt;

  ImportCamOpt imp_cam_opt;

  ImportPosOpt imp_pos_opt;

  ImportMinOpt imp_min_opt;

  ImportLitOpt imp_lit_opt;

  ImportTxtOpt imp_txt_opt;

  ImportSndOpt imp_snd_opt;

  ImportImgOpt imp_img_opt;

  ImportFogOpt imp_fog_opt;

  ImportDisOpt imp_dis_opt;

  UnpackOpt unp_opt;

  PackOpt pac_opt;

  MD5ListOpt md5_opt;

  LocateOpt loc_opt;

  DetectOpt det_opt;
};
}  // namespace unit
