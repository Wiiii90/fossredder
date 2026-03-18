/**
 * @file core/tests/unit/TestErrorReporterRegistry.cpp
 * @brief Unit tests for the process-global error reporter registry.
 */

#include "gtest/gtest.h"

#include "core/errors/ErrorReporterRegistry.h"

#include <memory>
#include <utility>
#include <vector>

namespace {

class RecordingReporter : public core::errors::IErrorReporter {
public:
    void report(const core::errors::ErrorEvent& event) override
    {
        events.push_back(event);
    }

    std::vector<core::errors::ErrorEvent> events;
};

class ScopedReporterReset {
public:
    ~ScopedReporterReset()
    {
        core::errors::setGlobalErrorReporter(nullptr);
    }
};

} // namespace

TEST(ErrorReporterRegistryTests, SetAndGetGlobalReporterRoundTripsSharedPointer)
{
    ScopedReporterReset reset;
    auto reporter = std::make_shared<RecordingReporter>();

    core::errors::setGlobalErrorReporter(reporter);

    EXPECT_EQ(core::errors::globalErrorReporter(), reporter);
}

TEST(ErrorReporterRegistryTests, ReportOverloadForwardsStructuredEventToGlobalReporter)
{
    ScopedReporterReset reset;
    auto reporter = std::make_shared<RecordingReporter>();
    core::errors::setGlobalErrorReporter(reporter);

    core::errors::report(core::errors::ErrorSeverity::Warning,
                         "TEST_CODE",
                         "core::tests::report",
                         "registry forwarded",
                         {{"key", "value"}});

    ASSERT_EQ(reporter->events.size(), 1u);
    const auto& event = reporter->events.front();
    EXPECT_EQ(event.severity, core::errors::ErrorSeverity::Warning);
    EXPECT_EQ(event.code, "TEST_CODE");
    EXPECT_EQ(event.origin, "core::tests::report");
    EXPECT_EQ(event.message, "registry forwarded");
    ASSERT_EQ(event.context.size(), 1u);
    EXPECT_EQ(event.context.front().first, "key");
    EXPECT_EQ(event.context.front().second, "value");
}
