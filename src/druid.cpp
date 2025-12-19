#include "../include/druid.h"

#include "../include/ork.h"
#include "../include/squirrel.h"

#include <iostream>
#include <mutex>

Druid::Druid(const std::string &name, int x, int y) : NPC(DruidType, name, x, y) {}
Druid::Druid(std::istream &is) : NPC(DruidType, is) {}

bool Druid::accept(const std::shared_ptr<NPC> &attacker)
{
    if (!is_alive())
        return false;
    return attacker->fight(std::dynamic_pointer_cast<Druid>(shared_from_this()));
}

bool Druid::fight(const std::shared_ptr<Ork> &other)
{
    fight_notify(other, false);
    return false;
}

bool Druid::fight(const std::shared_ptr<Squirrel> &other)
{
    const int attack = roll_dice();
    const int defense = roll_dice();
    const bool win = attack > defense;
    fight_notify(other, win);
    return win;
}

bool Druid::fight(const std::shared_ptr<Druid> &other)
{
    fight_notify(other, false);
    return false;
}

void Druid::print() const
{
    std::cout << "Druid " << name << " (" << get_x() << ", " << get_y() << ")" << std::endl;
}

void Druid::save(std::ostream &os) const
{
    NPC::save(os);
}
