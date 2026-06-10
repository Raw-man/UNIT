#pragma once

#include "app.hpp"

#define UNIT_LOG(t, v)                    \
  do {                                    \
    auto& app = unit::App::GetInstance(); \
    app.Log(t, v);                        \
  } while (false)

#define UNIT_LOG_DEBUG(msg)                    \
  do {                                         \
    UNIT_LOG(msg, unit::App::LogType::kDebug); \
  } while (false)

#define UNIT_LOG_INFO(msg)                    \
  do {                                        \
    UNIT_LOG(msg, unit::App::LogType::kInfo); \
  } while (false)

#define UNIT_LOG_WARN(msg)                    \
  do {                                        \
    UNIT_LOG(msg, unit::App::LogType::kWarn); \
  } while (false)

#define UNIT_LOG_ERROR(msg)                    \
  do {                                         \
    UNIT_LOG(msg, unit::App::LogType::kError); \
  } while (false)
