
#include "export.hpp"

namespace unit {
bool ExportPPHD8(const asset::AssetView& asset_container,
                 const ExportOptions& options) {
  const fs::path output_dir_path = options.output_path;

  nnl::BufferView pphd8 = asset_container.at(0);
  nnl::BufferView audio_data = asset_container.at(1);

  audio_data.Seek(0);
  pphd8.Seek(0x18);

  auto offset_sample = pphd8.ReadLE<u32>();

  offset_sample += 0x20;
  pphd8.Seek(offset_sample);

  auto buffer_infos = pphd8.ReadArrayLE<phd::raw::RVagParam>(
      (pphd8.Len() - pphd8.Tell()) / sizeof(phd::raw::RVagParam));

  std::vector<nnl::BufferView> adpcm_buffers;

  adpcm_buffers.reserve(buffer_infos.size());

  for (auto& buff_info : buffer_infos) {
    if (buff_info.offset == static_cast<u32>(-1) ||
        buff_info.size == static_cast<u32>(-1))
      adpcm_buffers.push_back({});
    else
      adpcm_buffers.push_back(
          audio_data.SubView(buff_info.offset, buff_info.size));
  }

  for (std::size_t k = 0; k < buffer_infos.size(); k++) {
    auto buff_info = buffer_infos.at(k);

    auto name = nnl::utl::string::PrependZero(k, 2);

    if (buff_info.offset != static_cast<u32>(-1) &&
        buff_info.size != static_cast<u32>(-1)) {
      auto& adpcm_buffer = adpcm_buffers.at(k);
      name += "_sr_" + std::to_string(buff_info.sample_rate);
      name +=
          "_" + nnl::utl::string::IntToHex(nnl::utl::data::CRC32(adpcm_buffer));

      SAudio wav;

      wav.sample_rate = buff_info.sample_rate;
      wav.pcm = adpcm::Decode(adpcm_buffer);

      wav.ExportWAV(output_dir_path / nnl::utl::filesys::ReplaceExtension(
                                       fs::u8path(name), fs::u8path(".wav")));
    } else {
      name += "_placeholder";

      std::ofstream placeholder(output_dir_path /
                                nnl::utl::filesys::ReplaceExtension(
                                    fs::u8path(name), fs::u8path(".wav")));
    }
  }

  return true;
}
}  // namespace unit
