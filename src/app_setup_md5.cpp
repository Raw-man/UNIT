#include <CLI/CLI.hpp>

#include "NNL/game_asset/container/md5list.hpp"
#include "app.hpp"
#include "utils.hpp"
#include "validators.hpp"
namespace unit {

using namespace nnl;

void App::RunSubcmdMD5() {
  nnl::Buffer bin_dig = utl::LoadFile(md5_opt.input_path);
  auto dig_view = dig::ImportView(bin_dig);
  auto md5list = md5list::Generate(dig_view);
  nnl::Buffer bin_md5list = md5list::Export(md5list);

  utl::SaveFile(md5_opt.output_path, bin_md5list);
}

void App::SetUpSubcmdMD5() {
  auto* sub = this->add_subcommand("md5", "Generate an md5 hashsum list for a .BIN archive");

  sub->group("ADVANCED SUBCOMMANDS");

  sub->alias("md5list");

  sub->usage("\nunit md5 <INPUT> <OUTPUT>");

  sub->add_option("-i,--input,input", md5_opt.input_path, "An input path to a primary data archive (.BIN file)")
      ->required()
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  sub->add_option("-o,--output,output", md5_opt.output_path, "An output path to the resulting .md5 file")
      ->required()
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub->fallthrough(true);

  // block propagation
  sub->add_option("export-files-block-4", this->input_paths)
      ->group("")
      ->check([](const std::string& str) { return "The following argument was not expected: " + str; })
      ->configurable(false);

  sub->callback([this, sub]() {
    if (this->print_config) this->LogInfo("\n\n" + sub->config_to_str(true));
    this->RunSubcmdMD5();
  });
}
}  // namespace unit
