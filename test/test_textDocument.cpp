
#include <gtest/gtest.h>
#include "ProtocolStructures.hpp"

TEST(textDocument, getLine)
{
      textDocument td("hola0\nhola1\nhola2\nhola3\nmulti word test\nmulti word test");
      textDocument td2;

      for (int i = 0; i < 4; i++) {
            ASSERT_STREQ(("hola" + std::to_string(i)).c_str(), td.getLine(i).c_str());
      }

      ASSERT_STREQ("", td2.getLine(2).c_str());
      ASSERT_STREQ("", td.getLine(-1).c_str());
      ASSERT_STREQ("", td.getLine(8).c_str());
      ASSERT_STREQ("multi word test", td.getLine(4).c_str());
}

TEST(textDocument, wordDelimiter)
{
      ASSERT_TRUE(textDocument::isWordDelimiter('`'));
      ASSERT_TRUE(textDocument::isWordDelimiter('\\'));
      ASSERT_TRUE(textDocument::isWordDelimiter('\"'));
      ASSERT_TRUE(textDocument::isWordDelimiter('?'));
      ASSERT_TRUE(textDocument::isWordDelimiter(' '));

      ASSERT_FALSE(textDocument::isWordDelimiter('a'));
      ASSERT_FALSE(textDocument::isWordDelimiter('z'));
      ASSERT_FALSE(textDocument::isWordDelimiter('A'));
      ASSERT_FALSE(textDocument::isWordDelimiter('Z'));
      ASSERT_FALSE(textDocument::isWordDelimiter('_'));

}

TEST(textDocument, wordUnderCursor) {
      textDocument td("Lorem ipsum dolor sit amet consectetur adipiscing elit\n Consectetur adipiscing elit quisque faucibus ex sapien vitae\n Ex sapien vitae pellentesque sem placerat in id\n Placerat in id{cursus}mi pretium tellus duis\n Pretium tellus duis convallis tempus leo eu aenean\n");

      ASSERT_STREQ("dolor", td.wordUnderCursor({0, 12}).c_str());
      ASSERT_STREQ("elit", td.wordUnderCursor({1, 25}).c_str());
      ASSERT_STREQ("", td.wordUnderCursor({2, 3}).c_str());
      ASSERT_STREQ("", td.wordUnderCursor({0, 200}).c_str());
      ASSERT_STREQ("cursus", td.wordUnderCursor({3, 18}).c_str());
      ASSERT_STREQ("", td.wordUnderCursor({3, 15}).c_str());
}

int main(){
      ::testing::InitGoogleTest();
      return RUN_ALL_TESTS();
}
