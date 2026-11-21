#include <unity.h>
#include <modules/light/schedule.h>

using modules::schedule::isActive;
using modules::schedule::parseTimeOfDay;
using modules::schedule::SECONDS_PER_DAY;

namespace {
    constexpr uint32_t HOUR = 3600;
    constexpr uint32_t DAY_OFFSET = 20000 * SECONDS_PER_DAY;
}

void setUp() {}
void tearDown() {}

void test_daytime_period() {
    TEST_ASSERT_FALSE(isActive(5 * HOUR - 1, 5 * HOUR, 12 * HOUR));
    TEST_ASSERT_TRUE(isActive(5 * HOUR, 5 * HOUR, 12 * HOUR));
    TEST_ASSERT_TRUE(isActive(17 * HOUR - 1, 5 * HOUR, 12 * HOUR));
    TEST_ASSERT_FALSE(isActive(17 * HOUR, 5 * HOUR, 12 * HOUR));
}

void test_period_wrapping_midnight() {
    TEST_ASSERT_TRUE(isActive(22 * HOUR, 20 * HOUR, 10 * HOUR));
    TEST_ASSERT_TRUE(isActive(0, 20 * HOUR, 10 * HOUR));
    TEST_ASSERT_TRUE(isActive(6 * HOUR - 1, 20 * HOUR, 10 * HOUR));
    TEST_ASSERT_FALSE(isActive(6 * HOUR, 20 * HOUR, 10 * HOUR));
    TEST_ASSERT_FALSE(isActive(20 * HOUR - 1, 20 * HOUR, 10 * HOUR));
}

void test_period_ending_at_midnight() {
    TEST_ASSERT_TRUE(isActive(23 * HOUR, 12 * HOUR, 12 * HOUR));
    TEST_ASSERT_FALSE(isActive(0, 12 * HOUR, 12 * HOUR));
}

void test_epoch_time_is_reduced_to_time_of_day() {
    TEST_ASSERT_TRUE(isActive(DAY_OFFSET + 6 * HOUR, 5 * HOUR, 2 * HOUR));
    TEST_ASSERT_FALSE(isActive(DAY_OFFSET + 8 * HOUR, 5 * HOUR, 2 * HOUR));
}

void test_zero_duration_is_always_off() {
    TEST_ASSERT_FALSE(isActive(0, 0, 0));
    TEST_ASSERT_FALSE(isActive(5 * HOUR, 5 * HOUR, 0));
}

void test_full_day_is_always_on() {
    TEST_ASSERT_TRUE(isActive(0, 5 * HOUR, SECONDS_PER_DAY));
    TEST_ASSERT_TRUE(isActive(4 * HOUR, 5 * HOUR, SECONDS_PER_DAY));
}

void test_parse_time_of_day() {
    uint32_t seconds = 0;

    TEST_ASSERT_TRUE(parseTimeOfDay("05:30", seconds));
    TEST_ASSERT_EQUAL_UINT32(5 * HOUR + 30 * 60, seconds);

    TEST_ASSERT_TRUE(parseTimeOfDay("00:00", seconds));
    TEST_ASSERT_EQUAL_UINT32(0, seconds);

    TEST_ASSERT_TRUE(parseTimeOfDay("23:59", seconds));
    TEST_ASSERT_EQUAL_UINT32(23 * HOUR + 59 * 60, seconds);
}

void test_parse_rejects_invalid_values() {
    uint32_t seconds = 42;

    TEST_ASSERT_FALSE(parseTimeOfDay(nullptr, seconds));
    TEST_ASSERT_FALSE(parseTimeOfDay("", seconds));
    TEST_ASSERT_FALSE(parseTimeOfDay("5:30", seconds));
    TEST_ASSERT_FALSE(parseTimeOfDay("24:00", seconds));
    TEST_ASSERT_FALSE(parseTimeOfDay("12:60", seconds));
    TEST_ASSERT_FALSE(parseTimeOfDay("12:00:00", seconds));
    TEST_ASSERT_FALSE(parseTimeOfDay("ab:cd", seconds));
    TEST_ASSERT_EQUAL_UINT32(42, seconds);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_daytime_period);
    RUN_TEST(test_period_wrapping_midnight);
    RUN_TEST(test_period_ending_at_midnight);
    RUN_TEST(test_epoch_time_is_reduced_to_time_of_day);
    RUN_TEST(test_zero_duration_is_always_off);
    RUN_TEST(test_full_day_is_always_on);
    RUN_TEST(test_parse_time_of_day);
    RUN_TEST(test_parse_rejects_invalid_values);
    return UNITY_END();
}
