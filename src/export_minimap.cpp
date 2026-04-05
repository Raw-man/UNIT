
#include "export.hpp"
#include "logger.hpp"
#include "utils.hpp"
namespace unit {

using namespace std::string_literals;

bool ExportMinimap(const minimap::MinimapConfig& minimap, const ExportOptions& options) {
  auto bin_texture = utl::LoadFile(options.base_path);

  if (!texture::IsOfType(bin_texture)) {
    UNIT_LOG_ERROR("the base asset is not a texture container: " + options.base_path.u8string());
    return false;
  }

  SAsset3D sminimap;

  auto textures = texture::Import(bin_texture);

  auto& stexture = sminimap.textures.emplace_back(texture::Convert(textures.at(0)));

  sminimap.positions = minimap::Convert(minimap, stexture.width);

  auto& top_left = sminimap.positions.at(0);
  auto& top_right = sminimap.positions.at(1);

  auto& smat = sminimap.model.materials.emplace_back();

  smat.name = "dummy_material";
  smat.texture_id = 0;
  smat.lit = false;
  smat.ambient = glm::vec3{1.0f};

  auto& splane = sminimap.model.meshes.emplace_back();

  splane.name = "dummy_visual_reference_ortho_scale_";

  splane.name += nnl::utl::string::FloatToString((float)stexture.width / minimap.pixels_per_unit, 3);

  SVertex v;

  v.weights = {{0, 1.0f}};

  v.position = top_left.translation;  // 0

  v.uv = {0.0f, 0.0f};

  splane.vertices.push_back(v);

  v.position = {v.position.x, v.position.y, v.position.z + ((float)stexture.height / minimap.pixels_per_unit)};  // 1

  v.uv = {0.0f, 1.0f};

  splane.vertices.push_back(v);

  v.position = top_right.translation;  // 2

  v.uv = {1.0f, 0.0f};

  splane.vertices.push_back(v);

  v.position = {v.position.x, v.position.y, v.position.z + ((float)stexture.height / minimap.pixels_per_unit)};  // 3

  v.uv = {1.0f, 1.0f};

  splane.vertices.push_back(v);

  splane.indices = {0, 1, 2, 2, 1, 3};

  auto new_path = unit::utl::ReplaceExtensionFront(options.output_path, fs::u8path(".glb"));

  sminimap.ExportGLB(new_path, false, options.pack_textures);

  return true;
}
}  // namespace unit
