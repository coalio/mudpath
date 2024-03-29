#include <memory>
#include <iostream>
#include <cmath>
#include <string>
#include "utilities.h"
#include "state.h"
#include "bot.h"
#include "genetics.h"
#include "algorithm"

void Bot::setTarget(int x, int y) {
    this->state->target_x = x;
    this->state->target_y = y;
}

void Bot::setAcceleration(int acceleration) {
    this->state->power = acceleration;
}

void Bot::setAction(std::string action) {
    this->state->action = action;
}

#if LEAGUE < 6
void Bot::update_state(std::unique_ptr<State>& state) {
    this->old_state = std::move(this->state);
    this->state = std::move(state);

    DEBUG("Next checkpoint X: " << this->state->next_checkpoint_x);
    DEBUG("Next checkpoint Y: " << this->state->next_checkpoint_y);
    DEBUG("Next checkpoint dist: " << this->state->next_checkpoint_dist);
    DEBUG("Next checkpoint angle: " << this->state->next_checkpoint_angle);
    DEBUG("Opponent X: " << this->state->opponent_x);
    DEBUG("Opponent Y: " << this->state->opponent_y);
}
#endif

#if LEAGUE <= 5
#define MAX_POWER 100
#define MIN_POWER 30
#define MAX_ANGLE 120
#define MIN_BOOST_DISTANCE 7000
bool is_boost_available = true;
void Bot::update(int current_turn) {
    this->setTarget(
        this->state->next_checkpoint_x,
        this->state->next_checkpoint_y
    );

    this->setAcceleration(100);
    this->setAction("");

    int threshold_distance;
    int distance_compensation;
    // Get the distance X and distance Y from the opponent
    int opponent_distance_x = this->state->opponent_x - this->state->x;
    int opponent_distance_y = this->state->opponent_y - this->state->y;
    int opponent_distance = std::sqrt(
        std::pow(this->state->opponent_x - this->state->x, 2)
        + std::pow(this->state->opponent_y - this->state->y, 2)
    );
    DEBUG("Opponent distance: " << opponent_distance);
    int opponent_to_checkpoint_distance = std::sqrt(
        std::pow(this->state->next_checkpoint_x - this->state->opponent_x, 2)
        + std::pow(this->state->next_checkpoint_y - this->state->opponent_y, 2)
    );
    DEBUG("Opponent to checkpoint distance: " << opponent_to_checkpoint_distance);
    // Draw an imaginary circle, the top left corner of the circle will be the center
    // of the checkpoint. The radius of the circle will be the one of the checkpoint * 0.5
    steering_circle.radius = Checkpoint::radius / 2;
    steering_circle.center.x = this->state->next_checkpoint_x;
    steering_circle.center.y = this->state->next_checkpoint_y;

    // Find the angle of the bot from the center of the circle
    float bot_angle = (std::atan2(
        this->state->y - steering_circle.center.y,
        this->state->x - steering_circle.center.x
    ) * 360 / M_PI);
    // Aim to the angle distance_compensation of the circle, 0 being the bottom and 90 being the right
    int angle_distance = this->state->next_checkpoint_angle + bot_angle;
    // Invert angle distance (if its positive, make negative, otherwise make positive)
    angle_distance = -angle_distance;

    DEBUG("Angle distance: " << angle_distance);
    DEBUG("Bot angle: " << bot_angle);
    int angle_x = steering_circle.radius * std::sin(M_PI * 2 * angle_distance / 360);
    int angle_y = steering_circle.radius * std::cos(M_PI * 2 * angle_distance / 360);
    // Multiply angle_x and angle_y by the speed of the bot (the current position - the position of the turn before)
    int bot_speed_x = this->state->x - this->old_state->x;
    int bot_speed_y = this->state->y - this->old_state->y;
    DEBUG("Bot speed x: " << bot_speed_x);
    DEBUG("Bot speed y: " << bot_speed_y);
    int offset_multiplier = -((int) M_PI); // Cast M_PI to integer
    int offset_x = offset_multiplier * bot_speed_x;
    int offset_y = offset_multiplier * bot_speed_y;
    DEBUG("Angle X: " << angle_x);
    DEBUG("Angle Y: " << angle_y);
    DEBUG("Offset X: " << offset_x);
    DEBUG("Offset Y: " << offset_y);
    // Sum the angle distance to the distance of the center of the circle
    int absolute_angle_x = angle_x + offset_x;
    int absolute_angle_y = angle_y + offset_y;
    DEBUG("Absolute angle X: " << offset_x);
    DEBUG("Absolute angle Y: " << offset_y);
    this->state->target_x = steering_circle.center.x + absolute_angle_x;
    this->state->target_y = steering_circle.center.y + absolute_angle_y;

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

    if (this->state->power < MIN_POWER) {
        this->state->power = MIN_POWER;
    } else if (this->state->power > MAX_POWER) {
        this->state->power = MAX_POWER;
    }

    // If the angle is > 30, set power to 0
    if (std::abs(this->state->next_checkpoint_angle) > MAX_ANGLE) {
        this->state->power = 0;
    } else if (std::abs(this->state->next_checkpoint_angle) > MAX_ANGLE / 2) {
        this->state->power = 10;
    }

    // If the distance from the opponent is less than closest_allowed, and
    // the distance to the next_checkpoint is less than closest_allowed
#if LEAGUE <= 5
    if (
        opponent_distance < this->state->aware_area.closest_allowed
        && this->state->next_checkpoint_dist < this->state->aware_area.closest_allowed
        && is_boost_available
    ) {
        if (opponent_to_checkpoint_distance < this->state->next_checkpoint_dist) {
            // If the angle is < MAX_ANGLE / 2, aim at the opponent
            if (std::abs(this->state->next_checkpoint_angle) < MAX_ANGLE / 2) {
                is_boost_available = false;
                this->state->target_x = this->state->opponent_x - 40;
                this->state->target_y = this->state->opponent_y - 40;
                this->state->action = "BOOST";
            }
        }
    }
#elif LEAGUE == 2 or LEAGUE == 3
    // If the distance from the checkpoint is more than MIN_BOOST_DISTANCE
    // and the absolute angle is less than 5, boost
    if (
        this->state->next_checkpoint_dist > MIN_BOOST_DISTANCE
        && std::abs(this->state->next_checkpoint_angle) < 5
    ) {
        this->state->action = "BOOST";
    }
#endif

    DEBUG("Opponent distance: " << opponent_distance_x);
    DEBUG("Target X: " << this->state->target_x);
    DEBUG("Target Y: " << this->state->target_y);
    DEBUG("Power: " << this->state->power);
}
#else
void Bot::update(Genetics::Move turn) {
    // Since we delegated all the processing to the Simulation class, we just
    // need to perform a little bit of processing on the state to
    // transform it into the format that the game expects
    DEBUG("Updating bot: " <<  turn.angle << ", " << turn.thrust);
    float a = this->state->angle + turn.angle;

    if (a >= 360.0) {
        a = a - 360.0;
    } else if (a < 0.0) {
        a += 360.0;
    }

    // Look for a point corresponding to the angle we want
    // Multiply by 10000.0 to limit rounding errors
    a = a * M_PI / 180.0;
    float px = this->state->x + std::cos(a) * 10000.0;
    float py = this->state->y + std::sin(a) * 10000.0;

    this->state->target_x = px;
    this->state->target_y = py;
    this->state->power = turn.thrust;

    float new_angle = this->state->angle;
    Genetics::Simulation::rotate(
        Point {
            this->state->x,
            this->state->y
        },
        Point {
            static_cast<float>(this->state->target_x),
            static_cast<float>(this->state->target_y)
        },
        new_angle
    );

    DEBUG("Expected angle: " << new_angle)

    this->state->angle = new_angle;
}
#endif

void Bot::output() {
    std::cout <<
        std::round(this->state->target_x) << " " << std::round(this->state->target_y) << " ";

    if (this->state->action == "") {
        std::cout << this->state->power << std::endl;
    } else {
        std::cout << this->state->action << std::endl;
    }
}