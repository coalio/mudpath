#include <memory>
#include <iostream>
#include <string>
#include "utilities.h"
#include "state.h"
#include "debug.h"
#include "genetics.h"

#define CANVAS_WIDTH 16000
#define CANVAS_HEIGHT 9000

#if LEAGUE >= 6
#define NAME "Two Wheels and a Bullet"
#else
#define NAME "Unique Machine"
#endif

class Bot {
private:
    Circle steering_circle;
public:
    std::shared_ptr<State> state;
    std::shared_ptr<State> old_state;

    Bot(std::shared_ptr<State> _state) noexcept : state(_state) {
        DEBUG("Started Mudpath: " << NAME)
    }

    Point target;

    void setTarget(int x, int y);
    void setAcceleration(int acceleration);
    void setAction(std::string action);
    void aimForAttack();

    void update_state(std::unique_ptr<State> &state);
#if LEAGUE <= 5
    void update(int current_turn);
#else
    void update(Genetics::Move turn);
#endif
    void output();
};