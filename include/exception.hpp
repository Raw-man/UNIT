#pragma once

#include <string>

namespace unit {

class Exception : public std::exception {
 public:
  Exception(std::string what_arg) : m(std::move(what_arg)) {}

  const char* what() const noexcept override { return m.c_str(); }

  virtual ~Exception() = default;

 protected:
  static std::string Name(const std::string& ename) {
    return "[unit." + ename + "] ";
  }
  const std::string m;
};

class RuntimeError : public Exception {
 public:
  RuntimeError(const std::string& what_arg)
      : Exception(Exception::Name("runtime_error") + what_arg) {}
};

}  // namespace unit
