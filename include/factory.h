#pragma once

#include "npc.h"

#include <memory>
#include <vector>

std::shared_ptr<NPC> factory(NpcType type,
                             const std::string &name,
                             int x,
                             int y,
                             const std::vector<std::shared_ptr<IFightObserver>> &observers);

std::shared_ptr<NPC> factory(std::istream &is,
                             const std::vector<std::shared_ptr<IFightObserver>> &observers);
