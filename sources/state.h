#include <string>

struct State {
    int x;
    int y;
    int next_checkpoint_x;
    int next_checkpoint_y;
    int next_checkpoint_dist;
    int next_checkpoint_angle;
    int opponent_x;
    int opponent_y;

    std::string getName() {
        return "Mud";
    }
};