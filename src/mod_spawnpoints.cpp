#include "MapMgr.h"
#include "Player.h"
#include "ScriptMgr.h"

struct SpawnPoints
{
    uint32 MapId;
    float X;
    float Y;
    float Z;
    float O;
};

std::vector<SpawnPoints> spawnPoints;

class SpawnPointsPlayer : public PlayerScript
{
public:
    SpawnPointsPlayer() : PlayerScript("SpawnPointsPlayer") {}

    void OnCreate(Player* player) override
    {
        if (player->getClass() == CLASS_DEATH_KNIGHT)
            return;

        uint32 mapId = spawnPoints[player->GetTeamId()].MapId;
        float x = spawnPoints[player->GetTeamId()].X;
        float y = spawnPoints[player->GetTeamId()].Y;
        float z = spawnPoints[player->GetTeamId()].Z;
        float orientation = spawnPoints[player->GetTeamId()].O;
        const WorldLocation& location = WorldLocation(mapId, x, y, z, orientation);

        player->Relocate(&location);
        player->ResetMap();
        player->SetMap(sMapMgr->CreateMap(mapId, player));
        player->SaveToDB(false, false);
    }
};

class SpawnPointWorld : public WorldScript
{
public:
    SpawnPointWorld() : WorldScript("SpawnPointWorld") { }

    void OnStartup() override
    {
        LOG_INFO("server.loading", "Loading spawn points...");
        {
            QueryResult result = WorldDatabase.Query("SELECT `team_id`, `map_id`, `pos_x`, `pos_y`, `pos_z`, `orientation` FROM `mod_spawnpoints`");

            if (!result)
            {
                LOG_INFO("server.loading", ">> Loaded 0 spawn points. DB table `mod_spawnpoints` is empty.");
                return;
            }

            int i = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32 TeamId = fields[0].Get<uint32>();

                spawnPoints.push_back(SpawnPoints());
                spawnPoints[TeamId].MapId = fields[1].Get<uint32>();
                spawnPoints[TeamId].X = fields[2].Get<float>();
                spawnPoints[TeamId].Y = fields[3].Get<float>();
                spawnPoints[TeamId].Z = fields[4].Get<float>();
                spawnPoints[TeamId].O = fields[5].Get<float>();

                i++;
            } while (result->NextRow());

            LOG_INFO("server.loading", ">> Loaded {} spawn points", i);
        }
    }
};

void AddSpawnPointsScripts()
{
    new SpawnPointsPlayer();
    new SpawnPointWorld();
}
