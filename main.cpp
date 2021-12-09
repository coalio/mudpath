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
        std::shared_ptr<Mudpath> bot;
    public:
        void start();
        void get_input();
};

void Runtime::get_input() {
    std::cin
        >> x
        >> y
        >> next_checkpoint_x
        >> next_checkpoint_y
        >> next_checkpoint_dist
        >> next_checkpoint_angle,
    std::cin.ignore(),
    std::cin
        >> opponent_x
        >> opponent_y,
    std::cin.ignore(),
    std::cout << next_checkpoint_x << " " << next_checkpoint_y << " 80" << std::endl;
}

void Runtime::start() {
    std::shared_ptr<State> state(new State());
    std::shared_ptr<Mudpath> bot = std::make_shared<Mudpath>(state);

    while (
        this->get_input(), true
    ) {
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
    }
}

int main() {
    Runtime runtime = Runtime();
    runtime.start();
}