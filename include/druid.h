#pragma once

#include "npc.h"

class Druid : public NPC
{
public:
    Druid(const std::string &name, int x, int y);
    explicit Druid(std::istream &is);


    bool accept(const std::shared_ptr<NPC> &attacker) override;

    bool fight(const std::shared_ptr<Ork> &other) override;
    bool fight(const std::shared_ptr<Squirrel> &other) override;
    bool fight(const std::shared_ptr<Druid> &other) override;

    void print() const override;
    void save(std::ostream &os) const override;
};
