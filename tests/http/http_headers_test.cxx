#include <gtest/gtest.h>
#include <isl/Http.hxx>

class HttpHeadersTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		headers.insert(isl::Http::Headers::value_type("Host", "localhost"));
		headers.insert(isl::Http::Headers::value_type("HOST", "storozhilov.com"));
		headers.insert(isl::Http::Headers::value_type("Content-type", "text/html"));
		headers.insert(isl::Http::Headers::value_type("Content-length", "128"));
	}
	
	isl::Http::Headers headers;
};

TEST_F(HttpHeadersTest, HasHeader)
{
	EXPECT_TRUE(isl::Http::hasHeader(headers, "Host"));
	EXPECT_TRUE(isl::Http::hasHeader(headers, "HOST"));
	EXPECT_FALSE(isl::Http::hasHeader(headers, "HOSTt"));
	EXPECT_TRUE(isl::Http::hasHeader(headers, "Content-type"));
	EXPECT_TRUE(isl::Http::hasHeader(headers, "content-type"));
}

TEST_F(HttpHeadersTest, HeaderValue)
{
	EXPECT_EQ("localhost", isl::Http::headerValue(headers, "Host"));
	EXPECT_EQ("text/html", isl::Http::headerValue(headers, "cONTENT-TYPE"));
	EXPECT_EQ("128", isl::Http::headerValue(headers, "cOntent-lengtH"));
	EXPECT_EQ(std::string(), isl::Http::headerValue(headers, "cOntent-lengtg"));
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

