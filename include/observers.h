#pragma once

#include "npc.h"

#include <fstream>
#include <memory>
#include <mutex>
#include <string>

class ConsoleObserver : public IFightObserver
{
public:
    void on_fight(const std::shared_ptr<NPC> attacker,
                  const std::shared_ptr<NPC> defender,
                  bool win) override;
};

class FileObserver : public IFightObserver
{
public:
    explicit FileObserver(const std::string &path);
    void on_fight(const std::shared_ptr<NPC> attacker,
                  const std::shared_ptr<NPC> defender,
                  bool win) override;

private:
    std::ofstream out;
    std::mutex mtx;
};
