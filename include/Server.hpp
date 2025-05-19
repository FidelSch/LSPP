#include <thread>
#include "ProtocolStructures.hpp"


class LSPServer {
      std::thread m_listener;
      bool force_shutdown;
      ServerCapabilities m_capabilities;
public:
      LSPServer();
      ~LSPServer();
      int init(const uint64_t& capabilities);
      void stop();
      int exit();
      static void server_main(LSPServer* server);
};

hoverResult hoverCallback(const hoverParams &params);
definitionResult definitionCallback(const definitionParams &params);
