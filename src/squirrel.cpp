#include "../include/squirrel.h"

#include "../include/druid.h"
#include "../include/ork.h"

#include <iostream>
#include <mutex>

Squirrel::Squirrel(const std::string &name, int x, int y) : NPC(SquirrelType, name, x, y) {}
Squirrel::Squirrel(std::istream &is) : NPC(SquirrelType, is) {}

bool Squirrel::accept(const std::shared_ptr<NPC> &attacker)
{
    if (!is_alive())
        return false;
    return attacker->fight(std::dynamic_pointer_cast<Squirrel>(shared_from_this()));
}

bool Squirrel::fight(const std::shared_ptr<Ork> &other)
{
    fight_notify(other, false);
    return false;
}

bool Squirrel::fight(const std::shared_ptr<Squirrel> &other)
{
    fight_notify(other, false);
    return false;
}

bool Squirrel::fight(const std::shared_ptr<Druid> &other)
{
    fight_notify(other, false);
    return false;
}

void Squirrel::print() const
{
    std::cout << "Squirrel " << name << " (" << get_x() << ", " << get_y() << ")" << std::endl;
}

void Squirrel::save(std::ostream &os) const
{
    NPC::save(os);
}
