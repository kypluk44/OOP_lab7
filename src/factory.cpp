#include "../include/factory.h"

#include "../include/druid.h"
#include "../include/ork.h"
#include "../include/squirrel.h"

#include <iostream>
#include <istream>

namespace
{
    void attach_observers(const std::shared_ptr<NPC> &npc,
                          const std::vector<std::shared_ptr<IFightObserver>> &observers)
    {
        if (!npc)
            return;
        for (auto &observer : observers)
            npc->subscribe(observer);
    }
}

std::shared_ptr<NPC> factory(NpcType type,
                             const std::string &name,
                             int x,
                             int y,
                             const std::vector<std::shared_ptr<IFightObserver>> &observers)
{
    std::shared_ptr<NPC> result;
    switch (type)
    {
    case OrkType:
        result = std::make_shared<Ork>(name, x, y);
        break;
    case SquirrelType:
        result = std::make_shared<Squirrel>(name, x, y);
        break;
    case DruidType:
        result = std::make_shared<Druid>(name, x, y);
        break;
    default:
        break;
    }

    attach_observers(result, observers);
    return result;
}

std::shared_ptr<NPC> factory(std::istream &is,
                             const std::vector<std::shared_ptr<IFightObserver>> &observers)
{
    int type_value{0};
    if (!(is >> type_value))
        return nullptr;

    std::shared_ptr<NPC> result;
    switch (static_cast<NpcType>(type_value))
    {
    case OrkType:
        result = std::make_shared<Ork>(is);
        break;
    case SquirrelType:
        result = std::make_shared<Squirrel>(is);
        break;
    case DruidType:
        result = std::make_shared<Druid>(is);
        break;
    default:
        std::cerr << "unexpected NPC type:" << type_value << std::endl;
        return nullptr;
    }

    attach_observers(result, observers);
    return result;
}
