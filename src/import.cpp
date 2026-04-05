#include "import.hpp"

namespace unit {

std::string GetAssetStats(const SAsset3D& sasset, bool all) {
  auto bone_ref = sasset.model.skeleton.GetBoneRefs();

  std::size_t num_mesh_groups = 1;

  std::stringstream s;

  for (auto& smesh : sasset.model.meshes) {
    if (smesh.mesh_group + 1 > num_mesh_groups) num_mesh_groups = smesh.mesh_group + 1;
  }

  if (all || !sasset.model.meshes.empty()) s << "meshes: " << sasset.model.meshes.size() << "\n";

  // always at least 1
  if (all || !sasset.model.meshes.empty()) s << "mesh groups: " << num_mesh_groups << "\n";

  if (all || !sasset.textures.empty()) s << "textures: " << sasset.textures.size() << "\n";

  if (all || !sasset.model.materials.empty()) s << "materials: " << sasset.model.materials.size() << "\n";

  if (all || !sasset.model.material_variants.empty())
    s << "material variants: " << sasset.model.material_variants.size() << "\n";

  if (all) s << "bones: " << bone_ref.size() << "\n";

  if (all || !sasset.animations.empty()) s << "animations: " << sasset.animations.size() << "\n";

  if (all || !sasset.visibility_animations.empty())
    s << "visibility animations: " << sasset.visibility_animations.size() << "\n";

  if (all || !sasset.model.uv_animations.empty()) s << "uv animations: " << sasset.model.uv_animations.size() << "\n";

  if (all || !sasset.model.attachments.empty())
    s << "external attachments: " << sasset.model.attachments.size() << "\n";

  if (all || !sasset.lights.empty()) s << "lights: " << sasset.lights.size() << "\n";

  if (all || !sasset.positions.empty()) s << "positions: " << sasset.positions.size() << "\n";

  return s.str();
}

}  // namespace unit
