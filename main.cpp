#include "sources/debug.h"
#include "sources/utilities.h"
#include "sources/state.h"
#include "sources/bot.h"
#include "sources/bot.cpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

class Runtime {
    private:
        int x, y;
        int next_checkpoint_x, next_checkpoint_y;
        int next_checkpoint_dist, next_checkpoint_angle;
        int opponent_x, opponent_y;
        std::shared_ptr<Bot> bot;
    public:
        void start();
        void get_input();
};

void Runtime::get_input() {
    std::cin >> x >> y >> next_checkpoint_x >> next_checkpoint_y >> next_checkpoint_dist >> next_checkpoint_angle;
    std::cin.ignore();
    std::cin >> opponent_x >> opponent_y;
    std::cin.ignore();
}

void Runtime::start() {
    std::shared_ptr<State> state(new State());
    std::shared_ptr<Bot> bot = std::make_shared<Bot>(state);

    int current_turn = 0;
    while (
        this->get_input(), true)
    {
        current_turn++;
        DEBUG("Turn: " << current_turn);
        std::unique_ptr<State> updated_state(new State());
        updated_state->x = x;
        updated_state->y = y;
        updated_state->next_checkpoint_x = next_checkpoint_x;
        updated_state->next_checkpoint_y = next_checkpoint_y;
        updated_state->next_checkpoint_dist = next_checkpoint_dist;
        updated_state->next_checkpoint_angle = next_checkpoint_angle;
        updated_state->opponent_x = opponent_x;
        updated_state->opponent_y = opponent_y;

        // Update state
        bot->update_state(updated_state);
        // Update bot
        bot->update(current_turn);
        // Output
        bot->output();
    }
}

int main() {
    Runtime runtime = Runtime();
    runtime.start();
}