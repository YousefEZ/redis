#include <gtest/gtest.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

class SpdlogEnv : public ::testing::Environment {
    spdlog::level::level_enum m_level = spdlog::level::info;

  public:
    void set_level(spdlog::level::level_enum level) { m_level = level; }

    void SetUp() override
    {
        spdlog::set_level(m_level);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");

        SPDLOG_DEBUG("SpdlogEnv::SetUp() - spdlog initialized");
    }
};

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    SpdlogEnv* logger = new SpdlogEnv;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.starts_with("--log=")) {
            logger->set_level(spdlog::level::from_str(arg.substr(6)));
        }
    }

    ::testing::AddGlobalTestEnvironment(logger);
    return RUN_ALL_TESTS();
}
