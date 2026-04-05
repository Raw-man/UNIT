#include "import.hpp"
#include "logger.hpp"
#include "utils.hpp"
namespace unit {
bool ImportMinimap(const ImportMinOpt& opt) {
  SAsset3D sasset = SAsset3D::Import(opt.input_path);

  if (sasset.positions.empty())
    throw unit::RuntimeError(
        sasset.name +
        ": no anchor nodes were found. Make sure the glb file has a node called \"positions\" with "
        "anchors as its children.");

  if (sasset.positions.size() < 2)
    throw unit::RuntimeError(sasset.name + ": at least 2 anchor nodes are required (1 found)");

  UNIT_LOG_INFO(sasset.name + ":\n" + GetAssetStats(sasset, false));

  auto mini = minimap::Convert(sasset.positions, opt.texture_width);
  auto bin_mini = minimap::Export(mini);

  utl::SaveFile(opt.output_path, bin_mini);

  return true;
}
}  // namespace unit
