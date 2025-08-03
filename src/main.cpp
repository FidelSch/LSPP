#include "Server.hpp"

int main() {

	LSPServer server;

	server.init(ServerCapabilities::hoverProvider | ServerCapabilities::definitionProvider);
	// server.stop();

	return server.exit();
}
