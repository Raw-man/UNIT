
#include "export.hpp"
#include "utils.hpp"
namespace unit {

using namespace std::string_literals;

bool ExportFog(const fog::Fog& fog, const ExportOptions& options) {
  std::string toml = "[imp.fog]\n"s + "color = \"" +
                     nnl::utl::color::IntToHex(fog.color, false) + "\"\n" +
                     "near = " + std::to_string(fog.near_) + "\n" +
                     "far = " + std::to_string(fog.far_) + "\n";

  utl::SaveFile(
      utl::ReplaceExtensionFront(options.output_path, fs::u8path(".toml")),
      toml);

  return true;
}
}  // namespace unit
