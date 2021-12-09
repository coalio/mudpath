#include "utilities.h"
#include "state.h"
#include "debug.h"
#include <memory>

class Mudpath {
private:
    std::shared_ptr<State> state;
public:
    Mudpath(std::shared_ptr<State> state) : state(state) {
        DEBUG("Started Mudpath: " << state->getName())
    }

    ~Mudpath();

    Point target;

    int getX();
    int getY();

    void setTarget(int x, int y);
    void setAcceleration(int acc);

    void update_state(std::unique_ptr<State> &state);
    void update();
};