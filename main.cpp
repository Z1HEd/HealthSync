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
    if (entity->getName() == "Player" && ((EntityPlayer*)entity)->ownedPlayer != nullptr) return ((EntityPlayer*)entity)->ownedPlayer->health;
    Console::printLine("Unknown entity or empty EntityPlayer::ownedPlayer in getHealth ");
    return 0;
}

void onHealthChanged(WorldServer* server, Entity* entity) {
    nlohmann::json healthData;

    healthData["entityId"] = stl::uuid::to_string(entity->id);
    healthData["health"] = getEntityHealth(entity);

    JSONData::broadcastPacket(server, "entityHealthSync", healthData);
}

void onHealthChanged(WorldServer* server, Player* player) {
    nlohmann::json healthData;

    healthData["entityId"] = stl::uuid::to_string(player->EntityPlayerID);
    healthData["health"] = player->health;

    JSONData::broadcastPacket(server, "entityHealthSync", healthData);
}

// Update health on client

void setEntityHealth(Entity* entity, float health) {
    if (entity->getName()=="Spider") ((EntitySpider*)entity)->health = health;
    else if (entity->getName() == "Butterfly")((EntityButterfly*)entity)->health = health;
    else if (entity->getName() == "Player" && ((EntityPlayer*)entity)->ownedPlayer!=nullptr) ((EntityPlayer*)entity)->ownedPlayer->health = health;
    else Console::printLine("Unknown entity or empty EntityPlayer::ownedPlayer in setHealth ");
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

    Console::printLine("Initializing HealthSync mod");

    // initialize opengl stuff
    glewExperimental = true;
    glewInit();
    glfwInit();

    JSONData::SCaddPacketCallback("entityHealthSync", onEntityHealthSync);
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
        Console::printLine("PlayerHealthChanged! ");
        onHealthChanged(server, self);
    }
    lastUpdateHealth = thisUpdateHealth;
	
}

$hook(void, EntitySpider, takeDamage, float damage, World* world) {
    original(self, damage, world);

    if (world->getType() != World::TYPE_SERVER || damage <= 0) return;

    WorldServer* server = (WorldServer*)(world);


    Console::printLine("SpiderHealthChanged! ");
    onHealthChanged(server, self);
}

$hook(void, EntityButterfly, takeDamage, float damage, World* world) {
    original(self, damage, world);

    if (world->getType() != World::TYPE_SERVER || damage <= 0) return;

    WorldServer* server = (WorldServer*)(world);

    

    Console::printLine("ButterflyHealthChanged! ");
    onHealthChanged(server, self);
}
