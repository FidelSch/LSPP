#pragma once
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <optional>
#include <vector>
#include <nlohmann/json.hpp>

#include "Server.hpp"

namespace testutil
{

      inline std::string makeWireMessage(const std::string &jsonPayload)
      {
            return "Content-Length: " + std::to_string(jsonPayload.size()) + "\r\n\r\n" + jsonPayload;
      }

      struct WireResponse
      {
            std::string wire;
            std::string header;
            std::string body;
            size_t contentLength{0};
            nlohmann::json json; // parsed first message body
            int serverExitCode{0};
      };

      inline std::vector<nlohmann::json> parseAllResponses(const std::string &wire)
      {
            static constexpr size_t content_length_header_size = 16; // "Content-Length: ".size()
            std::vector<nlohmann::json> out;
            if (wire.empty())
                  return out;
            size_t pos = 0;
            while (true)
            {
                  auto headerEnd = wire.find("\r\n\r\n", pos);
                  if (headerEnd == std::string::npos)
                        break;

                  std::string_view header = std::string_view(wire).substr(pos, headerEnd - pos);
                  auto lenPos = header.find("Content-Length: ");
                  if (lenPos == std::string::npos)
                        break;

                  size_t lenStart = lenPos + content_length_header_size;
                  size_t lenEnd = header.find('\r', lenStart);
                  if (lenEnd == std::string::npos)
                        lenEnd = header.size();
                  if (lenEnd <= lenStart)
                        break;

                  size_t expected = static_cast<size_t>(std::stoul(std::string(header.substr(lenStart, lenEnd - lenStart))));
                  size_t bodyStart = headerEnd + 4;
                  if (wire.size() < bodyStart + expected)
                        break;
                  out.emplace_back(nlohmann::json::parse(wire.substr(bodyStart, expected)));
                  pos = bodyStart + expected;
            }
            return out;
      }

      /* Launch server and run a single request */
      inline WireResponse runSingle(uint64_t capabilities,
                                    const std::string &payloadJson,
                                    bool stopAfter = false,
                                    std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
      {
            std::istringstream in(makeWireMessage(payloadJson));
            std::ostringstream out;

            LSPServer server;
            server.init(capabilities, in, out);

            // Wait for full response
            std::string wire;
            auto start = std::chrono::steady_clock::now();
            while (true)
            {
                  wire = out.str();
                  const auto headerEnd = wire.find("\r\n\r\n");
                  if (headerEnd != std::string::npos)
                  {
                        const auto header = wire.substr(0, headerEnd);
                        const auto pos = header.find("Content-Length: ");
                        if (pos != std::string::npos)
                        {
                              size_t lenStart = pos + std::string("Content-Length: ").size();
                              size_t lenEnd = header.find('\r', lenStart);
                              const auto lenStr = header.substr(lenStart, lenEnd - lenStart);
                              const size_t expected = static_cast<size_t>(std::stoul(lenStr));
                              const std::string body = wire.substr(headerEnd + 4);
                              if (body.size() >= expected)
                                    break;
                        }
                  }
                  if (std::chrono::steady_clock::now() - start > timeout)
                        break;
                  std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

            if (stopAfter)
                  server.stop();
            // exit() now also stops the server to guarantee join won't hang
            int code = server.exit();

            // Parse header/body and JSON according to Content-Length
            WireResponse resp;
            resp.wire = std::move(wire);
            resp.serverExitCode = code;
            const auto headerEnd = resp.wire.find("\r\n\r\n");
            if (headerEnd != std::string::npos)
            {
                  resp.header = resp.wire.substr(0, headerEnd);
                  resp.body = resp.wire.substr(headerEnd + 4);
                  const auto pos = resp.header.find("Content-Length: ");
                  if (pos != std::string::npos)
                  {
                        size_t lenStart = pos + std::string("Content-Length: ").size();
                        size_t lenEnd = resp.header.find('\r', lenStart);
                        const auto lenStr = resp.header.substr(lenStart, lenEnd - lenStart);
                        resp.contentLength = static_cast<size_t>(std::stoul(lenStr));
                        if (resp.body.size() >= resp.contentLength)
                        {
                              resp.json = nlohmann::json::parse(resp.body.substr(0, resp.contentLength));
                        }
                  }
            }

            return resp;
      }

      struct BatchResponse
      {
            std::vector<nlohmann::json> jsonResponses;
            int serverExitCode{0};
      };

      /* Run a batch of requests sequentially on a new server instance and return the responses */
      inline BatchResponse runBatch(uint64_t capabilities,
                                    const std::vector<std::string> &payloads,
                                    size_t expected_responses = 0,
                                    std::chrono::milliseconds timeout = std::chrono::milliseconds(3000),
                                    bool /*waitResponses*/ = true)
      {
            std::string all;
            all.reserve(1024);
            for (const auto &p : payloads)
                  all += makeWireMessage(p);

            std::istringstream in(all);
            std::ostringstream out;

            LSPServer server;
            server.init(capabilities, in, out);

            std::vector<nlohmann::json> responses;
            auto start = std::chrono::steady_clock::now();

            while (true)
            {
                  auto wire = out.str();
                  responses = parseAllResponses(wire);
                  if (responses.size() >= payloads.size())
                        break;
                  if (std::chrono::steady_clock::now() - start > timeout)
                  {
                        break;
                  }
                  if (expected_responses > 0 && responses.size() >= expected_responses)
                        break;

                  // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            // Shutdown server
            server.stop();

            int code = server.exit();

            return {responses, code};
      }

} // namespace testutil
