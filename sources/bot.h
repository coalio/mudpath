#include <memory>
#include <iostream>
#include <string>
#include "utilities.h"
#include "state.h"
#include "debug.h"

class Bot {
private:
    Circle steering_circle;
    std::shared_ptr<State> state;
    std::shared_ptr<State> old_state;
public:
    Bot(std::shared_ptr<State> state) : state(state) {
        DEBUG("Started Mudpath: " << state->getName())
    }

    Point target;

    void setTarget(int x, int y);
    void setAcceleration(int acceleration);
    void setAction(std::string action);
    void aimForAttack();

    void update_state(std::unique_ptr<State> &state);
    void update(int current_turn);
    void output();
};