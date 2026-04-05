
#include "export.hpp"
#include "utils.hpp"
namespace unit {
bool ExportLit(const lit::Lit& lit, const ExportOptions& options) {
  SAsset3D sasset;

  sasset.lights = lit::Convert(lit);

  SLight& ambient_light = sasset.lights.emplace_back();

  ambient_light.name = "light_ambient";

  ambient_light.color = nnl::utl::color::IntToFloat(lit.global_ambient);

  ambient_light.direction = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));

  ambient_light.extras["ambient"] = true;

  sasset.ExportGLB(
      utl::ReplaceExtensionFront(options.output_path, fs::u8path(".glb")));

  return true;
}
}  // namespace unit
