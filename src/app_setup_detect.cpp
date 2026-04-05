#include <CLI/CLI.hpp>

#include "app.hpp"
#include "detect.hpp"
#include "utils.hpp"
#include "validators.hpp"
namespace unit {

void App::RunSubcmdDetect() {
  auto buffer = utl::LoadFile(det_opt.input_path);
  auto res = format::DetectAll(buffer);
  std::string description;
  for (auto type : res) {
    description += GetExtension(type).substr(1) + ":\n" + GetDescription(type);
    if (type == format::kAssetContainer) {
      auto view = asset::ImportView(buffer);
      auto cat = asset::Categorize(view);
      description += "\n\nsubcategory - " + GetCatExtension(cat).substr(1) + ":\n";
      description += GetCatDescription(cat);
    }

    description += "\n\n\n";
  }

  std::cout << (!description.empty() ? "Detected the following types:\n\n" : "Unknown game asset type!") << description
            << std::endl;
}

void App::SetUpSubcmdDetect() {
  auto* sub = this->add_subcommand("det", "Detect and describe the asset type");

  sub->group("ADVANCED SUBCOMMANDS");

  sub->alias("detect");

  sub->usage("\nunit det <INPUT>");

  sub->add_option("-i,--input,input", det_opt.input_path, "An input path to a file to analyze")
      ->required()
      ->transform(NormalizePath)
      ->check(CLI::ExistingFile);

  sub->fallthrough(true);

  // block propagation
  sub->add_option("export-files-block-0", this->input_paths)
      ->group("")
      ->check([](const std::string& str) { return "The following argument was not expected: " + str; })
      ->configurable(false);

  sub->callback([this, sub]() {
    if (this->print_config) this->LogInfo("\n\n" + sub->config_to_str(true));
    this->RunSubcmdDetect();
  });
}
}  // namespace unit
