#include <thread>
#include "ProtocolStructures.hpp"


class LSPServer {
      std::thread m_listener;
      bool is_running;
      ServerCapabilities m_capabilities;
public:
      LSPServer();
      ~LSPServer();
      int init(const uint64_t& capabilities);
      void stop();
      static void server_main(LSPServer* server);
};

hoverResult hoverCallback(const hoverParams &params);
