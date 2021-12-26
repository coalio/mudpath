#include <memory>
#include <vector>
#include "bot.h"
#include "state.h"
#include "utilities.h"
#define MAX_MOVES 6
#define MAX_SOLUTIONS 10
#define BOT_RADIUS 400
// The genetic algorithm is designed to run as many
// calculations as possible per turn. The performance
// is a critical factor in the algorithm, therefore I
// avoid as much overhead as possible in order to
// maximize it.

namespace Genetics {
    class Move {
        public:
            int thrust;
            float angle;
            float x;
            float y;
    };

    class Solution {
        public:
            State state;
            Genetics::Move moves_racer[MAX_MOVES];
            Genetics::Move moves_hunter[MAX_MOVES];
            float fitness;
            // Racers final distance
            int final_distance_r;
            // Hunters final distance (to the enemy checkpoint)
            int final_distance_h;

            int checkpoints_passed;
            int hunter_collisions;
            int final_timeout;

            // Racer's last checkpoint pos
            Point last_checkpoint_r;

            void randomize(float mutation_rate);
            void mutate(float amplitude);
            void crossover(Solution &other);
            float evaluate();
    };

    class Unit {
        public:
            Unit() {}
            Unit(int _id, float _r, float _x, float _y, float _vx, float _vy, float _angle)
                : id(_id), r(_r), x(_x), y(_y), vx(_vx), vy(_vy), angle(_angle) {}

            int id;
            float r;
            float x;
            float y;
            float vx;
            float vy;
            float angle;
            int thrust;

            bool is_checkpoint = false;
    };

    class Collision {
        public:
            Collision(Genetics::Unit _a, Genetics::Unit _b, float _t)
                : a(_a), b(_b), t(_t) {}

            Genetics::Unit a;
            Genetics::Unit b;
            float t;
    };

    class Simulation {
        private:
            int bot_x, bot_y, bot_vx, bot_vy;
            float bot_angle;
        public:
            Genetics::Solution run(
                State* states[4],
                std::vector<Checkpoint> checkpoints,
                int timeout
            );
            Genetics::Collision* await_collision(
                Genetics::Unit self, Genetics::Unit other
            );
            void end(float &vx, float &vy, float &x, float &y);

            static float distance(Point a, Point b);
            static float distance_base(Point a, Point b);
            static float get_angle(Point point_a, Point point_b);
            static float diff_angle(Point a, Point , float angle);
            static void rotate(Point a, Point b, float &ref_angle);
            static void boost(int thrust, float &vx, float &vy, float angle);
            static void move(float &x, float &y, float vx, float vy, float t);
            static void bounce(
                float &x,
                float &y,
                float &vx,
                float &vy,
                int &timeout,
                int &checkpoints_passed,
                bool shield,
                Genetics::Unit &other
            );
    };
}