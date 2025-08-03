#include <gtest/gtest.h>
#include "Message.hpp"
#include <fstream>
#include <istream>

TEST(Message, constructors)
{
      std::istringstream s1("");
      Message m1(s1);

      Message m2;

      std::istringstream s3("Content-Length: 7\r\n\r\n{\"a\":1}");
      Message m3(s3); // Normal data payload

      std::istringstream s4("Content-Length: 7\r\n\r\n{\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\":1}");
      Message m4(s4); // Actual data longer than expected

      std::istringstream s5("Content-Length: 0\r\n\r\n");
      Message m5(s5); // Declared 0 length

      std::istringstream s6("Content-Length: 5\r\n\r\n");
      Message m6(s6); // No data, wrong declared length

      ASSERT_STREQ("", m1.get().c_str());
      ASSERT_STREQ("", m2.get().c_str());
      ASSERT_STREQ("{\"a\":1}", m3.get().c_str());
      ASSERT_STREQ("{\"aaaaa", m4.get().c_str());
      ASSERT_STREQ("", m5.get().c_str());
      ASSERT_STREQ("", m6.get().c_str());
}

TEST(Message, parsing) {
      std::ifstream example("../test/initExample.txt");

      ASSERT_TRUE(example.is_open());

      Message m;
      m.readMessage(example);

      ASSERT_STREQ("initialize", m.method_description().c_str());
      ASSERT_EQ(Message::Method::INITIALIZE, m.method());
      ASSERT_EQ(1, m.id());
}

int main(){
      ::testing::InitGoogleTest();
      return RUN_ALL_TESTS();
}
