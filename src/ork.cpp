#include "../include/ork.h"

#include "../include/druid.h"
#include "../include/squirrel.h"

#include <iostream>
#include <mutex>

Ork::Ork(const std::string &name, int x, int y) : NPC(OrkType, name, x, y) {}
Ork::Ork(std::istream &is) : NPC(OrkType, is) {}

bool Ork::accept(const std::shared_ptr<NPC> &attacker)
{
    if (!is_alive())
        return false;
    return attacker->fight(std::dynamic_pointer_cast<Ork>(shared_from_this()));
}

bool Ork::fight(const std::shared_ptr<Ork> &other)
{
    fight_notify(other, false);
    return false;
}

bool Ork::fight(const std::shared_ptr<Squirrel> &other)
{
    fight_notify(other, false);
    return false;
}

bool Ork::fight(const std::shared_ptr<Druid> &other)
{
    const int attack = roll_dice();
    const int defense = roll_dice();
    const bool win = attack > defense;
    fight_notify(other, win);
    return win;
}

void Ork::print() const
{
    std::cout << "Ork " << name << " (" << get_x() << ", " << get_y() << ")" << std::endl;
}

void Ork::save(std::ostream &os) const
{
    NPC::save(os);
}
