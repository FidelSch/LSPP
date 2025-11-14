#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>

#include "Server.hpp"
#include "Message.hpp"
#include "TestClient.hpp"


int main(){
	::testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}

TEST(Server, RespondsToInitialize) {
	// A minimal valid initialize request
	const std::string payload = R"({
		"jsonrpc": "2.0",
		"id": 1,
		"method": "initialize",
		"params": {}
	})";

	auto resp = testutil::runSingle(ServerCapabilities::hoverProvider | ServerCapabilities::definitionProvider, payload, /*stopAfter*/ true);
	ASSERT_FALSE(resp.header.empty());
	ASSERT_FALSE(resp.body.empty());
	ASSERT_GE(resp.body.size(), resp.contentLength);
	auto& json = resp.json;
	ASSERT_TRUE(json.contains("id"));
	ASSERT_TRUE(json.contains("result"));
	ASSERT_TRUE(json["result"].is_object());

	// Check response id echoes request id
	ASSERT_EQ(1, json["id"]);

	// Validate InitializeResult contents
	const auto& result = json["result"];
	ASSERT_TRUE(result.contains("capabilities"));
	ASSERT_TRUE(result.contains("serverInfo"));

	const auto& caps = result["capabilities"];
	// positionEncoding must be utf-16 and textDocumentSync should be Incremental (2)
	ASSERT_EQ(std::string("utf-16"), caps["positionEncoding"].get<std::string>());
	ASSERT_EQ(ServerCapabilities::TextDocumentSyncOptions::Incremental, caps["textDocumentSync"].get<int>());

	// Server advertises hover and definition support
	ASSERT_TRUE(caps.contains("hoverProvider"));
	ASSERT_TRUE(caps.contains("definitionProvider"));
	ASSERT_TRUE(caps["hoverProvider"].get<bool>());
	ASSERT_TRUE(caps["definitionProvider"].get<bool>());

	const auto& serverInfo = result["serverInfo"];
	ASSERT_EQ(std::string("LSPP"), serverInfo["name"].get<std::string>());
	ASSERT_EQ(std::string("1.0"), serverInfo["version"].get<std::string>());

}

TEST(Server, AdvertisesOnlyInitCapabilities) {
	const std::string payload = R"({
		"jsonrpc": "2.0",
		"id": 2,
		"method": "initialize",
		"params": {}
	})";

	auto resp = testutil::runSingle(ServerCapabilities::hoverProvider, payload, /*stopAfter*/ true);
	ASSERT_TRUE(resp.json.contains("result"));
	const auto& result = resp.json["result"];
	ASSERT_TRUE(result.contains("capabilities"));
	const auto& caps = result["capabilities"];

	// Should advertise hoverProvider only, and not definitionProvider
	ASSERT_TRUE(caps.contains("hoverProvider"));
	ASSERT_TRUE(caps["hoverProvider"].get<bool>());
	ASSERT_FALSE(caps.contains("definitionProvider"));
}

TEST(Server, ProperLifecycle) {

      //Initialize the server
      const std::string initPayload = R"({
            "jsonrpc": "2.0",
            "id": 1,
            "method": "initialize",
            "params": {}
      })";

      // Shutdown 
      const std::string shutdownPayload = R"({
            "jsonrpc": "2.0",
            "id": 7,
            "method": "shutdown",
            "params": {}
	})";

	// Exit after shutdown
	const std::string exitPayload = R"({
		"jsonrpc": "2.0",
		"method": "exit",
		"params": {}
	})";

      size_t expected_responses = 2; // initialize + shutdown
      auto shutdownResp = testutil::runBatch(0, {initPayload, shutdownPayload, exitPayload}, expected_responses);

      ASSERT_EQ(expected_responses, shutdownResp.jsonResponses.size());

      // 1) initialize: result must be an object
      const auto& r0 = shutdownResp.jsonResponses[0];
      ASSERT_TRUE(r0.contains("id"));
      ASSERT_EQ(1, r0["id"]);
      ASSERT_TRUE(r0.contains("result"));
      ASSERT_TRUE(r0["result"].is_object());

      // 2) shutdown: result must be null
      const auto& r1 = shutdownResp.jsonResponses[1];
      ASSERT_TRUE(r1.contains("id"));
      ASSERT_EQ(7, r1["id"]);
      ASSERT_TRUE(r1.contains("result"));
      ASSERT_TRUE(r1["result"].is_null());

      // 3) exit after shutdown: no response and server exit code 0
      ASSERT_EQ(0, shutdownResp.serverExitCode);
}


TEST(Server, ExitWithoutInitialize) {
      const std::string exitPayload = R"({
            "jsonrpc": "2.0",
            "method": "exit",
            "params": {}
      })";

      size_t expected_responses = 0; // no responses expected
      auto exitResp = testutil::runBatch(0, {exitPayload}, expected_responses, std::chrono::milliseconds(50)); // give some time to process and exit

      ASSERT_EQ(expected_responses, exitResp.jsonResponses.size());
      ASSERT_EQ(exitResp.serverExitCode, 0);
}

TEST(Server, MessageBeforeInitialize) {
      const std::string messagePayload = R"({
            "jsonrpc": "2.0",
            "id": 1,
            "method": "textDocument/hover",
            "params": {}
      })";

      auto resp = testutil::runSingle(0, messagePayload, /*stopAfter*/ true);

      // Server must respond with error -32002
      ASSERT_TRUE(resp.json.contains("error"));
      ASSERT_EQ(resp.json["error"]["code"].get<int>(), -32002);
      ASSERT_TRUE(resp.json["error"]["message"].is_string());
}

TEST(Server, ExitBeforeShutdown) {
      const std::string initialize = R"({
            "jsonrpc": "2.0",
            "id": 1,
            "method": "initialize",
            "params": {}
      })";

      const std::string exitPayload = R"({
            "jsonrpc": "2.0",
            "method": "exit",
            "params": {}
      })";

      size_t expected_responses = 1; // initialize only
      auto resp = testutil::runBatch(0, {initialize, exitPayload}, expected_responses);

      // Server must exit with code 1
      ASSERT_EQ(resp.serverExitCode, 1);
}

TEST(Server, RequestAfterShutdownReturnsInvalidRequest) {
      // Initialize → shutdown → arbitrary request (hover) → exit
      const std::string initialize = R"({
            "jsonrpc": "2.0",
            "id": 1,
            "method": "initialize",
            "params": {}
      })";

      const std::string shutdown = R"({
            "jsonrpc": "2.0",
            "id": 2,
            "method": "shutdown",
            "params": {}
      })";

      const std::string hover = R"({
            "jsonrpc": "2.0",
            "id": 3,
            "method": "textDocument/hover",
            "params": {"textDocument": {"uri": "file:///tmp/x.cpp"}, "position": {"line": 0, "character": 0}}
      })";

      const std::string exitMsg = R"({
            "jsonrpc": "2.0",
            "method": "exit",
            "params": {}
      })";

      size_t expected_responses = 3; // initialize + shutdown + hover
      auto batch = testutil::runBatch(0, {initialize, shutdown, hover, exitMsg}, expected_responses);

      ASSERT_EQ(expected_responses, batch.jsonResponses.size());

      // 1) initialize ok
      const auto &r0 = batch.jsonResponses[0];
      ASSERT_TRUE(r0.contains("id"));
      ASSERT_EQ(1, r0["id"]);
      ASSERT_TRUE(r0.contains("result"));
      ASSERT_TRUE(r0["result"].is_object());

      // 2) shutdown ok (null result)
      const auto &r1 = batch.jsonResponses[1];
      ASSERT_TRUE(r1.contains("id"));
      ASSERT_EQ(2, r1["id"]);
      ASSERT_TRUE(r1.contains("result"));
      ASSERT_TRUE(r1["result"].is_null());

      // 3) any further request should be invalid (-32600)
      const auto &r2 = batch.jsonResponses[2];
      ASSERT_TRUE(r2.contains("id"));
      ASSERT_EQ(3, r2["id"]);
      ASSERT_TRUE(r2.contains("error"));
      ASSERT_TRUE(r2["error"].contains("code"));
      ASSERT_EQ(-32600, r2["error"]["code"].get<int>());

      // 4) exit after shutdown should be OK and server exit code 0
      ASSERT_EQ(0, batch.serverExitCode);
}