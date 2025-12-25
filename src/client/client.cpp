#include "client.hpp"
#include "engine/types.hpp"

Client::Client(client_id id) : id(id) {
    initialized = false;
}

Client::~Client() {

}

void Client::update() {
    ServerMsg_Connection connMsg;
    connMsg.clientId = id;
    snprintf(connMsg.name, sizeof(connMsg.name), "Client_%u", id);
}

