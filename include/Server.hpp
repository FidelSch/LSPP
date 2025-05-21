#include <thread>
#include "ProtocolStructures.hpp"


class LSPServer {
      std::thread m_listener;
      bool force_shutdown;
      ServerCapabilities m_capabilities;
public:
      LSPServer();
      ~LSPServer();
      int init(const uint64_t& capabilities);
      void stop();
      int exit();
      static void server_main(LSPServer* server);
};

hoverResult hoverCallback(const hoverParams &params);
definitionResult definitionCallback(const definitionParams &params);
declarationResult declarationCallback(const declarationParams &params);

#define DEFAULT_HOVER_RESULT { {MarkupKind::PlainText, "some response for: " + openDocuments.at(params.textDocument.uri).wordUnderCursor(params.position)}, std::nullopt }
#define DEFAULT_DEFINITION_RESULT { params.textDocument.uri, {{0, 0}, params.position} }
#define DEFAULT_DECLARATION_RESULT { params.textDocument.uri, {{0, 0}, params.position} }