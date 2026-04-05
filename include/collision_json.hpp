#pragma once

#include "NNL/game_asset/interaction/collision.hpp"
#include "NNL/simple_asset/svalue.hpp"

namespace unit {

inline void ToSValue(nnl::SValue& j, const nnl::collision::SurfaceFeatures& p) {
  using namespace nnl;
  using namespace nnl::collision;

  switch (p & SurfaceFeatures{0xF}) {
    case SurfaceFeatures::kNone:
      j["surface"] = "default";
      break;

    case SurfaceFeatures::kDirt:
      j["surface"] = "dirt";
      break;

    case SurfaceFeatures::kDirt2:
      j["surface"] = "dirt2";
      break;

    case SurfaceFeatures::kSand:
      j["surface"] = "sand";
      break;

    case SurfaceFeatures::kStone:
      j["surface"] = "stone";
      break;

    case SurfaceFeatures::kStone2:
      j["surface"] = "stone2";
      break;

    case SurfaceFeatures::kGrass:
      j["surface"] = "grass";
      break;

    case SurfaceFeatures::kWater:
      j["surface"] = "water";
      break;

    case SurfaceFeatures::kWood:
      j["surface"] = "wood";
      break;

    case SurfaceFeatures::kMetal:
      j["surface"] = "metal";
      break;

    default:
      j["surface"] = "REPORT_ERROR";
      break;
  }

  j["surface_values"] = "default, dirt, dirt2, sand, stone, stone2, grass, water, wood, metal";

  j["shadow"] = (bool)(p & SurfaceFeatures::kShadow);
  j["pass_throw"] = (bool)(p & SurfaceFeatures::kPassThrowables);
  j["pass_cam"] = (bool)(p & SurfaceFeatures::kPassCamera);
}

inline void ToSValue(nnl::SValue& j, const nnl::collision::PushFeatures& p) {
  using namespace nnl;
  using namespace nnl::collision;

  j["push_back"] = (bool)(p & PushFeatures::kPushBack);
  j["push_stuck"] = (bool)(p & PushFeatures::kPushIfStuck);
}

inline void ToSValue(nnl::SValue& j, const nnl::collision::ConvertParam& p) {
  ToSValue(j, p.surface_features);
  ToSValue(j, p.push_features);
}

inline void FromSValue(const nnl::SValue& j, nnl::collision::ConvertParam& p) {
  using namespace nnl;
  using namespace nnl::collision;

  if (j.Has("surface")) {
    auto s = j.At("surface").Get<std::string>();

    if (s == "dirt")
      p.surface_features = SurfaceFeatures::kDirt;
    else if (s == "dirt2")
      p.surface_features = SurfaceFeatures::kDirt2;
    else if (s == "sand")
      p.surface_features = SurfaceFeatures::kSand;
    else if (s == "stone")
      p.surface_features = SurfaceFeatures::kStone;
    else if (s == "stone2")
      p.surface_features = SurfaceFeatures::kStone2;
    else if (s == "grass")
      p.surface_features = SurfaceFeatures::kGrass;
    else if (s == "water")
      p.surface_features = SurfaceFeatures::kWater;
    else if (s == "wood")
      p.surface_features = SurfaceFeatures::kWood;
    else if (s == "metal")
      p.surface_features = SurfaceFeatures::kMetal;
  }

  if (j.Has("shadow") && j.At("shadow")) {
    p.surface_features |= SurfaceFeatures::kShadow;
  }

  if (j.Has("pass") && j.At("pass")) {
    p.surface_features |= SurfaceFeatures::kPassThrowables;
  }

  if (j.Has("pass_throw") && j.At("pass_throw")) {
    p.surface_features |= SurfaceFeatures::kPassThrowables;
  }

  if (j.Has("pass_cam") && j.At("pass_cam")) {
    p.surface_features |= SurfaceFeatures::kPassCamera;
  }

  if (j.Has("push_back") && j.At("push_back")) {
    p.push_features |= PushFeatures::kPushBack;
  }

  if (j.Has("push_stuck") && j.At("push_stuck")) {
    p.push_features |= PushFeatures::kPushIfStuck;
  }
}

}  // namespace unit
