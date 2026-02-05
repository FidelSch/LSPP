#include "Message.hpp"
#include <cstring>
#include <iostream>
#include <string>
#include <climits>
#include <cassert>
#include <fstream>
#include <optional>

Message::Message() : m_buffer(nullptr), m_payloadSize(0) {}

Message::~Message()
{
	if (m_buffer)
	{
		free(m_buffer);
	}
}

Message::Message(std::istream &buffer)
{
	if (buffer.peek() == EOF)
	{
		m_buffer = nullptr;
		m_payloadSize = 0;
		return;
	}
	readMessage(buffer);
}

std::string Message::get() const
{
	if (!m_buffer)
		return "";
	return std::string(m_buffer);
}

nlohmann::json Message::jsonData() const
{
	return m_jsonData;
}

int Message::readMessage(std::istream &stream)
{
	m_payloadSize = 0;

	stream.clear();
	stream.ignore(15);
	stream >> m_payloadSize;

	// TODO: handle this cleanly
	if (!(m_payloadSize > 0 && m_payloadSize < INT_MAX))
	{
		m_buffer = nullptr;
		m_payloadSize = 0;
		return 0;
	}

	m_buffer = static_cast<char *>(calloc(m_payloadSize + 2, 1));

	stream.clear();
	stream.ignore(4);

	stream.clear();
	stream.read(m_buffer, m_payloadSize);

	m_jsonData = nlohmann::json::parse(m_buffer, nullptr, false);

	return m_payloadSize;
}

std::string Message::method_description() const
{
	if (m_jsonData.is_discarded())
		return "";
	return m_jsonData["method"];
}

Message::Method Message::method() const
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

	if (methodMap.count(method_description()))
	{
		return methodMap.at(method_description());
	}
	return Message::Method::NONE; // Default case
}

nlohmann::json Message::params() const
{
	return m_jsonData["params"];
}

std::optional<int> Message::id() const
{
	if (m_jsonData.is_discarded() || !m_jsonData.contains("id"))
		return std::nullopt;
	return m_jsonData["id"].get<int>();
}

std::string Message::documentURI() const
{
	if (m_jsonData.is_discarded() || !params().contains("textDocument"))
		return "";
	return params()["textDocument"]["uri"];
}

void Message::log(const std::string_view &s)
{
	static char *logfile = std::getenv("LSPP_LOG_FILE");

	if (!(logfile))
	{
		return;
	}
	std::ofstream file(logfile, std::ios::app);
	file << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << ">>" << s << '\n';
	file.close();
}
