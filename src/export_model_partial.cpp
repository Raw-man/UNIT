#include "export.hpp"
#include "utils.hpp"
namespace unit {
bool ExportModelPartial(const model::Model& model,
                        const ExportOptions& options) {
  SAsset3D sasset;

  sasset.model = model::Convert(model);

  sasset.ExportGLB(
      utl::ReplaceExtensionFront(options.output_path, fs::u8path(".glb")),
      options.flip, options.pack_textures);

  return true;
}
}  // namespace unit
