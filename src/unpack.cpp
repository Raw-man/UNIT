#include "unpack.hpp"

#include "detect.hpp"
#include "logger.hpp"
#include "utils.hpp"
namespace unit {

std::string MD5ToStr(const std::array<u8, 16> md5sum) {
  std::string res;
  res.reserve(32);

  for (auto x : md5sum) {
    res += nnl::utl::string::IntToHex(x, false, true);
  }

  return res;
}

// Generate hashes used by the plugins
std::string HashAsset(BufferView buffer, format::FileFormat type, u32 id) {
  buffer.Seek(0);
  nnl::utl::data::MD5Context ctx;

  if (id == u32(-1)) return "";

  u16 dig_ent_ind = id >> 16;

  if (type == format::kFog) {
    ctx.Update(buffer);
  } else if (type == format::kLit) {
    ctx.Update(buffer.SubView(0, 0x35));
    u8 empty[11] = {0};
    ctx.Update({&empty, 11});
  } else if (type == format::kPositionData) {
    buffer.Seek(0x4);

    auto num_pos = buffer.ReadLE<u32>();

    ctx.Update(buffer.SubView(0, 0x8 + num_pos * 0x18));
  } else if (type == format::kRenderConfig) {
    ctx.Update(buffer);
  } else if (type == format::kATRAC3) {
    auto nsuni = nnl::utl::data::MD5(buffer.SubView(0, 0x70));
    auto nslar = nnl::utl::data::MD5(buffer.SubView(0, 0x100));
    return MD5ToStr(nsuni) + "_" + MD5ToStr(nslar);
  } else if (type == format::kTextureContainer) {
    buffer.Seek(0);
    auto header = buffer.ReadLE<texture::raw::RHeader>();

    if (header.num_clut_offsets == 0) return "";

    u32* ptr_clut = (u32*)(buffer.data() + header.offset_clut_offsets);

    ptr_clut += (header.num_clut_offsets - 1);

    std::size_t size = std::min<std::size_t>(0x4000u, *ptr_clut + (256 * 4));

    if (buffer.size() < size) return "";

    ctx.Update(buffer.SubView(0, size));
    return MD5ToStr(ctx.Final());
  } else {
    return "";
  }

  if (dig_ent_ind != u16(-1)) ctx.Update({&id, 4});

  return MD5ToStr(ctx.Final());
}

// Generate hashes used by the plugins
std::string HashContainer(BufferView buffer, asset::Category type, u32 id) {
  if (id == u32(-1)) return "";

  buffer.Seek(0);

  u16 dig_ent_ind = id >> 16;

  nnl::utl::data::MD5Context ctx;

  auto num_rec = buffer.ReadLE<u32>();
  std::size_t size = nnl::utl::math::RoundNum(0x4 + num_rec * 0x8, 0x10);

  if (type == asset::Category::kAsset3DModel) {
    ctx.Update(buffer.SubView(0, size));
  } else if (type == asset::Category::kBitmapTextFull) {
    ctx.Update(buffer.SubView(0, size));
  } else if (type == asset::Category::kSoundBank) {
    ctx.Update(buffer.SubView(0, size));
    return MD5ToStr(ctx.Final());
  } else {
    return "";
  }

  if (dig_ent_ind != u16(-1)) ctx.Update({&id, 4});

  return MD5ToStr(ctx.Final());
}

void AppendHash(std::string& output_path, BufferView buffer, format::FileFormat type, u32 id) {
  std::string hash = HashAsset(buffer, type, id);

  if (!hash.empty()) output_path = "_" + hash + output_path;
}

void AppendHash(std::string& output_path, BufferView buffer, asset::Category type, u32 id) {
  std::string hash = HashContainer(buffer, type, id);
  if (!hash.empty()) output_path = "_" + hash + output_path;
}

void AppendMainExtension(std::string& file_name, format::FileFormat type) { file_name += GetExtension(type); }

void AppendCategoryExtension(std::string& file_name, asset::Category type) { file_name += GetCatExtension(type); }

void GenerateConfig(std::string cmd, const fs::path& output_path) {
  std::string toml = "[pac." + cmd + "]\n" +
                     "input = \"."
                     "\"\n"
                     "recursive = true";
  auto new_path = output_path / fs::u8path(".toml");

  utl::SaveFile(new_path, toml);
}

struct FormatInfo {
  format::FileFormat type{};
  asset::Category category{};
};

bool RecursiveUnpack(FormatInfo fmt_inf, BufferView data, const fs::path& output_path, int recursive, u32 id);

void ProcessEntry(std::string name, BufferView view, const std::filesystem::path& output_path, int recursive, u32 id) {
  std::string output_name;

  FormatInfo fmt_inf;

  fmt_inf.type = format::Detect(view);

#ifndef NDEBUG
  {
    auto types = format::DetectAll(view);
    assert(types.empty() || types.size() == 1);  // ideally, only 1 type should be detected
  }
#endif

  AppendMainExtension(output_name, fmt_inf.type);

  AppendHash(output_name, view, fmt_inf.type, id);

  if (fmt_inf.type == format::kAssetContainer) {
    auto asset_view = asset::ImportView(view);
    fmt_inf.category = asset::Categorize(asset_view);
    AppendCategoryExtension(output_name, fmt_inf.category);
    AppendHash(output_name, view, fmt_inf.category, id);
  }

  output_name = name + output_name;

  auto output_path_entry = output_path / fs::u8path(output_name);

  if (recursive) {
    RecursiveUnpack(fmt_inf, view, output_path_entry, recursive, id);
  } else {
    utl::SaveFile(output_path_entry, view);
  }
}

bool UnpackDig(BufferView data, const fs::path& output_path, int recursive, UnpackOpt::Naming naming) {
  auto dig_view = dig::ImportView(data);
  utl::CreateDir(output_path);

  if (recursive) {
    GenerateConfig("dig", output_path);
  }

  for (std::size_t i = 0; i < dig_view.size(); i++) {
    const auto& rec = dig_view[i];
    nnl::Buffer decomp_buf;
    nnl::BufferView view = rec.buffer;

    if (rec.is_compressed) {
      decomp_buf = dig::Decompress(view, rec.decompressed_size);
      view = decomp_buf;
    }

    std::string name = std::to_string(i) + (rec.is_compressed ? "_comp" : "_nocomp");

    u32 id = -1;  // an asset id used by the games: dig entry index + nested entry index

    switch (naming) {
      case UnpackOpt::kPlugin:
        id = (i & 0xFFFF) << 16;  // new plugin hash 0000'0000
        break;
      case UnpackOpt::kOldPlugin:
        id = 0xFFFF'0000;  // old plugin hash 0xFFFF'0000
        break;
      default:
        break;
    }

    ProcessEntry(name, view, output_path, recursive, id);
  }

  return true;
}

bool UnpackDigEntry(BufferView data, const fs::path& output_path, int recursive, u32 id) {
  auto entry_view = dig_entry::ImportView(data);
  utl::CreateDir(output_path);

  if (recursive) GenerateConfig("ent", output_path);

  for (std::size_t i = 0; i < entry_view.size(); i++) {
    const auto& view = entry_view[i];

    std::string name = std::to_string(i);

    u32 full_id = -1;

    u16 dig_ent_ind = id >> 16;
    u16 cur_ent_ind = id & 0xFFFF;

    if (dig_ent_ind == u16(-1) && cur_ent_ind != u16(-1)) full_id = id;                 // old hash
    if (dig_ent_ind != u16(-1) && cur_ent_ind != u16(-1)) full_id = id | (0xFFFF & i);  // new hash

    ProcessEntry(name, view, output_path, recursive, full_id);
  }

  return true;
}

bool UnpackCollection(BufferView data, const fs::path& output_path, int recursive, u32 id) {
  auto asset_col = collection::ImportView(data);

  utl::CreateDir(output_path);

  if (recursive) GenerateConfig("col", output_path);

  for (std::size_t i = 0; i < asset_col.size(); i++) {
    const auto& view = asset_col[i];

    std::string name = std::to_string(i);

    ProcessEntry(name, view, output_path, recursive, id);
  }

  return true;
}

bool UnpackAsset(BufferView data, const fs::path& output_path, int recursive, u32 id) {
  auto asset_view = asset::ImportView(data);

  utl::CreateDir(output_path);

  if (recursive) GenerateConfig("ast", output_path);

  for (auto& [i, view] : asset_view) {
    std::string name = std::to_string(i);

    ProcessEntry(name, view, output_path, recursive, id);
  }
  return true;
}

bool RecursiveUnpack(FormatInfo fmt_inf, BufferView data, const fs::path& output_path, int recursive, u32 id) {
  if (fmt_inf.type == format::FileFormat::kDigEntry) {
    UnpackDigEntry(data, output_path, recursive, id);
  } else if (fmt_inf.type == format::kCollection) {
    UnpackCollection(data, output_path, recursive, id);
  } else if (fmt_inf.type == format::kAssetContainer && fmt_inf.category != asset::Category::kPlaceholder &&
             (recursive > 1 || fmt_inf.category == asset::Category::kUIConfigTextureContainer)) {
    UnpackAsset(data, output_path, recursive, id);
  } else {
    utl::SaveFile(output_path, data);
  }
  return true;
}

bool Unpack(const UnpackOpt& opt) {
  nnl::Buffer bin_asset = utl::LoadFile(opt.input_path);

  auto type = format::Detect(bin_asset);

  if (type == format::kUnknown) {
    UNIT_LOG_WARN("unknown asset type: " + opt.input_path.u8string());
    return false;
  }

  if (type == format::kDig) return UnpackDig(bin_asset, opt.output_path, opt.recursive, opt.naming);

  if (type == format::kDigEntry) return UnpackDigEntry(bin_asset, opt.output_path, opt.recursive, -1);

  if (type == format::kCollection) return UnpackCollection(bin_asset, opt.output_path, opt.recursive, -1);

  if (type == format::kAssetContainer) return UnpackAsset(bin_asset, opt.output_path, opt.recursive, -1);

  UNIT_LOG_WARN(
      "the file cannot be unpacked but it might be exported with the exp "
      "command. The type is \'" +
      GetExtension(type).substr(1) + "\'; " + opt.input_path.u8string());

  return false;
}

}  // namespace unit
