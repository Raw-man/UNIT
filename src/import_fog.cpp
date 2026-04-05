#include "import.hpp"
#include "utils.hpp"
namespace unit {
bool ImportFog(const ImportFogOpt& opt) {
  fog::Fog f;

  f.color = nnl::utl::color::HexToInt(opt.color);
  f.near_ = opt.near_;
  f.far_ = opt.far_;

  if (f.near_ > f.far_) f.far_ = f.near_;

  auto bin_asset = fog::Export(f);

  utl::SaveFile(opt.output_path, bin_asset);

  return true;
}
}  // namespace unit
