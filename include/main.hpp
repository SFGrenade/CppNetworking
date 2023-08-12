#ifndef MAIN_HPP_
#define MAIN_HPP_

#include <cstdint>
#if __cplusplus >= 202002L
#include <span>
#else
#include <vector>
#endif
#include <string>
#include <string_view>

namespace SFG {

extern "C" void signalHandler( int sigNum );

#if __cplusplus >= 202002L
[[nodiscard]] int32_t better_main( [[maybe_unused]] std::span< std::string_view > args ) noexcept;
#else
[[nodiscard]] int32_t better_main( [[maybe_unused]] std::vector< std::string_view > args ) noexcept;
#endif

void InitializeLoggers( std::string filePostfix ) noexcept;

void InitializeSignalHandler() noexcept;

}  // namespace SFG

int32_t main( int32_t const argc, char const *const *argv );

#endif /* MAIN_HPP_ */
