#include <cstddef>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

class Message {
	char* m_buffer;
	size_t m_payloadSize;
	/*std::unique_ptr<char> m_buffer;*/
	nlohmann::json m_jsonData;

public:
	// Message(const char* buffer, const size_t& bufsize);
	Message(std::istream& buffer);
	Message();
	~Message();
	std::string get() const;
	nlohmann::json jsonData() const;
	int readMessage(std::istream& stream);

	std::string method_description() const;
	nlohmann::json params() const;
	int id() const;
	std::string documentURI() const;

	static void log(const std::string_view& s);

	enum Method {
		NONE,
		INITIALIZE,
		SHUTDOWN,
		EXIT,
		HOVER,
		DEFINITION,
		DECLARATION,
		TEXT_DOCUMENT_DID_OPEN,
		TEXT_DOCUMENT_DID_CHANGE,
		TEXT_DOCUMENT_DID_CLOSE
	};

	Method method() const;
};
