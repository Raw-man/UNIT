
#include "import.hpp"
#include "utils.hpp"
namespace unit {
bool ImportDistance(const ImportDisOpt& opt) {
  render::RenderConfig d;

  d.bloom_translucency = (u32)(255.0f - std::round(255.0f * opt.bloom));

  d.mipmap_bias = opt.bias;

  d.mipmap_slope = opt.slope;

  d.fog_near = opt.near_;

  d.fog_draw_distance_far = opt.far_;

  if (d.fog_draw_distance_far < d.fog_near)
    d.fog_draw_distance_far = d.fog_near;

  d.fog_color = nnl::utl::color::HexToInt(opt.color);

  d.distance_transition_translucency =
      (u32)(255.0f - std::round(255.0f * opt.transition));

  auto bin_asset = render::Export(d);

  utl::SaveFile(opt.output_path, bin_asset);

  return true;
}
}  // namespace unit
