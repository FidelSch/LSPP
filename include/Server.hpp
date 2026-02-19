#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <mutex>
#include "ProtocolStructures.hpp"
#include "textDocument.hpp"
#include "Message.hpp"
#include "iostream"

class DocumentHandler
{
      std::map<std::string, textDocument> m_openDocuments;

public:
      bool openDocument(const std::string &uri, const std::string &document);
      bool closeDocument(const std::string &uri);
      bool updateDocument(const std::string &uri, const DidChangeTextDocumentParams &params);
      bool documentIsOpen(const std::string &uri) const;

      // Returns a reference to the open document if it exists
      std::optional<std::reference_wrapper<textDocument>> getOpenDocument(const std::string &uri);
};

class LSPServer
{
      std::thread m_listener;
      std::atomic<bool> force_shutdown;
      std::atomic<bool> thread_exiting;  // Signals when thread is about to exit
      bool isOKtoExit;
      bool m_shutdownRequested;
      bool m_initialized;
      ServerCapabilities m_capabilities;

      std::istream *m_input_stream;
      std::ostream *m_output_stream;
      mutable std::mutex m_output_mutex; // Protects m_output_stream (mutable for const methods)

      // Generic callback storage
      std::unordered_map<std::string, std::function<nlohmann::json(const nlohmann::json &)>> m_callbacks;

protected:
      DocumentHandler m_documentHandler;

public:
      LSPServer();
      ~LSPServer();
      int init(const uint64_t &capabilities, std::istream &in = std::cin, std::ostream &out = std::cout);
      void stop();
      int exit();
      static void server_main(LSPServer *server);
      bool hasCapability(uint64_t capability) const;
      Response processRequest(const Message &message);
      void processNotification(const Message &message);
      void send(const Response &response, bool flush = false);
      
      // Thread-safe method to get output (for testing)
      std::string getOutputSafe(std::ostringstream *out_stream) const;


      // Generic callback registration (by string)
      template <typename ParamsT, typename ResultT>
      void registerCallback(const std::string &method, std::function<ResultT(const ParamsT &)> callback)
      {
            m_callbacks[method] = [callback](const nlohmann::json &params) -> nlohmann::json
            {
                  ParamsT typedParams = params.get<ParamsT>();
                  ResultT result = callback(typedParams);
                  return nlohmann::json(result);
            };
      }

      // Generic callback registration (by Method enum)
      template <typename ParamsT, typename ResultT>
      void registerCallback(Message::Method method, std::function<ResultT(const ParamsT &)> callback)
      {
            registerCallback<ParamsT, ResultT>(Message::methodToString(method), callback);
      }

      std::optional<nlohmann::json> invokeCallback(const Message &message)
      {
            return invokeCallback(message.method_description(), message.params());
      }

      std::optional<nlohmann::json> invokeCallback(const Message::Method &method, const nlohmann::json &params)
      {
            return invokeCallback(Message::methodToString(method), params);
      }

      // Helper to invoke callbacks
      std::optional<nlohmann::json> invokeCallback(const std::string &method, const nlohmann::json &params)
      {
            auto it = m_callbacks.find(method);
            if (it != m_callbacks.end())
            {
                  return it->second(params);
            }
            return std::nullopt;
      }

      static uint64_t capabilityFlagForMethod(Message::Method method)
      {
            static const std::unordered_map<Message::Method, uint64_t> capabilityMap = {
                {Message::Method::HOVER, ServerCapabilities::hoverProvider},
                {Message::Method::DEFINITION, ServerCapabilities::definitionProvider},
                {Message::Method::DECLARATION, ServerCapabilities::declarationProvider},
                {Message::Method::TYPE_DEFINITION, ServerCapabilities::typeDefinitionProvider},
                {Message::Method::IMPLEMENTATION, ServerCapabilities::implementationProvider},
                {Message::Method::REFERENCES, ServerCapabilities::referencesProvider},
                {Message::Method::TEXT_DOCUMENT_HIGHLIGHT, ServerCapabilities::documentHighlightProvider},
                {Message::Method::TEXT_DOCUMENT_DOCUMENT_SYMBOL, ServerCapabilities::documentSymbolProvider},
                {Message::Method::TEXT_DOCUMENT_CODE_ACTION, ServerCapabilities::codeActionProvider},
                {Message::Method::TEXT_DOCUMENT_CODE_LENS, ServerCapabilities::codeLensProvider},
                {Message::Method::TEXT_DOCUMENT_DOCUMENT_LINK, ServerCapabilities::documentLinkProvider},
                {Message::Method::DOCUMENT_LINK_RESOLVE, ServerCapabilities::documentLinkProvider},
                {Message::Method::TEXT_DOCUMENT_DOCUMENT_COLOR, ServerCapabilities::colorProvider},
                {Message::Method::TEXT_DOCUMENT_COLOR_PRESENTATION, ServerCapabilities::colorProvider},
                {Message::Method::TEXT_DOCUMENT_FORMATTING, ServerCapabilities::documentFormattingProvider},
                {Message::Method::TEXT_DOCUMENT_RANGE_FORMATTING, ServerCapabilities::documentRangeFormattingProvider},
                {Message::Method::TEXT_DOCUMENT_ON_TYPE_FORMATTING, ServerCapabilities::documentOnTypeFormattingProvider},
                {Message::Method::TEXT_DOCUMENT_RENAME, ServerCapabilities::renameProvider},
                {Message::Method::TEXT_DOCUMENT_PREPARE_RENAME, ServerCapabilities::renameProvider},
                {Message::Method::TEXT_DOCUMENT_FOLDING_RANGE, ServerCapabilities::foldingRangeProvider},
                {Message::Method::TEXT_DOCUMENT_SELECTION_RANGE, ServerCapabilities::selectionRangeProvider},
                {Message::Method::TEXT_DOCUMENT_LINKED_EDITING_RANGE, ServerCapabilities::linkedEditingRangeProvider},
                {Message::Method::TEXT_DOCUMENT_SIGNATURE_HELP, ServerCapabilities::signatureHelpProvider},
                {Message::Method::TEXT_DOCUMENT_COMPLETION, ServerCapabilities::completionProvider},
                {Message::Method::COMPLETION_ITEM_RESOLVE, ServerCapabilities::completionProvider},
                {Message::Method::TEXT_DOCUMENT_DIAGNOSTIC, ServerCapabilities::diagnosticProvider},
                {Message::Method::WORKSPACE_DIAGNOSTIC, ServerCapabilities::diagnosticProvider},
                {Message::Method::PREPARE_CALL_HIERARCHY, ServerCapabilities::callHierarchyProvider},
                {Message::Method::INCOMING_CALLS, ServerCapabilities::callHierarchyProvider},
                {Message::Method::OUTGOING_CALLS, ServerCapabilities::callHierarchyProvider},
                {Message::Method::PREPARE_TYPE_HIERARCHY, ServerCapabilities::typeHierarchyProvider},
                {Message::Method::TYPE_HIERARCHY_SUPERTYPES, ServerCapabilities::typeHierarchyProvider},
                {Message::Method::TYPE_HIERARCHY_SUBTYPES, ServerCapabilities::typeHierarchyProvider},
                {Message::Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_FULL, ServerCapabilities::semanticTokensProvider},
                {Message::Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_FULL_DELTA, ServerCapabilities::semanticTokensProvider},
                {Message::Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_RANGE, ServerCapabilities::semanticTokensProvider},
                {Message::Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_REFRESH, ServerCapabilities::semanticTokensProvider},
                {Message::Method::TEXT_DOCUMENT_INLAY_HINT, ServerCapabilities::inlayHintProvider},
                {Message::Method::INLAY_HINT_RESOLVE, ServerCapabilities::inlayHintProvider},
                {Message::Method::TEXT_DOCUMENT_INLINE_VALUE, ServerCapabilities::inlineValueProvider},
                {Message::Method::TEXT_DOCUMENT_MONIKER, ServerCapabilities::monikerProvider},
                {Message::Method::WORKSPACE_CODE_LENS_REFRESH, ServerCapabilities::codeLensProvider},
                {Message::Method::WORKSPACE_INLAY_HINT_REFRESH, ServerCapabilities::inlayHintProvider},
                {Message::Method::WORKSPACE_INLINE_VALUE_REFRESH, ServerCapabilities::inlineValueProvider},
                // Note: The following don't have direct capability flags or are special cases
                // Message::Method::INITIALIZE - special lifecycle method
                // Message::Method::SHUTDOWN - special lifecycle method
                // Message::Method::EXIT - special lifecycle method
                // Message::Method::TEXT_DOCUMENT_DID_OPEN - notification
                // Message::Method::TEXT_DOCUMENT_DID_CHANGE - notification
                // Message::Method::TEXT_DOCUMENT_DID_CLOSE - notification
                // Message::Method::TEXT_DOCUMENT_PUBLISH_DIAGNOSTICS - server notification
                // Message::Method::WORKSPACE_DIAGNOSTIC_REFRESH - server notification
                // Message::Method::CODE_LENS_RESOLVE - uses codeLensProvider
                // Message::Method::CODE_ACTION_RESOLVE - uses codeActionProvider
            };

            auto it = capabilityMap.find(method);
            if (it != capabilityMap.end())
            {
                  return it->second;
            }
            return 0;
      }
};