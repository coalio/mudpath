#define LEAGUE 6 // 1 = Wood 3, 2 = Wood 2, 3 = Wood 1, 4 = Bronze, 5 = Silver, 6 = Gold, 7 = Legend

#include "sources/debug.h"
#include "sources/utilities.h"
#include "sources/state.h"
#include "sources/genetics.h"
#include "sources/genetics.cpp"
#include "sources/bot.h"
#include "sources/bot.cpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <utility>

#if LEAGUE >= 6
#define MODE "GENETIC"
#else
#define MODE "HEURISTICS"
#endif

class Runtime {
    private:
#if LEAGUE >= 6
        std::vector<Checkpoint> checkpoints;
        std::shared_ptr<Bot> racer;
        std::shared_ptr<Bot> hunter;
        std::shared_ptr<Bot> enemy_1;
        std::shared_ptr<Bot> enemy_2;
#else
        std::shared_ptr<Bot> bot;
#endif
    public:
#if LEAGUE >= 6
        Runtime(
            std::shared_ptr<Bot> bot1,
            std::shared_ptr<Bot> bot2,
            std::shared_ptr<Bot> bot3,
            std::shared_ptr<Bot> bot4
        ) : racer(std::move(bot1)),
            hunter(std::move(bot2)),
            enemy_1(std::move(bot3)),
            enemy_2(std::move(bot4))
        {
            std::shared_ptr<State> hunter_state = std::make_shared<State>();
            std::shared_ptr<State> enemy_1_state = std::make_shared<State>();
            std::shared_ptr<State> enemy_2_state = std::make_shared<State>();

            hunter->state = hunter_state;
            enemy_1->state = enemy_1_state;
            enemy_2->state = enemy_2_state;

            DEBUG("Initializing Runtime. League: " << LEAGUE)
        }
#else
        Runtime() {
            DEBUG("Initializing Runtime. League: " << LEAGUE)
        }
#endif
        void start();
        void get_input();
#if LEAGUE < 6
        int x, y;
        int next_checkpoint_x, next_checkpoint_y;
        int next_checkpoint_dist, next_checkpoint_angle;
        int opponent_x, opponent_y;
#endif
};

void Runtime::get_input() {
#if LEAGUE < 6
    // Before Gold League

    std::cin >> this->x >> this->y >> this->next_checkpoint_x >> this->next_checkpoint_y >> this->next_checkpoint_dist >> this->next_checkpoint_angle;
    std::cin.ignore();
    std::cin >> this->opponent_x >> this->opponent_y;
    std::cin.ignore();
#else
    // After Gold League
    for (int i = 0; i < 2; i++) {
        int x; // x position
        int y; // y position
        int vx; // x speed
        int vy; // y speed
        int angle; // angle
        int next_check_point_id; // next check point id
        std::cin >> x >> y >> vx >> vy >> angle >> next_check_point_id; std::cin.ignore();
        DEBUG("Received angle: " << angle)

        if (i == 0) {
            racer->state->x = x;
            racer->state->y = y;
            racer->state->vx = vx;
            racer->state->vy = vy;
            racer->state->angle = angle;
            racer->state->next_checkpoint_id = next_check_point_id;
        } else {
            hunter->state->x = x;
            hunter->state->y = y;
            hunter->state->vx = vx;
            hunter->state->vy = vy;
            hunter->state->angle = angle;
            hunter->state->next_checkpoint_id = next_check_point_id;
        }

    }

    for (int i = 0; i < 2; i++) {
        int x_2; // x position (opponent)
        int y_2; // y position (opponent)
        int vx_2; // x speed (opponent)
        int vy_2; // y speed (opponent)
        int angle_2; // angle (opponent)
        int next_check_point_id_2; // next check point id (opponent)
        std::cin >> x_2 >> y_2 >> vx_2 >> vy_2 >> angle_2 >> next_check_point_id_2; std::cin.ignore();

        if (i == 0) {
            enemy_1->state->x = x_2;
            enemy_1->state->y = y_2;
            enemy_1->state->vx = vx_2;
            enemy_1->state->vy = vy_2;
            enemy_1->state->angle = angle_2;
            enemy_1->state->next_checkpoint_id = next_check_point_id_2;
        } else {
            enemy_2->state->x = x_2;
            enemy_2->state->y = y_2;
            enemy_2->state->vx = vx_2;
            enemy_2->state->vy = vy_2;
            enemy_2->state->angle = angle_2;
            enemy_2->state->next_checkpoint_id = next_check_point_id_2;
        }
    }
#endif
}

void Runtime::start() {
    int current_turn = 0;
    int timeout = 100;

#if LEAGUE >= 6
    // After Gold League
    int laps; std::cin >> laps; std::cin.ignore();

    int checkpoint_count; std::cin >> checkpoint_count; std::cin.ignore();
    for (int i = 0; i < checkpoint_count; i++) {
        int checkpoint_x, checkpoint_y;
        std::cin >> checkpoint_x >> checkpoint_y;
        std::cin.ignore();

        checkpoints.push_back(
            Checkpoint(checkpoint_x, checkpoint_y)
        );
    }
#else
    std::shared_ptr<State> state(new State());
    bot = std::make_shared<Bot>(state);
#endif
    while (
        this->get_input(), true
    ) {
        current_turn++;
        DEBUG("Turn: " << current_turn);
#if LEAGUE < 6
        std::unique_ptr<State> updated_state(new State());
        updated_state->x = this->x;
        updated_state->y = this->y;
        updated_state->next_checkpoint_x = this->next_checkpoint_x;
        updated_state->next_checkpoint_y = this->next_checkpoint_y;
        updated_state->next_checkpoint_dist = this->next_checkpoint_dist;
        updated_state->next_checkpoint_angle = this->next_checkpoint_angle;
        updated_state->opponent_x = this->opponent_x;
        updated_state->opponent_y = this->opponent_y;
        bot->update_state(updated_state);
        // Update state
        // Update bot
        bot->update(current_turn);
        // Output
        bot->output();
#else
        racer->setTarget(
            checkpoints[
                static_cast<std::vector<int>::size_type>(
                    racer->state->next_checkpoint_id
                )
            ].x,

            checkpoints[
                static_cast<std::vector<int>::size_type>(
                    racer->state->next_checkpoint_id
                )
            ].y
        );

        hunter->setTarget(
            checkpoints[
                static_cast<std::vector<int>::size_type>(
                    hunter->state->next_checkpoint_id
                )
            ].x,

            checkpoints[
                static_cast<std::vector<int>::size_type>(
                    hunter->state->next_checkpoint_id
                )
            ].y
        );

        racer->setAcceleration(100);
        hunter->setAcceleration(20);

        // Start simulation
        Genetics::Simulation simulation;

        // Under this context, I want to avoid STL containers
        State* states[4];
        states[0] = racer->state.get();
        states[1] = hunter->state.get();
        states[2] = enemy_1->state.get();
        states[3] = enemy_2->state.get();

        DEBUG("Simulating " << MAX_SOLUTIONS << " solutions...");
        Genetics::Solution elite = simulation.run(
            states,
            checkpoints,
            timeout
        );

        // Apply the solution to the bots during MAX_MOVES amount of turns
        for (int i = 0; i < MAX_MOVES; i++) {
            DEBUG("Applying move: " << i)
            DEBUG("Expected X Y: " << elite.moves_racer[i].x << " " << elite.moves_racer[i].y);
            racer->update(
                elite.moves_racer[i]
            );

            hunter->update(
                elite.moves_hunter[i]
            );

            racer->output();
            hunter->output();
        }
#endif
    }
}

int main() {
#if LEAGUE >= 6
    // We will use this base to copy the bot into different
    // shared pointers.
    std::shared_ptr<State> bot_state = std::make_shared<State>();
    std::shared_ptr<Bot> bot = std::make_shared<Bot>(
        bot_state
    );

    Runtime runtime = Runtime(
        bot, // racer
        std::make_shared<Bot>(*bot), // hunter
        std::make_shared<Bot>(*bot), // enemy_1
        std::make_shared<Bot>(*bot) // enemy_2
    );
#else
    Runtime runtime = Runtime();
#endif
    runtime.start();
}