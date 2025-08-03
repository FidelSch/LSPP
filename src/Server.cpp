#include "Server.hpp"
#include "Message.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include "ProtocolStructures.hpp"
#include <map>

void LSPServer::updateDocumentBuffer(const DidChangeTextDocumentParams& params)
{
      if (0 == m_openDocuments.count(params.textDocument.uri))
            return;

      textDocument& document = m_openDocuments.at(params.textDocument.uri);
      for (auto &j : params.contentChanges)
      {
            if (j.range.has_value())
            {
                  const Range& contentChanged = j.range.value();

                  int startIndex = document.findPos(contentChanged.start.line, contentChanged.start.character);
                  int endIndex = document.findPos(contentChanged.end.line, contentChanged.end.character);

                  document.m_content.replace(startIndex, endIndex - startIndex, j.text);
            }
            else document.m_content = j.text;
      }
}

LSPServer::LSPServer() : m_listener(), force_shutdown(false) {}

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
      } while (m.method() != Message::Method::EXIT);

      return force_shutdown? 1: 0;
}

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
                  switch (message.method())
                  {
                        case Message::Method::TEXT_DOCUMENT_DID_OPEN:
                        {
                              server->m_openDocuments.emplace(message.documentURI(), message.params()["textDocument"]["text"]);
                              break;
                        }
                        case Message::Method::TEXT_DOCUMENT_DID_CHANGE:
                        {
                              server->updateDocumentBuffer(message.params());
                              break;
                        }
                        case Message::Method::TEXT_DOCUMENT_DID_CLOSE:
                        {
                              server->m_openDocuments.erase(message.documentURI());
                              break;
                        }
                        default:
                              break;
                  }
            }
            else // Request
            {
                  nlohmann::json responseData{{"id", message.id()}};

                  switch (message.method())
                  {
                        case Message::Method::INITIALIZE:
                        {
                              InitializeResult initResult{{"utf-16", ServerCapabilities::TextDocumentSyncOptions::Incremental, ServerCapabilities::hoverProvider | ServerCapabilities::definitionProvider}, {"LSPP", "1.0"}};
                              responseData = {{"id", message.id()}, {"result", initResult}};
                              break;
                        }
                        case Message::Method::SHUTDOWN:
                              responseData["result"] = nullptr;
                              server->stop();
                              break;
                        case Message::Method::EXIT:
                              responseData["result"] = nullptr;
                              server->stop();
                              return; // Exit the server main loop
                        case Message::Method::HOVER:
                        {
                              if (server->m_capabilities.advertisedCapabilities & ServerCapabilities::hoverProvider)
                              {
                                    responseData["result"] = server->hoverCallback(message.params());
                              }
                              break;
                        }
                        case Message::Method::DEFINITION:
                        {
                              if (server->m_capabilities.advertisedCapabilities & ServerCapabilities::definitionProvider)
                              {
                                    responseData["result"] = server->definitionCallback(message.params());
                              }
                              break;
                        }
                        case Message::Method::DECLARATION:
                        {
                              if (server->m_capabilities.advertisedCapabilities & ServerCapabilities::declarationProvider)
                              {
                                    responseData["result"] = server->declarationCallback(message.params());
                              }
                              break;
                        }
                        case Message::Method::NONE:
                        default:
                              responseData["error"] = {{"code", -32601}, {"message", "Method not found"}};
                  }

                  Message::log(responseData.dump());
                  std::cout << "Content-Length: " + std::to_string(responseData.dump().length()) + "\r\n\r\n" << responseData.dump();
            }

      }
}

hoverResult LSPServer::hoverCallback(const hoverParams &params)
{
      return DEFAULT_HOVER_RESULT;
}

definitionResult LSPServer::definitionCallback(const definitionParams &params)
{
      return DEFAULT_DEFINITION_RESULT;
}

declarationResult LSPServer::declarationCallback(const declarationParams & params)
{
      return DEFAULT_DECLARATION_RESULT;
}
