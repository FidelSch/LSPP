#pragma once
#include <cstddef>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>
#include <optional>

class Message
{
	char *m_buffer;
	size_t m_payloadSize;
	/*std::unique_ptr<char> m_buffer;*/
	nlohmann::json m_jsonData;

public:
	// Message(const char* buffer, const size_t& bufsize);
	Message(std::istream &buffer);
	Message();
	~Message();
	std::string get() const;
	nlohmann::json jsonData() const;
	int readMessage(std::istream &stream);

	std::string method_description() const;
	nlohmann::json params() const;
	std::optional<int> id() const;
	std::string documentURI() const;

	static void log(const std::string_view &s);

	enum Method
	{
		NONE,
		INITIALIZE,
		SHUTDOWN,
		EXIT,
		HOVER,
		DEFINITION,
		DECLARATION,
		TYPE_DEFINITION,
		IMPLEMENTATION,
		REFERENCES,
		PREPARE_CALL_HIERARCHY,
		INCOMING_CALLS,
		OUTGOING_CALLS,
		PREPARE_TYPE_HIERARCHY,
		TYPE_HIERARCHY_SUPERTYPES,
		TYPE_HIERARCHY_SUBTYPES,
		TEXT_DOCUMENT_HIGHLIGHT,
		TEXT_DOCUMENT_DOCUMENT_LINK,
		DOCUMENT_LINK_RESOLVE,
		TEXT_DOCUMENT_CODE_LENS,
		CODE_LENS_RESOLVE,
		TEXT_DOCUMENT_FOLDING_RANGE,
		TEXT_DOCUMENT_SELECTION_RANGE,
		TEXT_DOCUMENT_DOCUMENT_SYMBOL,
		TEXT_DOCUMENT_SEMANTIC_TOKENS_FULL,
		TEXT_DOCUMENT_SEMANTIC_TOKENS_FULL_DELTA,
		TEXT_DOCUMENT_SEMANTIC_TOKENS_RANGE,
		TEXT_DOCUMENT_SEMANTIC_TOKENS_REFRESH,
		TEXT_DOCUMENT_INLAY_HINT,
		INLAY_HINT_RESOLVE,
		TEXT_DOCUMENT_INLINE_VALUE,
		TEXT_DOCUMENT_MONIKER,
		TEXT_DOCUMENT_COMPLETION,
		COMPLETION_ITEM_RESOLVE,
		TEXT_DOCUMENT_DIAGNOSTIC,
		WORKSPACE_DIAGNOSTIC,
		TEXT_DOCUMENT_SIGNATURE_HELP,
		TEXT_DOCUMENT_CODE_ACTION,
		CODE_ACTION_RESOLVE,
		TEXT_DOCUMENT_DOCUMENT_COLOR,
		TEXT_DOCUMENT_COLOR_PRESENTATION,
		TEXT_DOCUMENT_FORMATTING,
		TEXT_DOCUMENT_RANGE_FORMATTING,
		TEXT_DOCUMENT_ON_TYPE_FORMATTING,
		TEXT_DOCUMENT_RENAME,
		TEXT_DOCUMENT_PREPARE_RENAME,
		TEXT_DOCUMENT_LINKED_EDITING_RANGE,
		WORKSPACE_CODE_LENS_REFRESH,
		WORKSPACE_INLAY_HINT_REFRESH,
		WORKSPACE_INLINE_VALUE_REFRESH,
		TEXT_DOCUMENT_PUBLISH_DIAGNOSTICS,
		WORKSPACE_DIAGNOSTIC_REFRESH,
		TEXT_DOCUMENT_DID_OPEN,
		TEXT_DOCUMENT_DID_CHANGE,
		TEXT_DOCUMENT_DID_CLOSE
	};

	Method method() const;
};

class Response
{
public:
	nlohmann::json data;

	explicit Response(const Message &message) : data({{"id", message.id()}}) {}

	void setResult(const nlohmann::json &result)
	{
		data["result"] = result;
	}

	void setError(const nlohmann::json &error)
	{
		data["error"] = error;
	}

	std::string toString() const
	{
		try
		{
			// Dump JSON once and reuse
			std::string json_str = data.dump();

			// Pre-allocate to avoid multiple reallocations during concatenation
			std::string result;
			result.reserve(json_str.length() + 50); // Header + CRLF + safety margin

			result = "Content-Length: ";
			result += std::to_string(json_str.length());
			result += "\r\n\r\n";
			result += json_str;

			return result;
		}
		catch (const std::bad_alloc &)
		{
			// Return minimal valid response on allocation failure
			return "Content-Length: 2\r\n\r\n{}";
		}
	}
};