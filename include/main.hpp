#pragma once

#include <cstdint>
#include <span>
#include <string_view>

namespace SFG {

[[nodiscard]] int32_t better_main( [[maybe_unused]] std::span< std::string_view const > args ) noexcept;

void InitializeLoggers( std::string filePostfix ) noexcept;

}  // namespace SFG

int32_t main( int32_t const argc, char const *const *argv );
