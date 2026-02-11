#include "Server.hpp"
#include "Message.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include "ProtocolStructures.hpp"
#include <map>

LSPServer::LSPServer() : m_listener(), force_shutdown(false), isOKtoExit(false), m_shutdownRequested(false), m_initialized(false), m_input_stream(&std::cin), m_output_stream(&std::cout) {}

LSPServer::~LSPServer()
{
      stop();
      exit();
}

int LSPServer::init(const uint64_t &capabilities, std::istream &in, std::ostream &out)
{
      force_shutdown.store(false);
      isOKtoExit = false;
      m_shutdownRequested = false;
      m_initialized = false;
      m_capabilities.advertisedCapabilities = capabilities;

      m_input_stream = &in;
      m_output_stream = &out;

      m_listener = std::thread(server_main, this);
      return 0;
}

void LSPServer::stop()
{
      // TODO: Graceful shutdown actions, interrupt ongoing tasks, etc.
      force_shutdown.store(true);
      return;
}

int LSPServer::exit()
{
      if (m_listener.joinable())
      {
            // Wait for listener thread to finish
            m_listener.join();
      }
      return isOKtoExit ? 0 : 1;
}

void LSPServer::send(const Response &response, bool flush)
{
      try
      {
            std::string output = response.toString();
            // Extract JSON body from wire format for logging
            auto bodyStart = output.find("\r\n\r\n");
            if (bodyStart != std::string::npos)
            {
                  Message::log("OUTBOUND: " + output.substr(bodyStart + 4));
            }
            (*m_output_stream) << output;
            if (flush)
                  m_output_stream->flush();
      }
      catch (const std::bad_alloc &)
      {
            // Output minimal response on allocation failure
            (*m_output_stream) << "Content-Length: 2\r\n\r\n{}";
            if (flush)
                  m_output_stream->flush();
      }
      catch (...)
      {
            // Other exceptions - try minimal response
            try
            {
                  (*m_output_stream) << "Content-Length: 2\r\n\r\n{}";
                  if (flush)
                        m_output_stream->flush();
            }
            catch (...)
            {
                  // Give up
            }
      }
}

void LSPServer::server_main(LSPServer *server)
{
      Message message;
      textDocument document;

      while (!server->force_shutdown.load())
      {
            int readBytes = message.readMessage(*server->m_input_stream);
            if (readBytes < 0)
            {
                  // EOF or stream error - exit gracefully
                  break;
            }
            if (readBytes == 0)
            {
                  // Invalid message, but stream is still OK - continue
                  continue;
            }
            try
            {
                  Message::log("INBOUND: " + message.get());
            }
            catch (...)
            {
                  // Log failure is non-critical
            }

            auto now = std::chrono::steady_clock::now();

            if (!message.id().has_value()) // Notification
            {
                  server->processNotification(message);
            }
            else // Request
            {
                  Response response = server->processRequest(message);
                  server->send(response);
            }

            try
            {
                  Message::log("Processed in " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - now).count()) + " ms");
            }
            catch (...)
            {
                  // Log failure is non-critical
            }
      }
}

bool LSPServer::hasCapability(uint64_t capability) const
{
      return (m_capabilities.advertisedCapabilities & capability);
}

Response LSPServer::processRequest(const Message &message)
{
      Response response(message);

      // If not initialized yet, only allow 'initialize' and 'exit'
      if (!m_initialized)
      {
            if (message.method() != Message::Method::INITIALIZE && message.method() != Message::Method::EXIT)
            {
                  response.setError({{"code", -32002}, {"message", "Server not initialized"}});
                  return response;
            }
      }

      // If shutdown was requested, only allow 'exit'. All other requests must error.
      if (m_shutdownRequested)
      {
            if (message.method() != Message::Method::EXIT)
            {
                  response.setError({{"code", -32600}, {"message", "Server is shutting down"}});
                  return response;
            }
      }

      switch (message.method())
      {
      case Message::Method::INITIALIZE:
      {
            InitializeResult initResult{{"utf-16", ServerCapabilities::TextDocumentSyncOptions::Incremental, m_capabilities.advertisedCapabilities}, {"LSPP", "1.0"}};
            response.setResult(initResult);
            m_initialized = true;
            break;
      }
      case Message::Method::SHUTDOWN:
            response.setResult(nullptr);
            // Mark shutdown but keep serving to allow 'exit' and to error any other requests
            m_shutdownRequested = true;
            break;
      case Message::Method::NONE:
            response.setError({{"code", -32601}, {"message", "Method not found"}});
      default:
      {
            // Generic handler for all LSP methods with callbacks
            uint64_t requiredCapability = capabilityFlagForMethod(message.method());

            // If no capability required (0) or capability is advertised, try to invoke callback
            if (requiredCapability == 0 || hasCapability(requiredCapability))
            {
                  auto result = invokeCallback(message);
                  if (result)
                  {
                        response.setResult(*result);
                  }
                  else if (requiredCapability != 0)
                  {
                        // Capability advertised but no callback registered
                        response.setError({{"code", -32601}, {"message", "Method not implemented"}});
                  }
                  else
                  {
                        // Unknown method
                        response.setError({{"code", -32601}, {"message", "Method not found"}});
                  }
            }
            else
            {
                  // Capability not advertised
                  response.setError({{"code", -32601}, {"message", "Method not supported"}});
            }
            break;
      }
      }

      return response;
}

void LSPServer::processNotification(const Message &message)
{
      switch (message.method())
      {
      case Message::Method::EXIT:
            // Exit is OK if shutdown was requested, or if server was never initialized
            isOKtoExit = (m_shutdownRequested || !m_initialized);
            // EXIT received: stop the server loop now
            stop();
            break;
      case Message::Method::TEXT_DOCUMENT_DID_OPEN:
      {
            m_documentHandler.openDocument(message.documentURI(), message.params()["textDocument"]["text"]);
            break;
      }
      case Message::Method::TEXT_DOCUMENT_DID_CHANGE:
      {
            m_documentHandler.updateDocument(message.documentURI(), message.params());
            break;
      }
      case Message::Method::TEXT_DOCUMENT_DID_CLOSE:
      {
            m_documentHandler.closeDocument(message.documentURI());
            break;
      }
      default:
            break;
      }
}

bool DocumentHandler::updateDocument(const std::string &uri, const DidChangeTextDocumentParams &params)
{

      auto optionalDocument = getOpenDocument(uri);
      if (!optionalDocument.has_value())
            return false;

      textDocument &document = optionalDocument.value();
      for (auto &j : params.contentChanges)
      {
            if (j.range.has_value())
            {
                  const Range &contentChanged = j.range.value();

                  int startIndex = document.findPos(contentChanged.start.line, contentChanged.start.character);
                  int endIndex = document.findPos(contentChanged.end.line, contentChanged.end.character);

                  document.m_content.replace(startIndex, endIndex - startIndex, j.text);
            }
            else
                  document.m_content = j.text;
      }

      return true;
}
bool DocumentHandler::openDocument(const std::string &uri, const std::string &document)
{
      return m_openDocuments.emplace(uri, document).second;
}
bool DocumentHandler::closeDocument(const std::string &uri)
{
      return m_openDocuments.erase(uri) > 0;
}
bool DocumentHandler::documentIsOpen(const std::string &uri) const
{
      return m_openDocuments.count(uri) != 0;
}
std::optional<std::reference_wrapper<textDocument>> DocumentHandler::getOpenDocument(const std::string &uri)
{
      auto it = m_openDocuments.find(uri);
      if (m_openDocuments.end() == it)
      {
            return std::nullopt;
      }
      return std::make_optional<std::reference_wrapper<textDocument>>(it->second);
}
