#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include "types.hpp"
#include "player.hpp"

#define BULLET_SIZE 1

class Shot {
private:
    glm::vec3 position;
    glm::vec3 velocity;
    client_id ownerId;
    shot_id id;
    static shot_id GenerateLocalShotId();

public:
    Shot(shot_id globalId, glm::vec3 position, glm::vec3 velocity)
        : id(globalId), position(position), velocity(velocity) {
        ownerId = 1;
    }
    Shot(glm::vec3 position, glm::vec3 velocity)
        : Shot(GenerateLocalShotId(), position, velocity) {
        ownerId = 0;
    }

    void Update(float deltaTime);

    glm::vec3 GetPosition() const {
        return position;
    }

    template<ClientMsgSubType T> T Get() {
        if constexpr (std::is_same_v<T, ClientMsg_Shot>) {
            ClientMsg_Shot msg;
            msg.type = ClientMsg_Type_SHOT;
            msg.localShotId = id;
            msg.position = position;
            msg.velocity = velocity;
            return msg;
        } else {
            static_assert(sizeof(T) == 0, "Client: Unsupported message type");
        }
    }

    shot_id GetId() {
        return id;
    }

    client_id GetOwnerId() {
        return ownerId;
    }
};