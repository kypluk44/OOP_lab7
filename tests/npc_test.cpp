#include "../include/battle.h"
#include "../include/druid.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <sstream>

class CounterObserver : public IFightObserver
{
public:
    void on_fight(const std::shared_ptr<NPC> attacker,
                  const std::shared_ptr<NPC> defender,
                  bool win) override
    {
        if (win)
        {
            ++count;
            last_attacker = attacker;
            last_defender = defender;
        }
    }

    size_t count{0};
    std::shared_ptr<NPC> last_attacker;
    std::shared_ptr<NPC> last_defender;
};

class LoggingObserver : public IFightObserver
{
public:
    void on_fight(const std::shared_ptr<NPC> attacker,
                  const std::shared_ptr<NPC> defender,
                  bool win) override
    {
        if (win && attacker && defender)
            logs.push_back(attacker->get_name() + "->" + defender->get_name());
    }

    std::vector<std::string> logs;
};

TEST(FightRules, OrkKillsDruid)
{
    seed_random(1);
    auto observer = std::make_shared<CounterObserver>();
    std::vector<std::shared_ptr<IFightObserver>> observers{observer};

    set_t npcs;
    npcs.insert(factory(OrkType, "ork1", 0, 0, observers));
    npcs.insert(factory(DruidType, "druid1", 1, 1, observers));

    auto dead = fight(npcs, 5);
    for (auto &d : dead)
        npcs.erase(d);

    ASSERT_EQ(npcs.size(), 1u);
    EXPECT_EQ(observer->count, 1u);
}

TEST(FightRules, DruidKillsSquirrelOnly)
{
    seed_random(1);
    auto observer = std::make_shared<CounterObserver>();
    std::vector<std::shared_ptr<IFightObserver>> observers{observer};

    set_t npcs;
    npcs.insert(factory(DruidType, "druid2", 0, 0, observers));
    npcs.insert(factory(SquirrelType, "squirrel1", 0, 1, observers));
    npcs.insert(factory(OrkType, "ork2", 100, 100, observers));

    auto dead = fight(npcs, 5);
    for (auto &d : dead)
        npcs.erase(d);

    ASSERT_EQ(npcs.size(), 2u);
    EXPECT_EQ(observer->count, 1u);
}

TEST(Storage, SaveAndLoad)
{
    std::vector<std::shared_ptr<IFightObserver>> observers;
    set_t npcs;
    npcs.insert(factory(OrkType, "ork3", 10, 10, observers));
    npcs.insert(factory(SquirrelType, "squirrel2", 20, 20, observers));

    const std::string filename = "npc_test_data.txt";
    save(npcs, filename);

    auto loaded = load(filename, observers);

    ASSERT_EQ(loaded.size(), 2u);

    std::filesystem::remove(filename);
}

TEST(DistanceBoundary, KillAtEdge)
{
    seed_random(1);
    auto observer = std::make_shared<CounterObserver>();
    std::vector<std::shared_ptr<IFightObserver>> observers{observer};

    set_t npcs;
    npcs.insert(factory(OrkType, "ork_edge", 0, 0, observers));
    npcs.insert(factory(DruidType, "druid_edge", 6, 8, observers)); // расстояние 10

    auto dead = fight(npcs, 10);

    ASSERT_EQ(dead.size(), 1u);
    EXPECT_EQ(observer->count, 1u);
}

TEST(Creation, CreateAllTypes)
{
    std::vector<std::shared_ptr<IFightObserver>> observers;
    auto ork = factory(OrkType, "ork_new", 5, 6, observers);
    auto squirrel = factory(SquirrelType, "sq_new", 7, 8, observers);
    auto druid = factory(DruidType, "dr_new", 9, 10, observers);

    ASSERT_TRUE(ork);
    ASSERT_TRUE(squirrel);
    ASSERT_TRUE(druid);

    EXPECT_EQ(ork->get_type(), OrkType);
    EXPECT_EQ(ork->get_name(), "ork_new");
    EXPECT_EQ(ork->get_x(), 5);
    EXPECT_EQ(ork->get_y(), 6);

    EXPECT_EQ(squirrel->get_type(), SquirrelType);
    EXPECT_EQ(druid->get_type(), DruidType);
}

TEST(DistanceCheck, IsCloseUsesPythagoras)
{
    std::vector<std::shared_ptr<IFightObserver>> observers;
    auto ork = factory(OrkType, "ork_dist", 0, 0, observers);
    auto druid = factory(DruidType, "dr_dist", 3, 4, observers); // расстояние 5

    EXPECT_TRUE(ork->is_close(druid, 5));
    EXPECT_FALSE(ork->is_close(druid, 4));
}

TEST(Observer, NotifiedOnKill)
{
    seed_random(1);
    auto logger = std::make_shared<LoggingObserver>();
    std::vector<std::shared_ptr<IFightObserver>> observers{logger};
    auto ork = factory(OrkType, "ork_obs", 0, 0, observers);
    auto druid = factory(DruidType, "dr_obs", 0, 0, observers);

    auto dr = std::dynamic_pointer_cast<Druid>(druid);
    ork->fight(dr);

    ASSERT_EQ(logger->logs.size(), 1u);
    EXPECT_EQ(logger->logs.front(), "ork_obs->dr_obs");
}

TEST(Observer, LosingSideDoesNotLog)
{
    auto logger = std::make_shared<LoggingObserver>();
    std::vector<std::shared_ptr<IFightObserver>> squirrel_observers{logger};
    std::vector<std::shared_ptr<IFightObserver>> none;

    auto squirrel = factory(SquirrelType, "sq_obs", 0, 0, squirrel_observers);
    auto druid = factory(DruidType, "dr_no_log", 0, 0, none);

    auto dr = std::dynamic_pointer_cast<Druid>(druid);
    squirrel->fight(dr); // Белка проигрывает, win == false

    EXPECT_TRUE(logger->logs.empty());
}

TEST(Storage, SaveLoadPreservesData)
{
    std::vector<std::shared_ptr<IFightObserver>> observers;
    set_t npcs;
    npcs.insert(factory(OrkType, "save_ork", 1, 2, observers));
    npcs.insert(factory(SquirrelType, "save_sq", 3, 4, observers));
    npcs.insert(factory(DruidType, "save_dr", 5, 6, observers));

    const std::string filename = "npc_temp_save.txt";
    save(npcs, filename);

    auto loaded = load(filename, observers);
    std::filesystem::remove(filename);

    ASSERT_EQ(loaded.size(), 3u);

    size_t found = 0;
    for (auto &npc : loaded)
    {
        if (npc->get_name() == "save_ork")
        {
            ++found;
            EXPECT_EQ(npc->get_type(), OrkType);
            EXPECT_EQ(npc->get_x(), 1);
            EXPECT_EQ(npc->get_y(), 2);
        }
        if (npc->get_name() == "save_sq")
        {
            ++found;
            EXPECT_EQ(npc->get_type(), SquirrelType);
        }
        if (npc->get_name() == "save_dr")
        {
            ++found;
            EXPECT_EQ(npc->get_type(), DruidType);
        }
    }
    EXPECT_EQ(found, 3u);
}

TEST(Printing, PrintAllCapturesOutput)
{
    std::vector<std::shared_ptr<IFightObserver>> observers;
    set_t npcs;
    npcs.insert(factory(OrkType, "print_ork", 10, 10, observers));
    npcs.insert(factory(SquirrelType, "print_sq", 20, 20, observers));

    std::ostringstream ss;
    print_all(npcs, ss);

    const auto output = ss.str();
    EXPECT_NE(output.find("print_ork"), std::string::npos);
    EXPECT_NE(output.find("print_sq"), std::string::npos);
}

TEST(Fight, NoKillIfTooFar)
{
    auto observer = std::make_shared<CounterObserver>();
    std::vector<std::shared_ptr<IFightObserver>> observers{observer};

    set_t npcs;
    npcs.insert(factory(OrkType, "far_ork", 0, 0, observers));
    npcs.insert(factory(DruidType, "far_dr", 100, 100, observers));

    auto dead = fight(npcs, 10);

    EXPECT_TRUE(dead.empty());
    EXPECT_EQ(observer->count, 0u);
}

TEST(Fight, MultipleKillsByDruid)
{
    seed_random(1);
    auto observer = std::make_shared<CounterObserver>();
    std::vector<std::shared_ptr<IFightObserver>> observers{observer};

    set_t npcs;
    npcs.insert(factory(DruidType, "dr_multi2", 0, 0, observers));
    npcs.insert(factory(SquirrelType, "sq_m1", 1, 1, observers));
    npcs.insert(factory(SquirrelType, "sq_m2", 2, 2, observers));
    npcs.insert(factory(OrkType, "ork_far", 50, 50, observers));

    auto dead = fight(npcs, 5);

    EXPECT_EQ(dead.size(), 2u);
    EXPECT_EQ(observer->count, 2u);
}

TEST(Factory, CreateFromStream)
{
    std::vector<std::shared_ptr<IFightObserver>> observers;
    std::istringstream input("1 ork_stream 11 22\n");
    auto npc = factory(input, observers);

    ASSERT_TRUE(npc);
    EXPECT_EQ(npc->get_type(), OrkType);
    EXPECT_EQ(npc->get_name(), "ork_stream");
    EXPECT_EQ(npc->get_x(), 11);
    EXPECT_EQ(npc->get_y(), 22);
}

TEST(DistanceBoundary, NoKillBelowEdge)
{
    auto observer = std::make_shared<CounterObserver>();
    std::vector<std::shared_ptr<IFightObserver>> observers{observer};

    set_t npcs;
    npcs.insert(factory(OrkType, "ork_close", 0, 0, observers));
    npcs.insert(factory(DruidType, "druid_close", 6, 8, observers)); // расстояние 10

    auto dead = fight(npcs, 9);

    ASSERT_EQ(dead.size(), 0u);
    EXPECT_EQ(observer->count, 0u);
}

TEST(ObserverCountsMultipleKills, DruidVsTwoSquirrels)
{
    seed_random(1);
    auto observer = std::make_shared<CounterObserver>();
    std::vector<std::shared_ptr<IFightObserver>> observers{observer};

    set_t npcs;
    npcs.insert(factory(DruidType, "druid_multi", 100, 100, observers));
    npcs.insert(factory(SquirrelType, "sq1", 102, 100, observers));
    npcs.insert(factory(SquirrelType, "sq2", 103, 101, observers));

    auto dead = fight(npcs, 5);

    ASSERT_EQ(dead.size(), 2u);
    EXPECT_EQ(observer->count, 2u);
}

TEST(ZeroDistanceFight, KillOnSameCell)
{
    seed_random(1);
    auto observer = std::make_shared<CounterObserver>();
    std::vector<std::shared_ptr<IFightObserver>> observers{observer};

    set_t npcs;
    npcs.insert(factory(OrkType, "ork_same", 200, 200, observers));
    npcs.insert(factory(DruidType, "druid_same", 200, 200, observers));

    auto dead = fight(npcs, 0);

    ASSERT_EQ(dead.size(), 1u);
    EXPECT_EQ(observer->count, 1u);
}

TEST(Movement, ClampsToBounds)
{
    std::vector<std::shared_ptr<IFightObserver>> observers;
    auto ork = factory(OrkType, "ork_move", 0, 0, observers);

    ork->move(-10, -20, 100, 100);
    EXPECT_EQ(ork->get_x(), 0);
    EXPECT_EQ(ork->get_y(), 0);

    ork->move(150, 200, 100, 100);
    EXPECT_EQ(ork->get_x(), 100);
    EXPECT_EQ(ork->get_y(), 100);
}

TEST(Movement, DeadNpcDoesNotMove)
{
    std::vector<std::shared_ptr<IFightObserver>> observers;
    auto squirrel = factory(SquirrelType, "sq_dead", 5, 5, observers);
    squirrel->die();
    squirrel->move(10, 10, 100, 100);
    EXPECT_EQ(squirrel->get_x(), 5);
    EXPECT_EQ(squirrel->get_y(), 5);
}

TEST(EdgeCases, EmptyNameAllowed)
{
    std::vector<std::shared_ptr<IFightObserver>> observers;
    auto druid = factory(DruidType, "", 1, 2, observers);
    ASSERT_TRUE(druid);
    EXPECT_TRUE(druid->get_name().empty());
}

TEST(EdgeCases, NegativeCoordinatesPersist)
{
    std::vector<std::shared_ptr<IFightObserver>> observers;
    auto ork = factory(OrkType, "neg", -5, -10, observers);
    ASSERT_TRUE(ork);
    EXPECT_EQ(ork->get_x(), -5);
    EXPECT_EQ(ork->get_y(), -10);
}

TEST(Distance, ZeroDistanceFalseWhenApart)
{
    std::vector<std::shared_ptr<IFightObserver>> observers;
    auto ork = factory(OrkType, "o", 0, 0, observers);
    auto druid = factory(DruidType, "d", 1, 1, observers);
    EXPECT_FALSE(ork->is_close(druid, 0));

    auto squirrel = factory(SquirrelType, "s", 0, 0, observers);
    EXPECT_TRUE(ork->is_close(squirrel, 0));
}

TEST(Fight, SquirrelCannotKillOrk)
{
    auto observer = std::make_shared<CounterObserver>();
    std::vector<std::shared_ptr<IFightObserver>> observers{observer};

    set_t npcs;
    npcs.insert(factory(SquirrelType, "sq", 0, 0, observers));
    npcs.insert(factory(OrkType, "ork", 1, 1, observers));

    auto dead = fight(npcs, 5);
    EXPECT_TRUE(dead.empty());
    EXPECT_EQ(observer->count, 0u);
}
