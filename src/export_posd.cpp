#include "export.hpp"
#include "utils.hpp"
namespace unit {
bool ExportPositionData(const posd::PositionData& positions,
                        const ExportOptions& options) {
  SAsset3D sasset;

  sasset.positions = posd::Convert(positions);

  sasset.ExportGLB(
      utl::ReplaceExtensionFront(options.output_path, fs::u8path(".glb")));

  return true;
}
}  // namespace unit
