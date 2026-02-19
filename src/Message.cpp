#include "Message.hpp"
#include <cstring>
#include <iostream>
#include <string>
#include <climits>
#include <cstddef>
#include <cassert>
#include <fstream>
#include <optional>
#include <algorithm>

Message::Message() : m_buffer(nullptr), m_payloadSize(0), m_jsonData(nlohmann::json::value_t::null) {}

Message::~Message()
{
	if (m_buffer)
	{
		free(m_buffer);
	}
}

Message::Message(std::istream &buffer) : m_buffer(nullptr), m_payloadSize(0), m_jsonData(nlohmann::json::value_t::null)
{
	if (buffer.peek() == EOF)
	{
		return;
	}
	readMessage(buffer);
}

std::string Message::get() const
{
	if (!m_buffer)
		return "";
	try
	{
		return std::string(m_buffer);
	}
	catch (const std::bad_alloc &)
	{
		// Return empty string on allocation failure
		return "";
	}
}

nlohmann::json Message::jsonData() const
{
	return m_jsonData;
}

int Message::readMessage(std::istream &stream)
{
	// Free old buffer if exists (for message reuse)
	if (m_buffer)
	{
		free(m_buffer);
		m_buffer = nullptr;
	}
	m_payloadSize = 0;

	// Check if stream is in good state before reading
	if (!stream.good() && !stream.eof())
	{
		return -1; // Signal error (but allow eof)
	}

	// Read header line: "Content-Length: <size>" (case-insensitive)
	std::string headerLine;
	if (!std::getline(stream, headerLine))
	{
		return -1; // Failed to read header
	}

	// Remove trailing whitespace/carriage return
	while (!headerLine.empty() && (headerLine.back() == '\r' || headerLine.back() == '\n' || headerLine.back() == ' '))
	{
		headerLine.pop_back();
	}

	// Parse "Content-Length: <size>" (case-insensitive)
	const std::string headerPrefixLower = "content-length: ";
	std::string headerLower = headerLine;
	std::transform(headerLower.begin(), headerLower.end(), headerLower.begin(), ::tolower);
	if (headerLower.size() < headerPrefixLower.size() || headerLower.substr(0, headerPrefixLower.size()) != headerPrefixLower)
	{
		return -1; // Invalid header format
	}

	size_t temp_size = 0;
	try
	{
		std::string sizeStr = headerLine.substr(headerPrefixLower.size());
		temp_size = std::stoul(sizeStr);

		// Use reasonable upper limit (10MB) to prevent bad_alloc from malformed/malicious data
		constexpr size_t MAX_MESSAGE_SIZE = 10 * 1024 * 1024; // 10 MB
		if (temp_size == 0 || temp_size > MAX_MESSAGE_SIZE)
		{
			return -1; // Invalid size
		}
	}
	catch (const std::exception &)
	{
		return -1; // Parsing failed
	}

	// Skip the blank line (\r\n)
	std::string blankLine;
	if (!std::getline(stream, blankLine))
	{
		return -1; // Failed to skip blank line
	}

	// Allocate buffer for payload
	m_buffer = static_cast<char *>(malloc(temp_size + 1));
	if (!m_buffer)
	{
		return -1;
	}

	// Read JSON payload - use a new scope to ensure stream ref goes out of scope quickly
	{
		stream.read(m_buffer, temp_size);
		std::streamsize bytesRead = stream.gcount();
		if (bytesRead != static_cast<std::streamsize>(temp_size))
		{
			free(m_buffer);
			m_buffer = nullptr;
			return -1;
		}
	}

	// Null-terminate the buffer
	m_buffer[temp_size] = '\0';
	m_payloadSize = temp_size;

	// Parse JSON with exception handling
	try
	{
		m_jsonData = nlohmann::json::parse(m_buffer, nullptr, false);
	}
	catch (const std::bad_alloc &)
	{
		// Memory allocation failed during parsing - free resources
		free(m_buffer);
		m_buffer = nullptr;
		m_payloadSize = 0;
		return -1;
	}
	catch (const std::exception &)
	{
		// JSON parsing failed - still keep the buffer
	}

	return m_payloadSize;
}

std::string Message::method_description() const
{
	if (m_jsonData.is_discarded() || m_jsonData.is_null() || !m_jsonData.contains("method"))
		return "";
	return m_jsonData["method"];
}

Message::Method Message::method() const
{
	return stringToMethod(method_description());
}

std::string Message::methodToString(Message::Method method)
{
	static const std::unordered_map<Method, std::string> reverseMap = {
	    {Method::INITIALIZE, "initialize"},
	    {Method::SHUTDOWN, "shutdown"},
	    {Method::EXIT, "exit"},
	    {Method::TEXT_DOCUMENT_DID_OPEN, "textDocument/didOpen"},
	    {Method::TEXT_DOCUMENT_DID_CHANGE, "textDocument/didChange"},
	    {Method::TEXT_DOCUMENT_DID_CLOSE, "textDocument/didClose"},
	    {Method::DECLARATION, "textDocument/declaration"},
	    {Method::DEFINITION, "textDocument/definition"},
	    {Method::TYPE_DEFINITION, "textDocument/typeDefinition"},
	    {Method::IMPLEMENTATION, "textDocument/implementation"},
	    {Method::REFERENCES, "textDocument/references"},
	    {Method::PREPARE_CALL_HIERARCHY, "textDocument/prepareCallHierarchy"},
	    {Method::INCOMING_CALLS, "callHierarchy/incomingCalls"},
	    {Method::OUTGOING_CALLS, "callHierarchy/outgoingCalls"},
	    {Method::PREPARE_TYPE_HIERARCHY, "textDocument/prepareTypeHierarchy"},
	    {Method::TYPE_HIERARCHY_SUPERTYPES, "typeHierarchy/supertypes"},
	    {Method::TYPE_HIERARCHY_SUBTYPES, "typeHierarchy/subtypes"},
	    {Method::TEXT_DOCUMENT_HIGHLIGHT, "textDocument/documentHighlight"},
	    {Method::TEXT_DOCUMENT_DOCUMENT_LINK, "textDocument/documentLink"},
	    {Method::DOCUMENT_LINK_RESOLVE, "documentLink/resolve"},
	    {Method::HOVER, "textDocument/hover"},
	    {Method::TEXT_DOCUMENT_CODE_LENS, "textDocument/codeLens"},
	    {Method::CODE_LENS_RESOLVE, "codeLens/resolve"},
	    {Method::TEXT_DOCUMENT_FOLDING_RANGE, "textDocument/FoldingRange"},
	    {Method::TEXT_DOCUMENT_SELECTION_RANGE, "textDocument/selectionRange"},
	    {Method::TEXT_DOCUMENT_DOCUMENT_SYMBOL, "textDocument/documentSymbol"},
	    {Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_FULL, "textDocument/semanticTokens/full"},
	    {Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_FULL_DELTA, "textDocument/semanticTokens/full/delta"},
	    {Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_RANGE, "textDocument/semanticTokens/range"},
	    {Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_REFRESH, "textDocument/semanticTokens/refresh"},
	    {Method::TEXT_DOCUMENT_INLAY_HINT, "textDocument/inlayHint"},
	    {Method::INLAY_HINT_RESOLVE, "inlayHint/resolve"},
	    {Method::TEXT_DOCUMENT_INLINE_VALUE, "textDocument/inlineValue"},
	    {Method::TEXT_DOCUMENT_MONIKER, "textDocument/moniker"},
	    {Method::TEXT_DOCUMENT_COMPLETION, "textDocument/completion"},
	    {Method::COMPLETION_ITEM_RESOLVE, "completionItem/resolve"},
	    {Method::TEXT_DOCUMENT_DIAGNOSTIC, "textDocument/diagnostic"},
	    {Method::WORKSPACE_DIAGNOSTIC, "workspace/diagnostic"},
	    {Method::TEXT_DOCUMENT_SIGNATURE_HELP, "textDocument/signatureHelp"},
	    {Method::TEXT_DOCUMENT_CODE_ACTION, "textDocument/codeAction"},
	    {Method::CODE_ACTION_RESOLVE, "codeAction/resolve"},
	    {Method::TEXT_DOCUMENT_DOCUMENT_COLOR, "textDocument/documentColor"},
	    {Method::TEXT_DOCUMENT_COLOR_PRESENTATION, "textDocument/colorPresentation"},
	    {Method::TEXT_DOCUMENT_FORMATTING, "textDocument/formatting"},
	    {Method::TEXT_DOCUMENT_RANGE_FORMATTING, "textDocument/rangeFormatting"},
	    {Method::TEXT_DOCUMENT_ON_TYPE_FORMATTING, "textDocument/onTypeFormatting"},
	    {Method::TEXT_DOCUMENT_RENAME, "textDocument/rename"},
	    {Method::TEXT_DOCUMENT_PREPARE_RENAME, "textDocument/prepareRename"},
	    {Method::TEXT_DOCUMENT_LINKED_EDITING_RANGE, "textDocument/linkedEditingRange"},
	    {Method::WORKSPACE_CODE_LENS_REFRESH, "workspace/codeLens/refresh"},
	    {Method::WORKSPACE_INLAY_HINT_REFRESH, "workspace/inlayHint/refresh"},
	    {Method::WORKSPACE_INLINE_VALUE_REFRESH, "workspace/inlineValue/refresh"},
	    {Method::TEXT_DOCUMENT_PUBLISH_DIAGNOSTICS, "textDocument/publishDiagnostics"},
	    {Method::WORKSPACE_DIAGNOSTIC_REFRESH, "workspace/diagnostic/refresh"},
	};

	auto it = reverseMap.find(method);
	if (it != reverseMap.end())
	{
		return it->second;
	}
	return ""; // Unknown method
}

nlohmann::json Message::params() const
{
	if (m_jsonData.is_discarded() || m_jsonData.is_null() || !m_jsonData.contains("params"))
		return nlohmann::json::object();
	return m_jsonData["params"];
}

std::optional<int> Message::id() const
{
	if (m_jsonData.is_discarded() || m_jsonData.is_null() || !m_jsonData.contains("id"))
		return std::nullopt;
	return m_jsonData["id"].get<int>();
}

std::string Message::documentURI() const
{
	if (m_jsonData.is_discarded() || m_jsonData.is_null())
		return "";
	auto p = params();
	if (!p.contains("textDocument") || !p["textDocument"].contains("uri"))
		return "";
	return p["textDocument"]["uri"];
}

void Message::log(const std::string_view &s)
{
	static char *logfile = std::getenv("LSPP_LOG_FILE");

	if (!(logfile))
	{
		return;
	}
	try
	{
		std::ofstream file(logfile, std::ios::app);
		file << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << ">>" << s << '\n';
		file.close();
	}
	catch (...)
	{
		// Silently fail - logging is non-critical
	}
}

Message::Method Message::stringToMethod(const std::string &methodStr)
{
	static const std::unordered_map<std::string, Method> methodMap = {
	    // Notifications
	    {"initialize", Message::Method::INITIALIZE},
	    {"shutdown", Message::Method::SHUTDOWN},
	    {"exit", Message::Method::EXIT},
	    {"textDocument/didOpen", Message::Method::TEXT_DOCUMENT_DID_OPEN},
	    {"textDocument/didChange", Message::Method::TEXT_DOCUMENT_DID_CHANGE},
	    {"textDocument/didClose", Message::Method::TEXT_DOCUMENT_DID_CLOSE},

	    // Requests
	    {"textDocument/declaration", Message::Method::DECLARATION},
	    {"textDocument/definition", Message::Method::DEFINITION},
	    {"textDocument/typeDefinition", Message::Method::TYPE_DEFINITION},
	    {"textDocument/implementation", Message::Method::IMPLEMENTATION},
	    {"textDocument/references", Message::Method::REFERENCES},
	    {"textDocument/prepareCallHierarchy", Message::Method::PREPARE_CALL_HIERARCHY},
	    {"callHierarchy/incomingCalls", Message::Method::INCOMING_CALLS},
	    {"callHierarchy/outgoingCalls", Message::Method::OUTGOING_CALLS},
	    {"textDocument/prepareTypeHierarchy", Message::Method::PREPARE_TYPE_HIERARCHY},
	    {"typeHierarchy/supertypes", Message::Method::TYPE_HIERARCHY_SUPERTYPES},
	    {"typeHierarchy/subtypes", Message::Method::TYPE_HIERARCHY_SUBTYPES},
	    {"textDocument/documentHighlight", Message::Method::TEXT_DOCUMENT_HIGHLIGHT},
	    {"textDocument/documentLink", Message::Method::TEXT_DOCUMENT_DOCUMENT_LINK},
	    {"documentLink/resolve", Message::Method::DOCUMENT_LINK_RESOLVE},
	    {"textDocument/hover", Message::Method::HOVER},
	    {"textDocument/codeLens", Message::Method::TEXT_DOCUMENT_CODE_LENS},
	    {"codeLens/resolve", Message::Method::CODE_LENS_RESOLVE},
	    {"textDocument/FoldingRange", Message::Method::TEXT_DOCUMENT_FOLDING_RANGE},
	    {"textDocument/selectionRange", Message::Method::TEXT_DOCUMENT_SELECTION_RANGE},
	    {"textDocument/documentSymbol", Message::Method::TEXT_DOCUMENT_DOCUMENT_SYMBOL},
	    {"textDocument/semanticTokens/full", Message::Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_FULL},
	    {"textDocument/semanticTokens/full/delta", Message::Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_FULL_DELTA},
	    {"textDocument/semanticTokens/range", Message::Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_RANGE},
	    {"textDocument/semanticTokens/refresh", Message::Method::TEXT_DOCUMENT_SEMANTIC_TOKENS_REFRESH},
	    {"textDocument/inlayHint", Message::Method::TEXT_DOCUMENT_INLAY_HINT},
	    {"inlayHint/resolve", Message::Method::INLAY_HINT_RESOLVE},
	    {"textDocument/inlineValue", Message::Method::TEXT_DOCUMENT_INLINE_VALUE},
	    {"textDocument/moniker", Message::Method::TEXT_DOCUMENT_MONIKER},
	    {"textDocument/completion", Message::Method::TEXT_DOCUMENT_COMPLETION},
	    {"completionItem/resolve", Message::Method::COMPLETION_ITEM_RESOLVE},
	    {"textDocument/diagnostic", Message::Method::TEXT_DOCUMENT_DIAGNOSTIC},
	    {"workspace/diagnostic", Message::Method::WORKSPACE_DIAGNOSTIC},
	    {"textDocument/signatureHelp", Message::Method::TEXT_DOCUMENT_SIGNATURE_HELP},
	    {"textDocument/codeAction", Message::Method::TEXT_DOCUMENT_CODE_ACTION},
	    {"codeAction/resolve", Message::Method::CODE_ACTION_RESOLVE},
	    {"textDocument/documentColor", Message::Method::TEXT_DOCUMENT_DOCUMENT_COLOR},
	    {"textDocument/colorPresentation", Message::Method::TEXT_DOCUMENT_COLOR_PRESENTATION},
	    {"textDocument/formatting", Message::Method::TEXT_DOCUMENT_FORMATTING},
	    {"textDocument/rangeFormatting", Message::Method::TEXT_DOCUMENT_RANGE_FORMATTING},
	    {"textDocument/onTypeFormatting", Message::Method::TEXT_DOCUMENT_ON_TYPE_FORMATTING},
	    {"textDocument/rename", Message::Method::TEXT_DOCUMENT_RENAME},
	    {"textDocument/prepareRename", Message::Method::TEXT_DOCUMENT_PREPARE_RENAME},
	    {"textDocument/linkedEditingRange", Message::Method::TEXT_DOCUMENT_LINKED_EDITING_RANGE},

	    // Server-side requests
	    {"workspace/codeLens/refresh", Message::Method::WORKSPACE_CODE_LENS_REFRESH},
	    {"workspace/inlayHint/refresh", Message::Method::WORKSPACE_INLAY_HINT_REFRESH},
	    {"workspace/inlineValue/refresh", Message::Method::WORKSPACE_INLINE_VALUE_REFRESH},

	    // Server-side notifications
	    {"textDocument/publishDiagnostics", Message::Method::TEXT_DOCUMENT_PUBLISH_DIAGNOSTICS},
	    {"workspace/diagnostic/refresh", Message::Method::WORKSPACE_DIAGNOSTIC_REFRESH},
	};

	if (methodMap.count(methodStr))
	{
		return methodMap.at(methodStr);
	}
	return Message::Method::NONE; // Default case
}
