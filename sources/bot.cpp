#include <memory>
#include <iostream>
#include <cmath>
#include "utilities.h"
#include "state.h"
#include "bot.h"
#include "algorithm"

/*
int x;
int y;
int next_checkpoint_x;
int next_checkpoint_y;
int next_checkpoint_dist;
int next_checkpoint_angle;
int opponent_x;
int opponent_y;
std::vector<Checkpoint> checkpoints;
*/

int current_checkpoint = 0;
void Mudpath::update_state(std::unique_ptr<State>& state) {
    this->state = std::move(state);

    DEBUG("Next checkpoint X: " << this->state->next_checkpoint_x);
    DEBUG("Next checkpoint Y: " << this->state->next_checkpoint_y);
    DEBUG("Next checkpoint dist: " << this->state->next_checkpoint_dist);
    DEBUG("Next checkpoint angle: " << this->state->next_checkpoint_angle);
    DEBUG("Opponent X: " << this->state->opponent_x);
    DEBUG("Opponent Y: " << this->state->opponent_y);

    Checkpoint checkpoint;
    checkpoint.x = this->state->next_checkpoint_x;
    checkpoint.y = this->state->next_checkpoint_y;

    // Check if checkpoint is not present in this->state->checkpoints
    // TODO: Fix this, it's not working
    // TODO: Also fix the orbiting bug which causes us to lose many races
    if (!std::any_of(this->state->checkpoints.begin(), this->state->checkpoints.end(),
        [checkpoint](Checkpoint checkpoint_a) {
            return checkpoint_a.x == checkpoint.x && checkpoint_a.y == checkpoint.y;
        })) {
        this->state->checkpoints.push_back(checkpoint);
    }

    // Set current_checkpoint to the position of the checkpoint in checkpoints
    for (int i = 0; i < this->state->checkpoints.size(); i++) {
        if (this->state->checkpoints[i].x == checkpoint.x &&
            this->state->checkpoints[i].y == checkpoint.y) {
            current_checkpoint = i;
        }
    }

    DEBUG("Current checkpoint: " << current_checkpoint);
}

#define CANVAS_WIDTH 16000
#define CANVAS_HEIGHT 9000
#define MIN_POWER 30
#define MAX_ANGLE 120
bool is_boost_available = true;
void Mudpath::update(int current_turn) {
    this->state->target_x = this->state->next_checkpoint_x;
    this->state->target_y = this->state->next_checkpoint_y;
    this->state->power = 100;
    this->state->action = "";
    int threshold_distance;
    int distance_compensation;
    // If the next checkpoint is in the radius this->state->aware_area.radius
    // substract the distance from the radius
    if (this->state->next_checkpoint_dist <= this->state->aware_area.radius) {
        threshold_distance = this->state->aware_area.radius - this->state->next_checkpoint_dist;
        DEBUG("Threshold distance: " << threshold_distance);
        distance_compensation = this->state->aware_area.threshold * (
            this->state->aware_area.radius - threshold_distance
        );
        DEBUG("Distance compensation: " << distance_compensation);
        // Substract this->state->aware_area->threshold * threshold_distance from power
        this->state->power = distance_compensation;
    }

    int MAX_POWER = 100;
    // If next_checkpoint_distance is less than 2500, decrease max power
    if (this->state->next_checkpoint_dist < 2500) {
        MAX_POWER = 70;
    }

    if (this->state->power < MIN_POWER) {
        this->state->power = MIN_POWER;
    } else if (this->state->power > MAX_POWER) {
        this->state->power = MAX_POWER;
    }

    // Get the distance X and distance Y from the opponent
    int distance_x = this->state->opponent_x - this->state->x;
    int distance_y = this->state->opponent_y - this->state->y;

    if (
        this->state->next_checkpoint_dist < this->state->aware_area.closest_allowed
        // If the distance of the opponent is less than closest_allowed / 2
        && (
               std::abs(distance_x) < this->state->aware_area.closest_allowed / 2
            || std::abs(distance_y) < this->state->aware_area.closest_allowed / 2
           )
        // If the angle is 7 or less
        && std::abs(this->state->next_checkpoint_angle) <= 7
    ) {
        // Enable shield to reduce impaect
        // this->state->action = "SHIELD";
    }

    // Draw an imaginary circle, the top left corner of the circle will be the center
    // of the checkpoint. The radius of the circle will be the one of the checkpoint * 1.5
    Circle steering_circle;
    steering_circle.radius = Checkpoint::radius * 1.5;
    // If the next_checkpoint_x is > to the CANVAS_WIDTH / 2
    if (this->state->next_checkpoint_x > CANVAS_WIDTH / 2) {
        steering_circle.center.x = this->state->next_checkpoint_x - steering_circle.radius / 2;
    } else {
        steering_circle.center.x = this->state->next_checkpoint_x + steering_circle.radius / 2;
    }

    // If the next_checkpoint_y is > to the CANVAS_HEIGHT / 2
    if (this->state->next_checkpoint_y > CANVAS_HEIGHT / 2) {
        steering_circle.center.y = this->state->next_checkpoint_y - steering_circle.radius / 2;
    } else {
        steering_circle.center.y = this->state->next_checkpoint_y + steering_circle.radius / 2;
    }

    // Aim to the angle distance_compensation of the circle, 0 being the bottom and 90 being the right
    int angle_distance = distance_compensation;
    DEBUG("Angle distance: " << angle_distance);
    int angle_x = steering_circle.radius * std::sin(M_PI * 2 * angle_distance / 360);
    int angle_y = steering_circle.radius * std::cos(M_PI * 2 * angle_distance / 360);
    DEBUG("Angle X: " << angle_x);
    DEBUG("Angle Y: " << angle_y);
    // Sum the angle distance to the distance of the center of the circle
    int absolute_angle_x = steering_circle.center.x + angle_x;
    int absolute_angle_y = steering_circle.center.y + angle_y;
    DEBUG("Absolute angle X: " << absolute_angle_x);
    DEBUG("Absolute angle Y: " << absolute_angle_y);
    this->state->target_x = absolute_angle_x;
    this->state->target_y = absolute_angle_y;

    // If distance is < closest_allowed, aim at the next_checkpoint_x and next_checkpoint_y
    // near the center of the map
    if (this->state->next_checkpoint_dist < this->state->aware_area.closest_allowed * 2) {
        this->state->target_x = this->state->next_checkpoint_x;
        this->state->target_y = this->state->next_checkpoint_y;
    }

    // If the angle is > 30, set power to 0
    if (std::abs(this->state->next_checkpoint_angle) > MAX_ANGLE) {
        this->state->power = 0;
    } else if (std::abs(this->state->next_checkpoint_angle) > MAX_ANGLE / 2) {
        this->state->power = 10;
    }

    // If the distance from the opponent is less than closest_allowed, and
    // the distance to the next_checkpoint is less than closest_allowed
    int opponent_distance = std::sqrt(
        std::pow(this->state->opponent_x - this->state->x, 2)
        + std::pow(this->state->opponent_y - this->state->y, 2)
    );
    if (
        opponent_distance < this->state->aware_area.closest_allowed
        && this->state->next_checkpoint_dist < this->state->aware_area.closest_allowed
        && is_boost_available
    ) {
        // If the angle is < MAX_ANGLE / 2, aim at the opponent
        if (std::abs(this->state->next_checkpoint_angle) < MAX_ANGLE / 2) {
            is_boost_available = false;
            this->state->target_x = this->state->opponent_x - 40;
            this->state->target_y = this->state->opponent_y - 40;
            this->state->action = "BOOST";
        }
    }

    DEBUG("Opponent distance: " << distance_x);
    DEBUG("Target X: " << this->state->target_x);
    DEBUG("Target Y: " << this->state->target_y);
    DEBUG("Power: " << this->state->power);
}

void Mudpath::output() {
    std::cout <<
        this->state->target_x << " " << this->state->target_y << " ";

    if (this->state->action == "") {
        std::cout << this->state->power << std::endl;
    } else {
        std::cout << this->state->action << std::endl;
    }
}