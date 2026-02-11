#include "Server.hpp"

class MyServer : public LSPServer
{
public:
	std::optional<hoverResult> hoverCallback(const hoverParams &params) override
	{
		auto docOpt = m_documentHandler.getOpenDocument(params.textDocument.uri);
		if (!docOpt)
		{
			return std::nullopt;
		}
		std::string word = docOpt->get().wordUnderCursor(params.position.line, params.position.character);

		return hoverResult{{MarkupKind::PlainText, "This is my custom response for: " + word}, std::nullopt};
	}
};

int main()
{

	MyServer server;

	server.init(ServerCapabilities::hoverProvider);

	return server.exit();
}
