#pragma once
#include <thread>
#include "ProtocolStructures.hpp"
#include "textDocument.hpp"
#include "Message.hpp"
#include "iostream"


class DocumentHandler {
      std::map<std::string, textDocument> m_openDocuments;
public:
      bool openDocument(const std::string& uri, const std::string& document);
      bool closeDocument(const std::string& uri);
      bool updateDocument(const std::string& uri, const DidChangeTextDocumentParams& params);
      bool documentIsOpen(const std::string& uri) const;

      // Returns a reference to the open document if it exists
      std::optional<std::reference_wrapper<textDocument>> getOpenDocument(const std::string& uri);
};

class LSPServer {
      std::thread m_listener;
      bool force_shutdown;
      bool isOKtoExit;
      bool m_shutdownRequested;
      bool m_initialized;
      ServerCapabilities m_capabilities;
      DocumentHandler m_documentHandler;

      std::istream* m_input_stream;
      std::ostream* m_output_stream;
public:
      LSPServer();
      ~LSPServer();
      int init(const uint64_t& capabilities, std::istream& in = std::cin, std::ostream& out = std::cout);
      void stop();
      int exit();
      static void server_main(LSPServer* server);
      bool hasCapability(uint64_t capability) const;
      Response processRequest(const Message& message);
      void processNotification(const Message& message);
      void send(const Response& response, bool flush = false);

      std::optional<hoverResult> hoverCallback(const hoverParams &params);
      definitionResult definitionCallback(const definitionParams &params);
      declarationResult declarationCallback(const declarationParams &params);
};


#define DEFAULT_HOVER_RESULT { {MarkupKind::PlainText, "some response for: " + m_documentHandler.getOpenDocument(params.textDocument.uri).value().get().wordUnderCursor(params.position.line, params.position.character)}, std::nullopt }
#define DEFAULT_DEFINITION_RESULT { params.textDocument.uri, {{0, 0}, params.position} }
#define DEFAULT_DECLARATION_RESULT { params.textDocument.uri, {{0, 0}, params.position} }