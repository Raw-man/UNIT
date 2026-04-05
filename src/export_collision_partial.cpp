#include "export.hpp"
#include "utils.hpp"
namespace unit {
bool ExportCollisionPartial(const collision::Collision& collision,
                            const ExportOptions& options) {
  SAsset3D sasset;

  sasset.model = collision::Convert(collision);

  sasset.ExportGLB(
      utl::ReplaceExtensionFront(options.output_path, fs::u8path(".glb")),
      options.flip);

  return true;
}
}  // namespace unit
