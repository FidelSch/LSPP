#include <gtest/gtest.h>
#include "ProtocolStructures.hpp"
#include <fstream>
#include <istream>

using json = nlohmann::json;
TEST(JSON, serialize_textDocumentSyncOptions) {
      ServerCapabilities::TextDocumentSyncOptions t{false, ServerCapabilities::TextDocumentSyncOptions::Full};
      json expected{{"change", ServerCapabilities::TextDocumentSyncOptions::Full}, {"openClose", false}};

      json j = t;


      ASSERT_TRUE(j == expected);
}
TEST(JSON, serialize_ServerCapabilities) {
      ServerCapabilities t{"utf-16", ServerCapabilities::TextDocumentSyncOptions::None, ServerCapabilities::definitionProvider | ServerCapabilities::hoverProvider};
      json expected{{"positionEncoding", "utf-16"}, {"textDocumentSync", ServerCapabilities::TextDocumentSyncOptions::None}, {"definitionProvider", true}, {"hoverProvider", true}};

      json j = t;
      std::cout << j<< std::endl;
      std::cout << expected<< std::endl;

      ASSERT_TRUE(j == expected);
}

TEST(JSON, serialize_ServerInfo) {
      ServerInfo t{"generic_name", "some_version"};
      json expected{{"name", "generic_name"}, {"version", "some_version"}};

      json j = t;

      ASSERT_TRUE(j == expected);
}

TEST(JSON, serialize_InitializeResult) {
      InitializeResult t{{"utf-16", ServerCapabilities::TextDocumentSyncOptions::None, ServerCapabilities::definitionProvider}, {"generic_name", "some_version"}};
      json expected{
            {"capabilities", {
                        {"positionEncoding", "utf-16"}, 
                        {"textDocumentSync", ServerCapabilities::TextDocumentSyncOptions::None }, 
                        {"definitionProvider", true}
                  }
            }, 
            {"serverInfo", {
                        {"name", "generic_name"}, 
                        {"version", "some_version"}
                  }
            }
      };

      json j = t;

      ASSERT_TRUE(j == expected);
}
/*
TEST(JSON, serialize_TYPE) {
      TYPE t{PARAMS};
      json expected{PARAMS};

      json j = t;

      ASSERT_TRUE(j == expected);
}
*/


int main(){
      ::testing::InitGoogleTest();
      return RUN_ALL_TESTS();
}