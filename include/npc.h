#pragma once

#include <cmath>
#include <cstddef>
#include <iostream>
#include <istream>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <utility>
#include <vector>

struct Ork;
struct Squirrel;
struct Druid;

using set_t = std::set<std::shared_ptr<class NPC>>;

enum NpcType
{
    Unknown = 0,
    OrkType = 1,
    SquirrelType = 2,
    DruidType = 3
};

struct IFightObserver
{
    virtual ~IFightObserver() = default;
    virtual void on_fight(const std::shared_ptr<class NPC> attacker,
                          const std::shared_ptr<class NPC> defender,
                          bool win) = 0;
};

class NPC : public std::enable_shared_from_this<NPC>
{
public:
    NPC(NpcType t, std::string name, int x_pos, int y_pos);
    NPC(NpcType t, std::istream &is);
    virtual ~NPC() = default;

    const std::string &get_name() const noexcept { return name; }
    NpcType get_type() const noexcept { return type; }
    int get_x() const;
    int get_y() const;
    std::pair<int, int> position() const;

    void subscribe(const std::shared_ptr<IFightObserver> &observer);
    void fight_notify(const std::shared_ptr<NPC> &defender, bool win);

    virtual bool is_close(const std::shared_ptr<NPC> &other, size_t distance) const;

    void move(int shift_x, int shift_y, int max_x, int max_y);
    bool is_alive() const;
    void die();

    virtual bool accept(const std::shared_ptr<NPC> &attacker) = 0;

    virtual bool fight(const std::shared_ptr<Ork> &other) = 0;
    virtual bool fight(const std::shared_ptr<Squirrel> &other) = 0;
    virtual bool fight(const std::shared_ptr<Druid> &other) = 0;
    virtual void print() const = 0;

    virtual void save(std::ostream &os) const;

    friend std::ostream &operator<<(std::ostream &os, NPC &npc);

protected:
    std::string name;
    NpcType type{Unknown};
    int x{0};
    int y{0};
    bool alive{true};
    std::vector<std::shared_ptr<IFightObserver>> observers;
    mutable std::shared_mutex state_mutex;
};

int roll_dice();
void seed_random(unsigned int seed);
std::mutex &console_mutex();
