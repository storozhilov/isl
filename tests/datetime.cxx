#include <gtest/gtest.h>
#include <isl/Timestamp.hxx>
#include <isl/TimeSpec.hxx>

TEST(TimeSpecTest, MakeTimeout)
{
	struct timespec to = isl::TimeSpec::makeTimeout(1, -500000000L);
	EXPECT_EQ(to.tv_sec, 0);
	EXPECT_EQ(to.tv_nsec, 500000000L);
	to = isl::TimeSpec::makeTimeout(1, -1500000000L);
	EXPECT_EQ(to.tv_sec, 0);
	EXPECT_EQ(to.tv_nsec, 0);
	to = isl::TimeSpec::makeTimeout(-1, 1500000000L);
	EXPECT_EQ(to.tv_sec, 0);
	EXPECT_EQ(to.tv_nsec, 500000000L);
}


TEST(TimestampTest, Subtraction)
{
	isl::Timestamp now = isl::Timestamp::now();
	isl::Timestamp after(now.second() + 1, now.nanoSecond() + 500000000L);
	EXPECT_EQ(after - now, isl::Timeout(1, 500000000L));
	EXPECT_EQ(now - after, isl::Timeout());
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
