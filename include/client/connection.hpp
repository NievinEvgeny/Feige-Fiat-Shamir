#pragma once
#include <string>

namespace libclient {

int connect_to_server(const std::string& hostname, const std::string& port);

}  // namespace libclient