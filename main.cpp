//#define DEBUG_CONSOLE // Uncomment this if you want a debug console to start. You can use the Console class to print. You can use Console::inStrings to get input.

#include <4dm.h>
#include "JSONData.h"

using namespace fdm;

// Initialize the DLLMain
initDLL

// Send health from server

float getEntityHealth(Entity* entity) {
    if (entity->getName() == "Spider") return ((EntitySpider*)entity)->health;
    if (entity->getName() == "Butterfly")return ((EntityButterfly*)entity)->health;
    if (entity->getName() == "Player") return ((EntityPlayer*)entity)->player->health;
    return 0;
}

void sendPacketInRegion(WorldServer* world, const fdm::stl::string& packet, const nlohmann::json& data, const glm::vec4& origin)
{
    for (int x = -1; x <= 1; ++x)
        for (int z = -1; z <= 1; ++z)
            for (int w = -1; w <= 1; ++w)
            {
                Chunk* chunk = world->getChunkFromCoords(origin.x + 8 * x, origin.z + 8 * z, origin.w + 8 * w);

                if (!chunk) continue;

                for (auto& entity : chunk->entities)
                {
                    if (entity->getName() != "Player") continue;
                    if (!world->entityPlayerIDs.contains(entity->id)) continue;

                    uint32_t handle = world->entityPlayerIDs.at(entity->id)->handle;

                    JSONData::sendPacketClient(world, packet, data, handle);
                }
            }
}

void onHealthChanged(WorldServer* server, Entity* entity) {
    nlohmann::json healthData;

    healthData["entityId"] = stl::uuid::to_string(entity->id);
    healthData["health"] = getEntityHealth(entity);
    

    sendPacketInRegion(server, "zihed.healthsync.entityHealthSync", healthData, entity->getPos());
}

// Update health on client

void setEntityHealth(Entity* entity, float health) {


    if (entity->getName()=="Spider") ((EntitySpider*)entity)->health = health;
    else if (entity->getName() == "Butterfly")((EntityButterfly*)entity)->health = health;
    else if (entity->getName() == "Player") {
        if (StateGame::instanceObj.player.EntityPlayerID == entity->id)
            StateGame::instanceObj.player.health = health;
        ((EntityPlayer*)entity)->player->health = health;
    }

}

void onEntityHealthSync(fdm::WorldClient* world, fdm::Player* player, const nlohmann::json& data) {
    auto entityId = stl::uuid()(data["entityId"].get<std::string>());
    float newHealth = data["health"].get<float>();
    fdm::Entity* entity = world->getEntity(entityId);
    if (entity)
        setEntityHealth(entity, newHealth);
}

// Initialise stuff

$hook(void, StateIntro, init, StateManager& s)
{
    original(self, s);
    // initialize opengl stuff
    glewExperimental = true;
    glewInit();
    glfwInit();

    JSONData::SCaddPacketCallback("zihed.healthsync.entityHealthSync", onEntityHealthSync);
}

// Send packets when stuff happens

$hook(void, Player, update, World* world, double dt, EntityPlayer* entityPlayer)
{
    original(self, world, dt, entityPlayer);

    if (world->getType() != World::TYPE_SERVER) return;

    WorldServer* server = (WorldServer*)(world);

    static int lastUpdateHealth = 100;
    int thisUpdateHealth = self->health;
	
    if (thisUpdateHealth != lastUpdateHealth) {
        onHealthChanged(server, entityPlayer);
    }
    lastUpdateHealth = thisUpdateHealth;
	
}

$hook(void, EntitySpider, takeDamage, float damage, World* world) {
    original(self, damage, world);

    if (world->getType() != World::TYPE_SERVER || damage <= 0) return;

    WorldServer* server = (WorldServer*)(world);

    onHealthChanged(server, self);
}

$hook(void, EntityButterfly, takeDamage, float damage, World* world) {
    original(self, damage, world);

    if (world->getType() != World::TYPE_SERVER || damage <= 0) return;

    WorldServer* server = (WorldServer*)(world);

    onHealthChanged(server, self);
}
