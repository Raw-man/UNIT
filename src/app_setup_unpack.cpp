#include <CLI/CLI.hpp>

#include "app.hpp"
#include "unpack.hpp"
#include "utils.hpp"
#include "validators.hpp"
namespace unit {

void App::RunSubcmdUnpack() {
  const UnpackOpt& unp_opt = this->unp_opt;

  if (!Unpack(unp_opt)) {
    throw unit::RuntimeError("failed to unpack: " + unp_opt.input_path.u8string());
  }
}

void App::SetUpSubcmdUnpack() {
  auto* sub = this->add_subcommand("unpack", "Unpack various containers");

  sub->group("ADVANCED SUBCOMMANDS");

  sub->alias("unp");

  sub->fallthrough(true);

  sub->usage("\nunit unp <INPUT> <OUTPUT> [OPTIONS]");

  sub->add_option("-i,--input,input", unp_opt.input_path, "An input path to a container")
      ->required()
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);
  sub->add_option("-o,--output,output", unp_opt.output_path, "An output path to the resulting folder")
      ->required()
      ->transform(NormalizePath)
      ->check(ExistingParentPathDir)
      ->check(CLI::NonexistentPath);

  const std::vector<std::pair<std::string, UnpackOpt::Naming>> naming{{"default", UnpackOpt::Naming::kDefault},
                                                                      {"plugin", UnpackOpt::Naming::kPlugin},
                                                                      {"old_plugin", UnpackOpt::Naming::kOldPlugin}};

  sub->add_option("--naming", unp_opt.naming, "Set file names when unpacking primary .BIN archives")
      ->transform(EnumTransformer(naming))
      ->default_str(naming.front().first);

  sub->add_flag("--recursive,!--no-recursive", unp_opt.recursive,
                "Unpack files from the container that are also simple "
                "containers. --recursive=2 unpacks everything (not "
                "recommended). This option should be "
                "used consistently during both packing and unpacking.")
      ->default_val(unp_opt.recursive);

  sub->callback([this]() { this->RunSubcmdUnpack(); });
}

}  // namespace unit
