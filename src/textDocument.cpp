#include "textDocument.hpp"
#include <sstream>

textDocument::textDocument() : m_content("") {}

textDocument::textDocument(const std::string &content) : m_content(content) {}

std::string textDocument::getLine(int n)
{
      std::istringstream stream(m_content);
      std::string line;

      if (n < 0)
            return "";

      do
      {
            std::getline(stream, line);
            n--;
      } while (n >= 0 && stream.good());

      if (n == -1)
            return line;
      else
            return "";
}

bool textDocument::isWordDelimiter(const char c)
{
      for (size_t i = 0; i < sizeof(word_delimiters); i++)
            if (c == word_delimiters[i])
                  return true;

      return false;
}

std::string textDocument::wordUnderCursor(const int line, const int character)
{
      std::string lineContents = getLine(line);

      if (character < 0 || character >= lineContents.length())
            return "";
      if (textDocument::isWordDelimiter(lineContents[character]))
            return "";

      int word_start = character, word_end = character;

      while (word_start >= 0 && !textDocument::isWordDelimiter(lineContents.at(word_start)))
      {
            word_start--;
      }
      while (word_end < lineContents.length() && !textDocument::isWordDelimiter(lineContents.at(word_end)))
      {
            word_end++;
      }

      word_start++;
      return lineContents.substr(word_start, word_end - word_start);
}

int textDocument::findPos(const int line, const int character) const
{
      std::istringstream stream(m_content);
      std::string lineContents;
      int result = 0;

      for (int l = 0; l < line; l++)
      {
            std::getline(stream, lineContents);
            result += lineContents.length() + 1; // Take into account \n character
      }

      for (int c = 0; c < character; c++)
      {
            stream.get();
            result++;
      }
      return result;
}