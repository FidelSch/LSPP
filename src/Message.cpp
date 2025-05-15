#include "Message.hpp"
#include <cstring>
#include <iostream>
#include <string>
#include <climits>
#include <cassert>
#include <fstream>

Message::Message(): m_buffer(nullptr), m_payloadSize(0) {}

Message::~Message()
{
	if (m_buffer){
		free(m_buffer);
	}
}

Message::Message(std::istream& buffer) {
	if (buffer.peek() == EOF) {
		m_buffer = nullptr;
		m_payloadSize = 0;
		return;
	}
	readMessage(buffer);
}

std::string Message::get() const {
	if (!m_buffer) return "";
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
	if (!(m_payloadSize > 0 && m_payloadSize < INT_MAX)) {
		m_buffer = nullptr;
		m_payloadSize = 0;
		return 0;
	}

	m_buffer = static_cast<char*>(calloc(m_payloadSize+2, 1));

	stream.clear();
	stream.ignore(4);

	stream.clear();
	stream.read(m_buffer, m_payloadSize);

	m_jsonData = nlohmann::json::parse(m_buffer, nullptr, false);

	return m_payloadSize;
}

std::string Message::method()
{
	if (m_jsonData.is_discarded()) return "";
      return m_jsonData["method"];
}

nlohmann::json Message::params()
{
      return m_jsonData["params"];
}

int Message::id(){
	if (m_jsonData.is_discarded() || !m_jsonData.contains("id")) return 0;
	return m_jsonData["id"];
}

std::string Message::documentURI()
{
	if (m_jsonData.is_discarded() || !params().contains("textDocument")) return "";
      return params()["textDocument"]["uri"];
}

void Message::log(const std::string_view& s)
{
	static char* logfile = std::getenv("LSPP_LOG_FILE");
		
	if (!(logfile)){
		return;
	}
	std::ofstream file(logfile, std::ios::app);
	file << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << ">>" << s << '\n';
	file.close();
}
