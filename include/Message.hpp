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

	std::string method();
	nlohmann::json params();
	int id();
	std::string documentURI();

	static void log(const std::string_view& s);
};
