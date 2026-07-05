#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

namespace {

class SpdlogEnv : public ::testing::Environment {
  public:
    void SetUp() override
    {
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");
    }
};

const auto* const g_spdlog_env = ::testing::AddGlobalTestEnvironment(
    new SpdlogEnv);

}  // namespace
