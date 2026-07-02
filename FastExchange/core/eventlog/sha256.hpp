#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace fastexchange {

std::vector<uint8_t> sha256(const std::vector<uint8_t>& data);
std::string sha256_hex(const std::vector<uint8_t>& data);

}  // namespace fastexchange
