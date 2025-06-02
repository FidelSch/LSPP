#include <string>

struct textDocument {
      static constexpr const char word_delimiters[] = " `~!@#$%^&*()-=+[{]}\\|;:'\",.<>/?";
      // std::string m_uri;
      std::string m_content;

      textDocument();
      textDocument(const std::string& content);
      std::string getLine(int n);
      static bool isWordDelimiter(const char c);
      std::string wordUnderCursor(const int line, const int column);
      int findPos(const int line, const int column) const;
};
