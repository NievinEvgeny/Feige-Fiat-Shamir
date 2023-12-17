#pragma once
#include <cxxopts.hpp>

namespace libclient {

bool authentication(const cxxopts::ParseResult& parse_cmd_line);

}  // namespace libclient