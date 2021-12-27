#include <vector>
#include <cmath>
#include "genetics.h"
#include "bot.h"
#include "state.h"
#include "utilities.h"
#include <algorithm>
using index_t = std::vector<int>::size_type;

void Genetics::Simulation::end(float &vx, float &vy, float &x, float &y) {
    x = std::round(x);
    y = std::round(y);
    vx = static_cast<int>(vx * 0.85); // truncate to remove decimal places
    vy = static_cast<int>(vy * 0.85);
}

float Genetics::Simulation::distance_base(Point pos, Point target_pos) {
    return (pos.x - target_pos.x) * (pos.x - target_pos.x)
         + (pos.y - target_pos.y) * (pos.y - target_pos.y);
}

float Genetics::Simulation::distance(Point pos, Point target_pos) {
    return std::sqrt(distance_base(pos, target_pos));
}

float Genetics::Simulation::get_angle(Point pos, Point target_pos) {
    float d = Genetics::Simulation::distance(pos, target_pos);
    float dx = (target_pos.x - pos.x) / d;
    float dy = (target_pos.y - pos.y) / d;

    // We multiply by 180.0 / PI to convert radiants to degrees.
    float a = std::acos(dx) * 180.0 / M_PI;

    // If the point I want is below me, I have to shift the angle for it to be correct
    if (dy < 0) {
        a = 360.0 - a;
    }

    return a;
}

float Genetics::Simulation::diff_angle(Point pos, Point target_pos, float angle) {
    float counter_angle = Genetics::Simulation::get_angle(pos, target_pos);

    // To know whether we should turn clockwise or not we look at the two ways and keep the smallest
    // The ternary operators replace the use of a modulo operator which would be slower
    float right = angle <= counter_angle ? counter_angle - angle : 360.0 - angle + counter_angle;
    float left = angle >= counter_angle ? angle - counter_angle : angle + 360.0 - counter_angle;

    if (right < left) {
        return right;
    } else {
        // We return a negative angle if we must rotate to left
        return -left;
    }
}

void Genetics::Simulation::rotate(Point pos, Point target_pos, float &ref_angle) {
    float angle = Genetics::Simulation::get_angle(pos, target_pos);
    if (angle > 18.0) {
        angle = 18.0;
    } else if (angle < -18.0) {
        angle = -18.0;
    }

    ref_angle += angle;

    // The % operator is slow. If we can avoid it, it's better.
    if (ref_angle >= 360.0) {
        ref_angle = ref_angle - 360.0;
    } else if (ref_angle < 0.0) {
        ref_angle += 360.0;
    }
}

void Genetics::Simulation::boost(int thrust, float &vx, float &vy, float angle) {
    float ra = angle * M_PI / 180.0;

    vx += cos(ra) * thrust;
    vy += sin(ra) * thrust;
}

void Genetics::Simulation::move(float &x, float &y, float vx, float vy, float t) {
    x += vx * t;
    y += vy * t;
}

float Genetics::Solution::evaluate() {
    // Racer fitness
    float distance = 0.0;
    float score_racer = 0;
    float score_hunter = 0;

    // If timeout is 0, we don't care about the score
    if (!this->final_timeout) {
        // The worst possible score as we already lost
        this->fitness = -INFINITY;
        return this->fitness;
    }

    // Racer is evaluated by the distance to the target and checkpoints passed
    // Hunter is evaluated by how many times it collided
    score_racer += this->checkpoints_passed << 16;
    score_hunter += this->hunter_collisions << 3;

    Genetics::Move racer_last_state = this->moves_racer[MAX_MOVES - 1];
    Genetics::Move hunter_last_state = this->moves_hunter[MAX_MOVES - 1];

    this->final_distance_r = Genetics::Simulation::distance(
        Point {
            racer_last_state.x,
            racer_last_state.y
        },
        Point {
            this->last_checkpoint_r.x,
            this->last_checkpoint_r.y
        }
    );

    // Are we too far away from the checkpoint?
    score_racer -= this->final_distance_r;

    this->fitness = score_racer + score_hunter;
    return this->fitness;
}

Genetics::Solution Genetics::Simulation::run(
    State* states[4],
    std::vector<Checkpoint> checkpoints,
    int timeout
) {
    // Simulate a turn and print the result
    // This tracks the time during the turn. The goal is to reach 1.0
    float t = 0.0;

    // Convert states to units for easier computing
    Genetics::Unit units[4];
    for (int i = 0; i < 4; i++) {
        units[i] = Genetics::Unit(
            i,
            BOT_RADIUS,
            states[i]->x,
            states[i]->y,
            states[i]->vx,
            states[i]->vy,
            states[i]->angle
        );
    }

    Genetics::Unit checkpoint_units[3];
    for (index_t i = 0; i < 3; i++) {
        checkpoint_units[i] = Genetics::Unit(
            i,
            Checkpoint::radius,
            checkpoints[i].x,
            checkpoints[i].y,
            0,
            0,
            0
        );

        checkpoint_units[i].is_checkpoint = true;
    }

    Genetics::Collision* prev_collision = NULL;
    Genetics::Collision* curr_collision = NULL;

    Genetics::Solution solutions[MAX_SOLUTIONS];

    // Generate solutions
    for (index_t solution_i = 0; solution_i < MAX_SOLUTIONS; solution_i++) {
        solutions[solution_i] = Genetics::Solution();
        solutions[solution_i].checkpoints_passed = 0;
        solutions[solution_i].final_timeout = timeout;

        float thrust = std::rand() % 100;

        if (thrust > 100.0) {
            thrust = 100.0;
        }

        for (int move_i = 0; move_i < MAX_MOVES; move_i++) {
            t = 0.0;
            solutions[solution_i].moves_racer[move_i] = Genetics::Move();
            solutions[solution_i].moves_hunter[move_i] = Genetics::Move();

            while (t < 1.0) {
                if (curr_collision != NULL) {
                    prev_collision = curr_collision;
                }

                // We look for all the collisions that are going to occur during the turn
                for (int i = 0; i < 4; i++) {
                    Point target;

                    if (i == 0) {
                        // Aim at the next checkpoint and give it a small deviation
                        float deviation_x =
                            (std::rand() % 1200) *
                            checkpoint_units[states[i]->next_checkpoint_id].x /
                            2400;

                        float deviation_y =
                            (std::rand() % 1200) *
                            checkpoint_units[states[i]->next_checkpoint_id].y /
                            2400;
                        target = Point {
                            checkpoint_units[states[i]->next_checkpoint_id].x
                                + deviation_x,
                            checkpoint_units[states[i]->next_checkpoint_id].y
                                + deviation_y
                        };
                    } else if (i == 1) {
                        // Aim at the opponent's racer
                        State enemy_racer;
                        // Find the one with the closest distance to its next checkpoint
                        float min_distance = INFINITY;
                        for (int j = 0; j < 4; j++) {
                            if (j == i) {
                                continue;
                            }

                            float distance = Genetics::Simulation::distance(
                                Point {
                                    states[j]->x,
                                    states[j]->y
                                },
                                Point {
                                    checkpoint_units[states[j]->next_checkpoint_id].x,
                                    checkpoint_units[states[j]->next_checkpoint_id].y
                                }
                            );

                            if (distance < min_distance) {
                                min_distance = distance;
                                enemy_racer = *states[j];
                            }
                        }

                        target = Point {
                            enemy_racer.x + enemy_racer.vx, // Aim at the racer's next position
                            enemy_racer.y + enemy_racer.vy
                        };
                    } else if (i == 2){
                        // Aim at their next checkpoint
                        target = Point {
                            checkpoint_units[states[i]->next_checkpoint_id].x,
                            checkpoint_units[states[i]->next_checkpoint_id].y
                        };
                    } else {
                        // Aim at our next checkpoint
                        target = Point {
                            checkpoint_units[states[1]->next_checkpoint_id].x,
                            checkpoint_units[states[1]->next_checkpoint_id].y
                        };
                    }

                    float angle = Genetics::Simulation::diff_angle(
                        Point{
                            units[i].x,
                            units[i].y
                        },
                        target,
                        units[i].angle
                    );

                    units[i].thrust = thrust;

                    if (i == 0) {
                        solutions[solution_i].moves_racer[move_i].angle = angle;
                        solutions[solution_i].moves_racer[move_i].thrust = thrust;
                    } else if (i == 1) {
                        solutions[solution_i].moves_hunter[move_i].angle = angle;
                        solutions[solution_i].moves_hunter[move_i].thrust = thrust;
                    }

                    // Aim at the target and boost
                    Genetics::Simulation::rotate(
                        Point {
                            units[i].x,
                            units[i].y
                        },
                        target,
                        units[i].angle
                    );
                    Genetics::Simulation::boost(
                        thrust,
                        units[i].vx,
                        units[i].vy,
                        units[i].angle
                    );


                    // Collision with another pod?
                    for (int j = i + 1; j < 4; j++) {
                        Genetics::Collision* col =
                            Genetics::Simulation::await_collision(
                                units[i],
                                units[j]
                            );

                        // If the collision occurs earlier than the one we currently have we keep it
                        if (col != NULL && col->t + t < 1.0 && (curr_collision == NULL || col->t < curr_collision->t)) {
                            curr_collision = col;
                        }

                        delete col;
                    }

                    // Collision with another checkpoint?
                    // It is unnecessary to check all checkpoints here. We only test the pod's next checkpoint.
                    // We could look for the collisions of the pod with all the checkpoints, but if such a collision happens it wouldn't impact the game in any way
                    Genetics::Collision* col =
                        Genetics::Simulation::await_collision(
                            units[i],
                            checkpoint_units[states[i]->next_checkpoint_id]
                        );

                    // If the collision happens earlier than the current one we keep it
                    if (col != NULL && col->t + t < 1.0 && (curr_collision == NULL || col->t < curr_collision->t)) {
                        curr_collision = col;
                    }

                    delete col;
                }

                if (curr_collision == NULL || (curr_collision != NULL && (
                        prev_collision != NULL
                        && curr_collision->a.id == prev_collision->a.id && curr_collision->b.id == prev_collision->b.id
                        && curr_collision->t == prev_collision->t && curr_collision->t == 0.0
                    ))
                ) {
                    // No collision, we can move the pods until the end of the turn
                    for (int i = 0; i < 4; i++) {
                        Genetics::Simulation::move(
                            units[i].x,
                            units[i].y,
                            units[i].vx,
                            units[i].vy,
                            1.0 - t
                        );
                    }

                    // End of the turn
                    t = 1.0;
                } else {
                    // Move the pods to reach the time `t` of the collision
                    for (int i = 0; i < 4; i++) {
                        Genetics::Simulation::move(
                            units[i].x,
                            units[i].y,
                            units[i].vx,
                            units[i].vy,
                            curr_collision->t
                        );
                    }

                    // Play out the collision
                    Genetics::Simulation::bounce(
                        units[curr_collision->a.id].x,
                        units[curr_collision->a.id].y,
                        units[curr_collision->a.id].vx,
                        units[curr_collision->a.id].vy,
                        solutions[solution_i].final_timeout,
                        solutions[solution_i].checkpoints_passed,
                        states[curr_collision->a.id]->shield,
                        curr_collision->b
                    );

                    t += curr_collision->t;

                    if (curr_collision->a.id == 1) {
                        // Hunter collided with an opponent
                        solutions[solution_i].hunter_collisions++;
                    }
                }

                solutions[solution_i].moves_racer[move_i].x = units[0].x;
                solutions[solution_i].moves_racer[move_i].y = units[0].y;
                solutions[solution_i].moves_hunter[move_i].x = units[1].x;
                solutions[solution_i].moves_hunter[move_i].y = units[1].y;
            }

            for (int i = 0; i < 4; i++) {
                this->end(
                    units[i].vx,
                    units[i].vy,
                    units[i].x,
                    units[i].y
                );
            }

            solutions[solution_i].final_timeout--;
        }

        // Restart the units
        for (int i = 0; i < 4; i++) {
            units[i].x = states[i]->x;
            units[i].y = states[i]->y;
            units[i].vx = states[i]->vx;
            units[i].vy = states[i]->vy;
            units[i].angle = states[i]->angle;
            units[i].thrust = 0;
        }
    }

    // Calculate the fitness of each solution
    float scores[MAX_SOLUTIONS];
    for (index_t i = 0; i < MAX_SOLUTIONS; i++) {
        scores[i] = solutions[i].evaluate();
    }

    // Sort the solutions by their fitness
    int loc, j, k;
    float selected;
    for (int i = 0; i < MAX_SOLUTIONS; i++) {
        j = i - 1;
        selected = scores[i];
        loc = binary_search(scores, selected, 0, j);

        // Move all elements after location to create space
        while (j >= loc) {
            scores[j + 1] = scores[j];
            j--;
        }

        scores[j + 1] = selected;
    }

    // Debug the scores
    for (int i = 0; i < MAX_SOLUTIONS; i++) {
        DEBUG("Solution " << i << ": " << scores[i]);
    }

    return solutions[MAX_SOLUTIONS - 1]; // Last solution is the best
}

Genetics::Collision* Genetics::Simulation::await_collision(Genetics::Unit self, Genetics::Unit other) {
    // Square of the distance
    float dist = Genetics::Simulation::distance(
        Point(self.x, self.y), Point(other.x, other.y)
    );
    float squared_radius = std::sqrt((self.r + other.r) * (self.r + other.r));

    if (dist < squared_radius) {
        // Objects are already touching each other. We have an immediate collision.
        return new Genetics::Collision(self, other, 0.0);
    }

    // Objects with the same speed will never collide
    if (self.vx == other.vx && self.vy == other.vy) {
        return NULL;
    }

    // We place ourselves in the reference frame of "other". "other" is therefore stationary and is at (0,0)
    float x = self.x - other.x;
    float y = self.y - other.y;
    Point myp = Point(x, y);
    float vx = self.vx - other.vx;
    float vy = self.vy - other.vy;
    Point up = Point(0, 0);

    // We look for the closest point to the other unit (which is in (0,0)) on the line described by our speed vector
    Point p = up.closest(myp, Point(x + vx, y + vy));

    // Square of the distance between the other unit and the closest point to it on the line described by our speed vector
    float pdist = Genetics::Simulation::distance(up, p);

    // Square of the distance between us and that point
    float mypdist = Genetics::Simulation::distance(myp, p);

    // If the distance between the other and this line is less than the sum of the radii, there might be a collision
    if (pdist < squared_radius) {
        // Our speed on the line
        float length = std::sqrt(vx * vx + vy * vy);

        // We move along the line to find the point of impact
        float backdist = std::sqrt(squared_radius - pdist);
        p.x = p.x - backdist * (vx / length);
        p.y = p.y - backdist * (vy / length);

        // If the point is now further away it means we are not going the right way, therefore the collision won't happen
        if (Genetics::Simulation::distance(myp, p) > mypdist) {
            return NULL;
        }

        pdist = Genetics::Simulation::distance(p, myp);

        // The point of impact is further than what we can travel in one turn
        if (pdist > length) {
            return NULL;
        }

        // Time needed to reach the impact point
        float t = pdist / length;

        return new Genetics::Collision(self, other, t);
    }

    return NULL;
}

// Elastic collision with a half-momentum of 120
void Genetics::Simulation::bounce(
    float &x,
    float &y,
    float &vx,
    float &vy,
    int &timeout,
    int &checkpoints_passed,
    bool shield,
    Genetics::Unit &other
) {
    if (other.is_checkpoint) {
        // Collision with a checkpoint
        timeout = 101; // + 1 so its 100 and not 99 for the next turn
        checkpoints_passed++;
        return;
    } else {
        // If a pod has its shield active its mass is 10 otherwise it's 1
        float m1 = shield ? 10 : 1;
        float m2 = 1;
        float mcoeff = (m1 + m2) / (m1 * m2);

        float nx = x - other.x;
        float ny = y - other.y;

        // Square of the distance between the 2 pods. This value could be hardcoded because it is always 800²
        float nxnysquare = nx*nx + ny*ny;

        float dvx = vx - other.vx;
        float dvy = vy - other.vy;

        // fx and fy are the components of the impact vector. product is just there for optimisation purposes
        float product = nx * dvx + ny * dvy;
        float fx = (nx * product) / (nxnysquare * mcoeff);
        float fy = (ny * product) / (nxnysquare * mcoeff);

        // We apply the impact vector once
        vx -= fx / m1;
        vy -= fy / m1;
        other.vx += fx / m2;
        other.vy += fy / m2;

        // If the norm of the impact vector is less than 120, we normalize it to 120
        float impulse = std::sqrt(fx*fx + fy*fy);
        if (impulse < 120.0) {
            fx = fx * 120.0 / impulse;
            fy = fy * 120.0 / impulse;
        }

        // We apply the impact vector a second time
        vx -= fx / m1;
        vy -= fy / m1;
        other.vx += fx / m2;
        other.vy += fy / m2;
    }
}