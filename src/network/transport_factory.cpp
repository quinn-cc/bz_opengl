#include "network/transport_factory.hpp"

#include "network/enet_transport.hpp"

namespace net {

std::unique_ptr<IClientTransport> createDefaultClientTransport() {
    return createEnetClientTransport();
}

std::unique_ptr<IServerTransport> createDefaultServerTransport(uint16_t port, int maxClients, int numChannels) {
    return createEnetServerTransport(port, maxClients, numChannels);
}

} // namespace net
