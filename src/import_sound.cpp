
#include "import.hpp"
#include "logger.hpp"
#include "samplerate.h"
#include "sndfile.h"
#include "utils.hpp"
namespace unit {

SAudio ImportAud(const std::filesystem::path& path, std::size_t max_samplerate = 44100) {
  SAudio audio;
  audio.name = path.filename().u8string();
  SNDFILE* sndfile = NULL;
  SF_INFO sfinfo;

  utl::Finally f([&sndfile]() {
    if (sndfile) sf_close(sndfile);
  });

#ifdef _WIN32
  sndfile = sf_wchar_open(path.wstring().c_str(), SFM_READ, &sfinfo);
#else
  sndfile = sf_open(path.u8string().c_str(), SFM_READ, &sfinfo);
#endif

  if (!sndfile) {
    UNIT_LOG_ERROR(sf_strerror(sndfile));
    throw unit::RuntimeError("failed to open the audio: " + path.u8string());
  }

  sf_count_t num_frames = sfinfo.frames;
  int num_channels = sfinfo.channels;

  if (num_channels > 2) throw unit::RuntimeError("the audio has more than 2 channels: " + path.u8string());

  audio.num_channels = num_channels;
  audio.sample_rate = sfinfo.samplerate;

  if ((std::size_t)sfinfo.samplerate <= max_samplerate) {
    audio.pcm.resize(num_frames * num_channels);
    sf_count_t frames_read = sf_readf_short(sndfile, audio.pcm.data(), num_frames);
    if (frames_read < 0) {
      UNIT_LOG_ERROR(sf_strerror(sndfile));
      throw unit::RuntimeError("failed to read the audio: " + path.u8string());
    }
  } else {
    std::unique_ptr<float[]> fbuffer(new float[num_channels * num_frames]);
    std::unique_ptr<float[]> fbuffer_out(new float[num_channels * num_frames]);
    sf_count_t frames_read = sf_readf_float(sndfile, fbuffer.get(), num_frames);
    if (frames_read < 0) {
      UNIT_LOG_ERROR(sf_strerror(sndfile));
      throw unit::RuntimeError("failed to read the audio: " + path.u8string());
    }

    SRC_DATA data;
    std::memset(&data, 0, sizeof(data));
    data.data_in = fbuffer.get();
    data.data_out = fbuffer_out.get();
    data.input_frames = num_frames;
    data.output_frames = num_frames;
    data.src_ratio = (double)max_samplerate / (double)sfinfo.samplerate;
    int result = src_simple(&data, SRC_SINC_MEDIUM_QUALITY, num_channels);
    if (result != 0) {
      UNIT_LOG_ERROR(src_strerror(result));
      throw unit::RuntimeError("error resampling audio file: " + path.u8string());
    }
    audio.pcm.resize(data.output_frames_gen * num_channels);
    src_float_to_short_array(fbuffer_out.get(), audio.pcm.data(), data.output_frames_gen * num_channels);

    audio.sample_rate = max_samplerate;
  }

  return audio;
}

bool ImportPPHD8(const ImportSndOpt& opt) {
  auto bin_asset = utl::LoadFile(opt.base_path);

  if (!asset::IsOfType(bin_asset)) {
    throw unit::RuntimeError("the base asset is not a sound bank: " + opt.input_path.u8string());
  }

  auto asset = asset::Import(opt.base_path);

  if (!phd::IsOfType(asset[asset::SoundBank::kPHD]))
    throw unit::RuntimeError("the base asset is not a sound bank: " + opt.input_path.u8string());

  nnl::BufferSpan pphd8 = asset.at(asset::SoundBank::kPHD);

  nnl::BufferRW raw;

  raw.Seek(0);

  pphd8.Seek(0x18);  // vag attr offset

  auto offset_sample = pphd8.ReadLE<u32>();  // Read vag attr offset
  offset_sample += 0x20;                     // Go to array of vagParam
  pphd8.Seek(offset_sample);

  auto audio_files = utl::GetSortedDirEntries(opt.input_path, {".wav", ".mp3"});

  UNIT_LOG_INFO("num entries: " + std::to_string(audio_files.size()));

  if (audio_files.empty()) {
    throw unit::RuntimeError("no wav files were found in the directory " + opt.input_path.u8string());
  }

  for (const auto& input_wav_path : audio_files) {
    if (pphd8.Len() < pphd8.Tell() + sizeof(phd::raw::RVagParam)) {
      UNIT_LOG_WARN("too many audio files: the base asset has less entries");
      break;
    }

    constexpr std::size_t min_wav_size = 0x2C;

    phd::raw::RVagParam buff_info;

    if (fs::file_size(input_wav_path) < min_wav_size) {
      UNIT_LOG_DEBUG("a placeholder: " + input_wav_path.u8string());
      pphd8.WriteLE(buff_info);
      continue;
    }

    SAudio wav = ImportAud(input_wav_path, opt.max_sr);

    wav.ToMono();

    auto adpcm = adpcm::Encode(wav.pcm);

    buff_info.offset = raw.Tell();
    buff_info.sample_rate = wav.sample_rate;
    buff_info.reserved = -1;

    buff_info.size = adpcm.size();

    pphd8.WriteLE(buff_info);
    raw.WriteArrayLE(adpcm);
  }

  asset.at(asset::SoundBank::kPBD) = std::move(raw);

  auto bin = asset::Export(asset);

  if (bin.size() > 4_MiB)
    UNIT_LOG_WARN("the output file is too big (" + utl::BytesToMegabytes(bin.size()) + ") and might not be loaded");

  utl::SaveFile(opt.output_path, bin);

  return true;
}
}  // namespace unit
