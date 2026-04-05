
#include "export.hpp"
#include "utils.hpp"
namespace unit {

using namespace std::string_literals;

bool ExportDistance(const render::RenderConfig& d, const ExportOptions& options) {
  std::string toml =
      "[imp.dis]\n"
      "color = \"" +
      nnl::utl::color::IntToHex(d.fog_color, false) + "\"\n" +
      "near = " + std::to_string(d.fog_near) + "\n" +
      "far = " + std::to_string(d.fog_draw_distance_far) + "\n" +
      "bloom = " + std::to_string((255 - d.bloom_translucency) / 255.0f) +
      "\n" + "transition = " +
      std::to_string((255 - d.distance_transition_translucency) / 255.0f) +
      "\n" + "bias = " + std::to_string(d.mipmap_bias) + "\n" +
      "slope = " + std::to_string(d.mipmap_slope) + "\n";

  utl::SaveFile(
      utl::ReplaceExtensionFront(options.output_path, fs::u8path(".toml")),
      toml);

  return true;
}
}  // namespace unit
