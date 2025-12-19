#include "../include/npc.h"

#include <algorithm>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <utility>

NPC::NPC(NpcType t, std::string nm, int x_pos, int y_pos) // конструктор с параметрами
    : name(std::move(nm)), type(t), x(x_pos), y(y_pos)
{
}

NPC::NPC(NpcType t, std::istream &is) : type(t) // конструктор из файла
{
    is >> name;
    is >> x;
    is >> y;
}

void NPC::subscribe(const std::shared_ptr<IFightObserver> &observer)
{
    observers.push_back(observer);
}

int NPC::get_x() const
{
    std::shared_lock<std::shared_mutex> lck(state_mutex);
    return x;
}

int NPC::get_y() const
{
    std::shared_lock<std::shared_mutex> lck(state_mutex);
    return y;
}

std::pair<int, int> NPC::position() const
{
    std::shared_lock<std::shared_mutex> lck(state_mutex);
    return {x, y};
}

void NPC::fight_notify(const std::shared_ptr<NPC> &defender, bool win)
{
    for (auto &o : observers)
        o->on_fight(shared_from_this(), defender, win);
}

bool NPC::is_close(const std::shared_ptr<NPC> &other, size_t distance) const
{
    if (!other)
        return false;

    std::shared_lock<std::shared_mutex> lck(state_mutex);
    const int self_x = x;
    const int self_y = y;
    const bool self_alive = alive;

    if (!self_alive || !other->is_alive())
        return false;

    const auto [other_x, other_y] = other->position();
    const auto dx = self_x - other_x;
    const auto dy = self_y - other_y;
    return static_cast<size_t>(dx * dx + dy * dy) <= distance * distance;
}

void NPC::move(int shift_x, int shift_y, int max_x, int max_y)
{
    std::lock_guard<std::shared_mutex> lck(state_mutex);
    if (!alive)
        return;
    x = std::clamp(x + shift_x, 0, max_x);
    y = std::clamp(y + shift_y, 0, max_y);
}

bool NPC::is_alive() const
{
    std::shared_lock<std::shared_mutex> lck(state_mutex);
    return alive;
}

void NPC::die()
{
    std::lock_guard<std::shared_mutex> lck(state_mutex);
    alive = false;
}

void NPC::save(std::ostream &os) const
{
    std::shared_lock<std::shared_mutex> lck(state_mutex);
    os << static_cast<int>(type) << ' ' << name << ' ' << x << ' ' << y << '\n';
}

namespace
{
    std::string type_to_string(NpcType t)
    {
        switch (t)
        {
        case OrkType:
            return "Ork";
        case SquirrelType:
            return "Squirrel";
        case DruidType:
            return "Druid";
        default:
            return "Unknown";
        }
    }
}

std::ostream &operator<<(std::ostream &os, NPC &npc)
{
    os << type_to_string(npc.type) << ' ' << npc.name << " {" << npc.get_x() << ", " << npc.get_y() << "}";
    return os;
}

namespace
{
    thread_local std::mt19937 rng{std::random_device{}()};
    thread_local std::uniform_int_distribution<int> dice_dist(1, 6);
}

int roll_dice()
{
    return dice_dist(rng);
}

void seed_random(unsigned int seed)
{
    rng.seed(seed);
}

std::mutex &console_mutex()
{
    static std::mutex mtx;
    return mtx;
}
