#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include "Message.hpp"
#include "nlohmann/json.hpp"
#include "Server.hpp"
#include "ProtocolStructures.hpp"


int main() {

	LSPServer server;

	server.init(ServerCapabilities::hoverProvider | ServerCapabilities::definitionProvider);
	// server.stop();

	return server.exit();
}
