#pragma once

#include "Exceptions.h"
#include <sstream>
#include <string>
#include <iomanip>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>
#include <set>
// used for debug output only
#include <iostream>

namespace Paxos {

  namespace {

    std::ostream& operator<<(std::ostream& oss, std::nullptr_t) {
      oss << "nullptr";
      return oss;
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& oss, const std::vector<T>& v)
    {
      oss << "[";
      const char* sep = "";
      for (const auto& e : v) {
        oss << sep << e;
        sep = " ,";
      }
      oss << "]";
      return oss;
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& oss, const std::set<T>& v)
    {
      oss << "[";
      const char* sep = "";
      for (const auto& e : v) {
        oss << sep << e;
        sep = " ,";
      }
      oss << "]";
      return oss;
    }

    /*
    * Currently parses fmt at runtime.
    * To use '{' need to escape it.
    */

    // This function is itself used to generate messages in its own exceptions
    template<typename... Args>
    std::string format(const char* fmt, Args&&... args);

    // format.h uses the short-hand FormatThrow instead of Throw defined in
    // common.h to avoid dependency loop
    template<typename... Args>
    void FormatThrow(const char* fmt, Args&&... args)
    {
      throw PaxosException(format(fmt, std::forward<Args>(args)...));
    }

    // TODO: differentiate between enum and enum class
    template<typename T>
    typename std::enable_if<
      !std::is_integral<
      typename std::remove_reference<T>::type
      >::value &&
      !std::is_enum<
      typename std::remove_reference<T>::type
      >::value
    >::type
    setFormatHex(std::ostream& oss, T&& value)
    {
      FormatThrow("value cannot be formatted as hex: {}",
        std::forward<T>(value));
    }

    template<typename T>
    typename std::enable_if<
      std::is_integral<
      typename std::remove_reference<T>::type
      >::value ||
      std::is_enum<
      typename std::remove_reference<T>::type
      >::value
    >::type
    setFormatHex(std::ostream& oss, T&& value)
    {
      int32_t x = static_cast<int32_t>(value);
      oss << "0x" << std::hex << std::setfill('0');
      if (x <= std::numeric_limits<uint16_t>::max()) {
        oss << std::setw(4);
      }
      else {
        oss << std::setw(8);
      }
    }

    template<typename T>
    void formatOutput(std::ostream& oss, char fmt, T&& value)
    {
      auto f = oss.flags();
      switch (fmt) {
      case 'x':
        setFormatHex(oss, std::forward<T>(value));
        break;
      default:
        FormatThrow("Unrecognized fmt character: {}", fmt);
        break;
      }
      oss << value;
      oss.flags(f);
    }

    std::ostream& format(std::ostream& oss, const char* fmt)
    {
      const char* s = fmt;
      if (!s) {
        return oss;
      }

      while (*s) {
        if (*s == '\\' && *(s + 1) == '{') {
          // An escaped '{' character
          ++s;
        }
        else if (*s == '{') {
          FormatThrow("fmt should not contain \\{. "
            "Fewer arguments are provided. "
            "Near '{}' inside '{}'",
            s,
            fmt);
        }
        oss << *s++;
      }
      return oss;
    }

    template<typename T, typename... Args>
    std::ostream& format(std::ostream& oss,
      const char* fmt,
      T&& value,
      Args&&... args)
    {
      const char* s = fmt;
      if (!s) {
        FormatThrow("null fmt with positive number of arguments: {}...",
          std::forward<T>(value));
      }

      while (*s) {
        if (*s == '\\' && *(s + 1) == '{') {
          // An escaped '{' character
          oss << *(s + 1);
          s += 2;
        }
        else if (*s == '{') {
          // try to detect the enclosing '}'
          const char* p;
          for (p = s + 1; *p && *p != '}'; ++p) {}
          if (*p != '}') {
            FormatThrow("ill formed fmt: no enclosing }: '{}' inside '{}'",
              s,
              fmt);
          }
          if (p - s > 2) {
            FormatThrow("Too many characters inside \\{}: {}",
              std::string(s + 1, p - s - 1));
          }
          else if (p - s == 2) {
            formatOutput(oss, *(s + 1), std::forward<T>(value));
          }
          else {
            oss << value;
          }
          return format(oss, p + 1, std::forward<Args>(args)...);
        }
        else {
          oss << *s++;
        }
      }

      // if we are still here, this indicates we have not exhausted
      // all the arguments
      FormatThrow("More arguments provided than needed: {}...",
        std::forward<T>(value));

      return oss;
    }

    std::ostream& formatDumpInternal(std::ostream& oss, int pos)
    {
      // nothing to dump
      return oss;
    }

    template<typename T, typename... Args>
    std::ostream& formatDumpInternal(
      std::ostream& oss, int pos, T&& value, Args&&... args)
    {
      if (pos > 0) {
        oss << ",";
      }
      oss << "<" << std::forward<T>(value) << ">";
      return formatDumpInternal(oss, pos + 1, std::forward<Args>(args)...);
    }

    template<typename... Args>
    std::string formatDump(Args&&... args)
    {
      std::ostringstream oss;
      oss << std::showpoint;
      formatDumpInternal(oss, 0, std::forward<Args>(args)...);
      return oss.str();
    }

    template<typename... Args>
    std::string format(const char* fmt, Args&&... args)
    {
      std::ostringstream oss;
      oss << std::showpoint;
      try {
        format(oss, fmt, std::forward<Args>(args)...);
      }
      catch (const std::exception& e) {
        FormatThrow(R"(Exeption `{}` while formatting "{}" with \{{}})",
          e.what(),
          fmt,
          formatDump(std::forward<Args>(args)...));
      }
      return oss.str();
    }

  } // anonymous

} // Paxos
