#include <CLI/CLI.hpp>
#include <CLI/Config.hpp>

#include "app.hpp"
#include "pack.hpp"
#include "utils.hpp"
#include "validators.hpp"
namespace unit {

void App::RunSubcmdPackAsset() {
  auto& pac_opt = this->pac_opt;
  auto res = PackAsset(pac_opt);
  utl::SaveFile(pac_opt.output_path, res);
}

void App::RunSubcmdPackCollection() {
  auto& pac_opt = this->pac_opt;
  auto res = PackCollection(pac_opt);
  utl::SaveFile(pac_opt.output_path, res);
}

void App::RunSubcmdPackDigEntry() {
  auto& pac_opt = this->pac_opt;
  auto res = PackDigEntry(pac_opt);
  utl::SaveFile(pac_opt.output_path, res);
}

void App::RunSubcmdPackDig() {
  auto& pac_opt = this->pac_opt;
  auto res = PackDig(pac_opt);
  utl::SaveFile(pac_opt.output_path, res);
}

void App::SetUpSubcmdPackDig(CLI::App* pack) {
  auto* sub = pack->add_subcommand("cfcdig", "Pack the main data archive (.BIN file)");
  sub->alias("dig");
  sub->alias("bin");
  sub->fallthrough(true);

  sub->usage(
      "This container represents the main (or one of the main) game archives "
      "(*.BIN or CFC.DIG files) that store all assets. Note: ANY changes to "
      "them may BREAK the games. In NSUNI, the archives are encrypted (and "
      "stored in a .PGD container). Integrity checks are performed on their "
      "entries using MD5 hashes (from .md5 files) "
      "The general structure of those entries is fixed and should be "
      "preserved. Archives that store the game's audio are "
      "expected to be at certain fixed LBA positions in the .iso file. "
      "In general, the games "
      "may expect files not to exceed certain fixed sizes. Some of these "
      "issues can be avoided by patching the game's executable. See "
      "https://bit.ly/NSUNI2020_7Z");

  auto& pac_opt = this->pac_opt;
  sub->add_option("-i,--input,input", pac_opt.input_path, "An input path to a folder with files to pack")
      ->required()
      ->transform(NormalizePath)
      ->check(CLI::ExistingDirectory);
  sub->add_option("-o,--output,output", pac_opt.output_path, "An output path to the resulting archive")
      ->required()
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub->add_flag("--compress,!--no-compress", pac_opt.compress,
                "Compress the archive entries if possible. Note: slows down loading")
      ->default_val(false);

  sub->callback([this, sub]() {
    if (this->print_config) this->LogInfo("\n\n" + sub->config_to_str(true));
    this->RunSubcmdPackDig();
  });
}

void App::SetUpSubcmdPackDigEntry(CLI::App* pack) {
  auto* sub = pack->add_subcommand("entry", "Pack an entry of the main game archive");
  sub->alias("ent");
  sub->fallthrough(true);

  sub->usage(
      "This container is an entry in the main game archive that often "
      "stores multiple related assets. Note: careless "
      "changes may "
      "result in crashes.");

  auto& pac_opt = this->pac_opt;
  sub->add_option("-i,--input,input", pac_opt.input_path, "An input path to a folder with files to pack")
      ->required()
      ->transform(NormalizePath)
      ->check(CLI::ExistingDirectory);
  sub->add_option("-o,--output,output", pac_opt.output_path, "An output path to the resulting container")
      ->required()
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub->callback([this, sub]() {
    if (this->print_config) this->LogInfo("\n\n" + sub->config_to_str(true));
    this->RunSubcmdPackDigEntry();
  });
}

void App::SetUpSubcmdPackCollection(CLI::App* pack) {
  auto* sub = pack->add_subcommand("collection", "Pack a collection of similar assets.");
  sub->alias("col");
  sub->fallthrough(true);

  sub->usage(
      "This container often holds multiple related but distinct assets (for "
      "instance, a main 3d map and additional 3d props). Note: careless "
      "changes may "
      "result in crashes.");

  auto& pac_opt = this->pac_opt;
  sub->add_option("-i,--input,input", pac_opt.input_path, "An input path to a folder with files to pack")
      ->required()
      ->transform(NormalizePath)
      ->check(CLI::ExistingDirectory);
  sub->add_option("-o,--output,output", pac_opt.output_path, "An output path to the resulting container")
      ->required()
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub->callback([this, sub]() {
    if (this->print_config) this->LogInfo("\n\n" + sub->config_to_str(true));
    this->RunSubcmdPackCollection();
  });
}

void App::SetUpSubcmdPackAsset(CLI::App* pack) {
  auto* sub = pack->add_subcommand("asset", "Pack parts of a complete asset");
  sub->alias("ast");

  sub->usage(
      "An asset container holds closely related components of a complete asset "
      "(for instance, textures and meshes of a 3d model). Note: these parts "
      "often "
      "reference each other or may be required by the game to function "
      "properly. Careless changes may result in crashes.");

  sub->fallthrough(true);
  auto& pac_opt = this->pac_opt;
  sub->add_option("-i,--input,input", pac_opt.input_path, "An input path to a folder with files to pack")
      ->required()
      ->transform(NormalizePath)
      ->check(CLI::ExistingDirectory);
  sub->add_option("-o,--output,output", pac_opt.output_path, "An output path to the resulting container")
      ->required()
      ->transform(NormalizePath)
      ->check(ExistingParentPathFile);

  sub->callback([this, sub]() {
    if (this->print_config) this->LogInfo("\n\n" + sub->config_to_str(true));
    this->RunSubcmdPackAsset();
  });
}

CLI::App* App::SetUpSubcmdPack() {
  auto* sub = this->add_subcommand("pack", "Pack various containers");

  sub->group("ADVANCED SUBCOMMANDS");

  sub->alias("pac");

  sub->fallthrough(true);

  sub->usage(
      "\nunit pac <SUBCOMMAND> <INPUT> <OUTPUT> [OPTIONS]\n\n"
      "Note: numeric prefixes in the names of archive entries must be "
      "preserved. Any careless changes to those entries may result in "
      "crashes. Repacking without any changes may not always create an "
      "identical archive.\n"
      "Use --help with a subcommand for more details");

  sub->add_flag("--recursive,!--no-recursive", pac_opt.recursive,
                "Pack subfolders as nested containers first. This option should be "
                "used consistently during both packing and unpacking.")
      ->default_val(false);

  // block propagation
  sub->add_option("export-files-block-5", this->input_paths)
      ->group("")
      ->check([](const std::string& str) { return "The following argument was not expected: " + str; })
      ->configurable(false);

  sub->require_subcommand(1);

  return sub;
}

}  // namespace unit
