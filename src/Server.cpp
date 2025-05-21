#include "Server.hpp"
#include "Message.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include "ProtocolStructures.hpp"
#include <map>

LSPServer::LSPServer(): m_listener(), force_shutdown(false) {}

LSPServer::~LSPServer()
{
      m_listener.join();
}

int LSPServer::init(const uint64_t& capabilities)
{
      force_shutdown = false;
      m_capabilities.advertisedCapabilities = capabilities;
      m_listener = std::thread(server_main, this);
      return 0;
}

void LSPServer::stop()
{
      force_shutdown = true;
      return;
}

int LSPServer::exit()
{
      m_listener.join();

      Message m;
      do {
            m.readMessage(std::cin);
      } while (m.method() != "exit");

      return force_shutdown? 1: 0;
}

std::map<std::string, textDocument> openDocuments;
void LSPServer::server_main(LSPServer* server)
{
      Message message;
      textDocument document;

      while (!server->force_shutdown)
      {
            message.readMessage(std::cin);
            Message::log(message.get());

            if (message.id() == 0) // Notification
            {
                  if (message.method() == "textDocument/didOpen")
                  {
                        openDocuments.emplace(message.documentURI(),  message.params()["textDocument"]["text"] );
                  }
                  else if (message.method() == "textDocument/didChange")
                  {
                        static std::vector<nlohmann::json> contentChanges;
                        if (0 == openDocuments.count(message.documentURI())) {continue;} // Some error, TODO: handle this

                        assert(message.params()["contentChanges"].is_array());
                        contentChanges = message.params()["contentChanges"];
                        for (auto& j: contentChanges) { 
                              Range contentChanged = j["range"];
                              int startIndex = openDocuments.at(message.documentURI()).findPos(contentChanged.start);
                              int endIndex = openDocuments.at(message.documentURI()).findPos(contentChanged.end);
                              openDocuments.at(message.documentURI()).m_content.replace(startIndex, endIndex-startIndex, j["text"]);
                        }
                  }
                  else if (message.method() == "textDocument/didClose")
                  {
                        openDocuments.erase(message.documentURI());
                  }
            }
            else // Request
            {
                  nlohmann::json responseData{{"id", message.id()}};

                  if (message.method() == "initialize")
                  {
                        InitializeResult initResult{{"utf-16", ServerCapabilities::TextDocumentSyncOptions::Incremental, ServerCapabilities::hoverProvider | ServerCapabilities::definitionProvider}, {"LSPP", "1.0"}};
                        responseData = {{"id", message.id()}, {"result", initResult}};
                  }
                  else if (message.method() == "textDocument/hover")
                  {
                        responseData["result"] = hoverCallback(message.params());
                  }
                  else if (message.method() == "textDocument/definition")
                  {
                        responseData["result"] = definitionCallback(message.params());
                  }
                  else if (message.method() == "textDocument/declaration")
                  {
                        responseData["result"] = declarationCallback(message.params());
                  }
                  else if (message.method() == "shutdown"){ // Expected server shutdown
                        responseData["result"] = NULL;
                        return;
                  }
                  else if (message.method() == "exit"){ // Unexpected server shutdown
                        responseData["result"] = NULL;
                        server->stop();
                  }

                  Message::log(responseData.dump());
                  std::cout << "Content-Length: " + std::to_string(responseData.dump().length()) + "\r\n\r\n" << responseData.dump();
            }

      }
}

hoverResult hoverCallback(const hoverParams &params)
{
      return DEFAULT_HOVER_RESULT;
}

definitionResult definitionCallback(const definitionParams &params)
{
      return DEFAULT_DEFINITION_RESULT;
}

declarationResult declarationCallback(const declarationParams & params)
{
      return DEFAULT_DECLARATION_RESULT;
}
