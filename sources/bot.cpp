#include "utilities.h"
#include "state.h"
#include <memory>
#include "bot.h"

void Mudpath::update_state(std::unique_ptr<State>& state) {
    this->state = std::move(state);
}