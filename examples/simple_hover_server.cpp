#include "Server.hpp"

class MyServer : public LSPServer
{
public:
	MyServer()
	{
		// Register hover callback
		registerCallback<hoverParams, std::optional<hoverResult>>(
		    Message::Method::HOVER,
		    [this](const hoverParams &params) -> std::optional<hoverResult>
		    {
			    auto docOpt = m_documentHandler.getOpenDocument(params.textDocument.uri);
			    if (!docOpt)
			    {
				    return std::nullopt;
			    }
			    std::string word = docOpt->get().wordUnderCursor(params.position.line, params.position.character);

			    return hoverResult{{MarkupKind::PlainText, "This is my custom response for: " + word}, std::nullopt};
		    });
	}
};

int main()
{

	MyServer server;

	server.init(ServerCapabilities::hoverProvider);

	return server.exit();
}
