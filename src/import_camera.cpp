#include "import.hpp"
#include "logger.hpp"
#include "utils.hpp"
namespace unit {
bool ImportCamera(const ImportCamOpt& opt) {
  asset::Asset asset_container;

  SAsset3D sasset = SAsset3D::Import(opt.input_path);

  if (sasset.animations.empty()) throw unit::RuntimeError(sasset.name + ": no animations were found");

  UNIT_LOG_INFO(sasset.name + ":\n" + GetAssetStats(sasset, false));

  auto& skeleton = sasset.model.skeleton;

  if (skeleton.roots.at(0).children.size() < 3)
    UNIT_LOG_WARN(sasset.name +
                  ": some camera animation channels ('VFX' nodes) are missing. Include "
                  "cameras when exporting");

  if (skeleton.roots.size() > 1 || skeleton.roots.at(0).children.size() > 3)
    UNIT_LOG_WARN(sasset.name + ": too many camera animation channels. Only 3 must be present");

  sasset.model.meshes.clear();

  SMesh def;

  def.indices = {0, 1, 2};

  SVertex v0, v1, v2;

  v0.position = {0.0f, 0.0f, 0.0f};
  v0.weights = {{0, 1.0f}};
  v1.position = {0.0f, 0.0f, -1.0f};
  v1.weights = v0.weights;
  v2.position = {-1.0f, 0.0f, 0.0f};
  v2.weights = v0.weights;

  def.vertices = {v0, v1, v2};

  sasset.model.meshes.push_back(def);

  sasset.model.materials.clear();

  sasset.model.materials.push_back({});

  model::ConvertParam param;

  param.indexed = false;

  std::vector<model::ConvertParam> params(sasset.model.meshes.size(), param);

  model::Model model = model::Convert(std::move(sasset.model), params, false);

  asset_container[asset::Asset3D::kModel] = model::Export(model);

  animation::ConvertParam anim_params;
  anim_params.unbake = opt.simplify;

  animation::AnimationContainer animation = animation::Convert(std::move(sasset.animations), anim_params);

  asset_container[asset::Asset3D::kAnimationContainer] = animation::Export(animation);

  auto bin_asset = asset::Export(asset_container);

  utl::SaveFile(opt.output_path, bin_asset);

  return true;
}

}  // namespace unit
