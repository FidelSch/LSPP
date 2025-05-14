#include "Server.hpp"
#include "Message.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include "ProtocolStructures.hpp"
#include <map>

LSPServer::LSPServer(): m_listener(), is_running(false) {}

LSPServer::~LSPServer()
{
      m_listener.join();
}

int LSPServer::init(const uint64_t& capabilities)
{
      is_running = true;
      m_capabilities.advertisedCapabilities = capabilities;
      m_listener = std::thread(server_main, this);
      return 0;
}

void LSPServer::stop()
{
      is_running = false;
      m_listener.join();
      return;
}

std::map<std::string, textDocument> openDocuments;
void LSPServer::server_main(LSPServer* server)
{
      Message message;
      textDocument document;

      do
      {
            message.readMessage(std::cin);
            Message::log(message.get());

            if (message.id() == 0) // Notification
            {
                  if (message.method() == "textDocument/didOpen")
                  {
                        openDocuments.emplace(message.params()["textDocument"]["uri"],  message.params()["textDocument"]["text"] );
                  }
                  else if (message.method() == "textDocument/didClose")
                  {
                        openDocuments.erase(message.params()["textDocument"]["uri"]);
                  }
            }
            else // Request
            {
                  nlohmann::json responseData{{"id", message.id()}};

                  if (message.method() == "initialize")
                  {
                        InitializeResult initResult{{"utf-16", ServerCapabilities::TextDocumentSyncOptions::None, ServerCapabilities::hoverProvider | ServerCapabilities::definitionProvider}, {"LSPP", "1.0"}};
                        responseData = {{"id", message.id()}, {"result", initResult}};
                  }
                  else if (message.method() == "textDocument/hover")
                  {
                        responseData["result"] = hoverCallback(message.params());
                  }
                  else if (message.method() == "textDocument/definition")
                  {
                        // responseData["result"] = {{"uri", ""}, {"range", {Position{3, 2}}}};
                  }
                  else if (message.method() == "shutdown"){
                        responseData["result"] = NULL;
                        break;
                  }

                  Message::log(responseData.dump());
                  std::cout << "Content-Length: " + std::to_string(responseData.dump().length()) + "\r\n\r\n" << responseData.dump();
            }

      } while (server->is_running);

      server->is_running = false;
}

hoverResult hoverCallback(const hoverParams &params)
{
      return hoverResult{MarkupKind::PlainText, "some response for: " + openDocuments.at(params.textDocument.uri).wordUnderCursor(params.position)};
}
