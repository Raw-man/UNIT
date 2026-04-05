#include "import.hpp"
#include "logger.hpp"
#include "utils.hpp"
namespace unit {
bool ImportPosition(const ImportPosOpt& opt) {
  SAsset3D sasset = SAsset3D::Import(opt.input_path);

  if (sasset.positions.empty())
    throw unit::RuntimeError(
        sasset.name +
        ": no position nodes were found. Make sure the glb file has a node called \"positions\" with children");

  UNIT_LOG_INFO(sasset.name + ":\n" + GetAssetStats(sasset, false));

  if (opt.scale_factor != 1.0f) sasset.Scale(opt.scale_factor);

  auto p = posd::Convert(sasset.positions);

  auto bin_asset = posd::Export(p);

  utl::SaveFile(opt.output_path, bin_asset);

  return true;
}
}  // namespace unit
