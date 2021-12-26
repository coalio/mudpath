#include <string>
#include <vector>
#include "utilities.h"
#ifndef LEAGUE
#define LEAGUE 6
#endif

struct AwareArea {
    int radius = 4400;
    int closest_allowed = 2000;
    float threshold = 0.05;
};

struct State {
#if LEAGUE <= 5
    int x;
    int y;
    int next_checkpoint_x;
    int next_checkpoint_y;
    int next_checkpoint_dist;
    int next_checkpoint_angle;
    int opponent_x;
    int opponent_y;

    int target_x;
    int target_y;
    int power;

    std::vector<Checkpoint> checkpoints;

    AwareArea aware_area;

    std::string action;

    std::string getName() {
        return "Mud";
    }
#else
    float x;
    float y;
    float vx;
    float vy;
    float angle;
    int next_checkpoint_id;
    int target_x;
    int target_y;
    int power;

    bool shield = false;
    int shield_counter = 0;

    std::string action;
#endif
};