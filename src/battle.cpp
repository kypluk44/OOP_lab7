#include "../include/battle.h"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>

void print_all(const set_t &array, std::ostream &os)
{
    if (array.empty())
    {
        os << "No NPCs\n";
        return;
    }
    for (auto &npc : array)
        os << *npc << '\n';
}

void save(const set_t &array, const std::string &filename)
{
    std::ofstream fs(filename);
    fs << array.size() << std::endl;
    for (auto &n : array)
        n->save(fs);
    fs.flush();
    fs.close();
}

set_t load(const std::string &filename, const std::vector<std::shared_ptr<IFightObserver>> &observers)
{
    set_t result;
    std::ifstream is(filename);
    if (is.good() && is.is_open())
    {
        int count;
        is >> count;
        for (int i = 0; i < count; ++i)
            result.insert(factory(is, observers));
        is.close();
    }
    else
        std::cerr << "Error: " << std::strerror(errno) << std::endl;
    return result;
}

std::ostream &operator<<(std::ostream &os, const set_t &array)
{
    for (auto &n : array)
        n->print();
    return os;
}

set_t fight(const set_t &array, size_t distance)
{
    set_t dead_list;

    for (const auto &attacker : array)
    {
        if (!attacker || !attacker->is_alive())
            continue;
        if (dead_list.count(attacker))
            continue;
        for (const auto &defender : array)
        {
            if (attacker == defender || dead_list.count(defender) || !defender || !defender->is_alive())
                continue;
            if (!attacker->is_close(defender, distance))
                continue;

            const bool defender_dead = defender->accept(attacker);
            const bool attacker_dead = attacker->accept(defender);

            if (defender_dead)
            {
                defender->die();
                dead_list.insert(defender);
            }
            if (attacker_dead)
            {
                attacker->die();
                dead_list.insert(attacker);
                break;
            }
        }
    }

    return dead_list;
}

bool name_exists(const set_t &array, const std::string &name)
{
    for (const auto &npc : array)
    {
        if (npc && npc->get_name() == name)
            return true;
    }
    return false;
}
