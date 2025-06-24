#include "player.hpp"
#include <enet.h>
#include <string.h>

void Player::sendPacket(void *data, int len) {
    ENetPacket* packet = enet_packet_create(
        data,
        len,
        ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT
    );

    enet_peer_send(peer, 0, packet);
    enet_host_flush(server);
}

std::string Player::toString() {
    std::string out = "";
    out += "Player{name=";
    out += name;
    out += ",";
    out += "clientId=";
    out += std::to_string(clientId);
    out += "}";
    return out;
}

bool Player::isPeer(ENetPeer *peer) {
    return this->peer == peer;
}

Player::Player() {

}