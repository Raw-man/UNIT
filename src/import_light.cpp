#include "import.hpp"
#include "logger.hpp"
#include "utils.hpp"
namespace unit {
bool ImportLight(const ImportLitOpt& opt) {
  SAsset3D sasset = SAsset3D::Import(opt.input_path);

  if (sasset.lights.empty()) throw unit::RuntimeError(sasset.name + ": no lights were found");

  UNIT_LOG_INFO(sasset.name + ":\n" + GetAssetStats(sasset, false));

  std::vector<SLight> lights;

  lights.reserve(4);

  glm::vec3 ambient{0.0f};

  for (auto& slight : sasset.lights)
    if (!slight.extras.Has("ambient")) {
      lights.push_back(slight);
    } else {
      UNIT_LOG_INFO("found an ambient light");
      ambient = slight.color;
    }

  auto l = lit::Convert(lights, ambient, opt.specular, opt.character_brightness);

  auto bin_asset = lit::Export(l);

  utl::SaveFile(opt.output_path, bin_asset);

  return true;
}
}  // namespace unit
