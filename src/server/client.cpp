#include "client.hpp"

Client::Client(client_id id, std::string ip) : id(id), ip(ip) {
}

bool Client::isClient(client_id cid) const {
    return cid == id;
}