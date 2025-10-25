#include <thread>
#include "ProtocolStructures.hpp"
#include "textDocument.hpp"
#include "Message.hpp"


class LSPServer {
      std::thread m_listener;
      bool force_shutdown;
      bool isOKtoExit;
      ServerCapabilities m_capabilities;
      std::map<std::string, textDocument> m_openDocuments;

      void updateDocumentBuffer(const DidChangeTextDocumentParams& params);
public:
      LSPServer();
      ~LSPServer();
      int init(const uint64_t& capabilities);
      void stop();
      int exit();
      static void server_main(LSPServer* server);
      bool hasCapability(uint64_t capability) const;
      Response processRequest(const Message& message);
      void processNotification(const Message& message);

      hoverResult hoverCallback(const hoverParams &params);
      definitionResult definitionCallback(const definitionParams &params);
      declarationResult declarationCallback(const declarationParams &params);
};


#define DEFAULT_HOVER_RESULT { {MarkupKind::PlainText, "some response for: " + m_openDocuments.at(params.textDocument.uri).wordUnderCursor(params.position.line, params.position.character)}, std::nullopt }
#define DEFAULT_DEFINITION_RESULT { params.textDocument.uri, {{0, 0}, params.position} }
#define DEFAULT_DECLARATION_RESULT { params.textDocument.uri, {{0, 0}, params.position} }