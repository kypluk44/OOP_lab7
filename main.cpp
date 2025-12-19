#include "battle.h"
#include "observers.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace
{
    constexpr int MAP_WIDTH = 100;
    constexpr int MAP_HEIGHT = 100;
    constexpr int GRID_SIZE = 20;
    constexpr int INITIAL_NPCS = 50;
    constexpr auto MOVE_TICK = 10ms;
    constexpr auto PRINT_TICK = 1s;
    constexpr auto GAME_DURATION = 30s;

    struct MoveRule
    {
        int step;
        int kill_distance;
    };

    MoveRule rules_for(NpcType type)
    {
        switch (type)
        {
        case OrkType:
            return {20, 10};
        case SquirrelType:
            return {5, 5};
        case DruidType:
            return {10, 10};
        default:
            return {1, 1};
        }
    }

    bool can_attack(NpcType attacker, NpcType defender)
    {
        return (attacker == OrkType && defender == DruidType) ||
               (attacker == DruidType && defender == SquirrelType);
    }

    char marker(NpcType type)
    {
        switch (type)
        {
        case OrkType:
            return 'O';
        case SquirrelType:
            return 'S';
        case DruidType:
            return 'D';
        default:
            return '?';
        }
    }

    struct FightEvent
    {
        std::shared_ptr<NPC> attacker;
        std::shared_ptr<NPC> defender;
    };

    class FightQueue
    {
    public:
        void push(FightEvent event)
        {
            std::lock_guard<std::mutex> lck(mtx);
            if (stopped)
                return;
            events.push(std::move(event));
            cv.notify_one();
        }

        bool pop(FightEvent &event)
        {
            std::unique_lock<std::mutex> lck(mtx);
            cv.wait(lck, [&]()
                    { return stopped || !events.empty(); });
            if (events.empty())
                return false;
            event = events.front();
            events.pop();
            return true;
        }

        void request_stop()
        {
            std::lock_guard<std::mutex> lck(mtx);
            stopped = true;
            cv.notify_all();
        }

    private:
        std::queue<FightEvent> events;
        std::mutex mtx;
        std::condition_variable cv;
        bool stopped{false};
    };

    std::pair<int, int> random_shift(NpcType type, std::mt19937 &rng)
    {
        const auto rule = rules_for(type);
        std::uniform_int_distribution<int> dist(-rule.step, rule.step);
        return {dist(rng), dist(rng)};
    }

    void print_map(const set_t &npcs)
    {
        std::array<char, GRID_SIZE * GRID_SIZE> cells{};
        cells.fill(' ');
        const int cell_w = MAP_WIDTH / GRID_SIZE;
        const int cell_h = MAP_HEIGHT / GRID_SIZE;

        for (const auto &npc : npcs)
        {
            if (!npc || !npc->is_alive())
                continue;
            const auto [x, y] = npc->position();
            const int i = std::clamp(x / cell_w, 0, GRID_SIZE - 1);
            const int j = std::clamp(y / cell_h, 0, GRID_SIZE - 1);
            cells[j * GRID_SIZE + i] = marker(npc->get_type());
        }

        std::lock_guard<std::mutex> lck(console_mutex());
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            for (int i = 0; i < GRID_SIZE; ++i)
            {
                const char c = cells[j * GRID_SIZE + i];
                if (c == ' ')
                    std::cout << "[ ]";
                else
                    std::cout << '[' << c << ']';
            }
            std::cout << '\n';
        }
        std::cout << std::string(GRID_SIZE * 3, '=') << '\n';
    }

    void print_survivors(const set_t &npcs)
    {
        std::lock_guard<std::mutex> lck(console_mutex());
        std::cout << "Survivors:\n";
        for (const auto &npc : npcs)
        {
            if (npc && npc->is_alive())
                std::cout << *npc << '\n';
        }
    }
}

int main()
{
    seed_random(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));

    auto console_observer = std::make_shared<ConsoleObserver>();
    auto file_observer = std::make_shared<FileObserver>("log.txt");
    std::vector<std::shared_ptr<IFightObserver>> observers{console_observer, file_observer};

    set_t npcs;
    std::mt19937 seed_rng{std::random_device{}()};
    std::uniform_int_distribution<int> type_dist(1, 3);
    std::uniform_int_distribution<int> x_dist(0, MAP_WIDTH - 1);
    std::uniform_int_distribution<int> y_dist(0, MAP_HEIGHT - 1);

    for (int i = 0; i < INITIAL_NPCS; ++i)
    {
        const auto type = static_cast<NpcType>(type_dist(seed_rng));
        const std::string name = "npc_" + std::to_string(i);
        auto npc = factory(type, name, x_dist(seed_rng), y_dist(seed_rng), observers);
        if (npc)
            npcs.insert(npc);
    }

    FightQueue fight_queue;
    std::atomic<bool> stop{false};

    std::thread fight_thread([&]()
                             {
        FightEvent event;
        while (fight_queue.pop(event))
        {
            if (!event.attacker || !event.defender)
                continue;
            if (!event.attacker->is_alive() || !event.defender->is_alive())
                continue;
            if (!can_attack(event.attacker->get_type(), event.defender->get_type()))
                continue;

            const int attack = roll_dice();
            const int defense = roll_dice();
            if (attack > defense)
            {
                event.attacker->fight_notify(event.defender, true);
                event.defender->die();
            }
            else
            {
                event.attacker->fight_notify(event.defender, false);
            }
        } });

    std::thread move_thread([&]()
                            {
        std::mt19937 rng{std::random_device{}()};
        while (!stop.load())
        {
            for (const auto &npc : npcs)
            {
                if (!npc || !npc->is_alive())
                    continue;
                const auto shift = random_shift(npc->get_type(), rng);
                npc->move(shift.first, shift.second, MAP_WIDTH, MAP_HEIGHT);
            }

            for (const auto &attacker : npcs)
            {
                if (!attacker || !attacker->is_alive())
                    continue;
                const auto kill_distance = rules_for(attacker->get_type()).kill_distance;

                for (const auto &defender : npcs)
                {
                    if (attacker == defender || !defender || !defender->is_alive())
                        continue;
                    if (!can_attack(attacker->get_type(), defender->get_type()))
                        continue;

                    if (attacker->is_close(defender, static_cast<size_t>(kill_distance)))
                        fight_queue.push({attacker, defender});
                }
            }

            std::this_thread::sleep_for(MOVE_TICK);
        }
        fight_queue.request_stop();
    });

    const auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < GAME_DURATION)
    {
        print_map(npcs);
        std::this_thread::sleep_for(PRINT_TICK);
    }

    stop = true;
    fight_queue.request_stop();
    move_thread.join();
    fight_thread.join();

    print_survivors(npcs);
    return 0;
}
