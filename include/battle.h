#pragma once

#include "factory.h"

#include <ostream>
#include <string>
#include <vector>

void print_all(const set_t &array, std::ostream &os);
void save(const set_t &array, const std::string &filename);
set_t load(const std::string &filename, const std::vector<std::shared_ptr<IFightObserver>> &observers);

std::ostream &operator<<(std::ostream &os, const set_t &array);

set_t fight(const set_t &array, size_t distance);
bool name_exists(const set_t &array, const std::string &name);
