#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <map>
#include <fstream>

#define CANVAS_SIZE 400
#define EMULATION_FRAMES 1000
#define OBJECTS_RADIUS 10

struct Pod {
    int x;
    int y;
    int radius;
};

struct Bot {
    int x;
    int y;
    int velocity_x;
    int velocity_y;
    int sqr_area;
    int angle;
};

struct Point {
    int x;
    int y;
};

struct Area {
    int x;
    int y;
    int width;
    int height;
};

int main(const int argc, const char *argv[]) {
    // Sends simple input to mudpath to verify that it is working

    std::map<std::string, std::string> args;
    args["canvas_size"] = std::to_string(CANVAS_SIZE);
    args["emulation_frames"] = std::to_string(EMULATION_FRAMES);
    // Get command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        std::string key = arg.substr(0, arg.find("="));
        std::string value = arg.substr(arg.find("=") + 1);
        args[key] = value;
    }

    // Initialize the random number generator with the current time
    srand(time(NULL));

    std::map<std::string, Pod> params;

    // Create 3 random pods, each pod must be at atleast radius + 100 away from the other pods
    for (int i = 0; i < 3; i++) {
        Pod pod;
        pod.x = rand() % CANVAS_SIZE;
        pod.y = rand() % CANVAS_SIZE;
        pod.radius = 10;
        params["pod" + std::to_string(i)] = pod;

        // Verify that the pod is at least radius + 100 away from the other pods, if not, move
        for (int j = 0; j < i; j++) {
            Pod other = params["pod" + std::to_string(j)];
            if (std::abs(pod.x - other.x) < pod.radius + other.radius + 100) {
                pod.x = pod.x + rand() % CANVAS_SIZE + other.radius + 100;
                pod.y = pod.y + rand() % CANVAS_SIZE + other.radius + 100;
                j = -1;
            }
        }
    }

    // Print the pods
    for (int i = 0; i < 3; i++) {
        Pod pod = params["pod" + std::to_string(i)];
        std::cout << "Pod " << i << ": " << pod.x << " " << pod.y << " " << pod.radius << std::endl;
    }

    // Initialize the player at pod 0
    Bot bot;
    bot.x = params["pod0"].x;
    bot.y = params["pod0"].y;
    bot.velocity_x = 0;
    bot.velocity_y = 0;
    bot.sqr_area = 10;

    // Print the player
    std::cout << "Player: " << bot.x << " " << bot.y << " " << bot.sqr_area << std::endl;

    std::vector<Point> points;
    std::vector<Point> total_points;

    // For EMULATION_FRAMES turns, move the player around the canvas and check if it is in a pod
    // If its in a pod, move it to the next pod at bot's velocity

    Pod next_pod = params["pod0"];
    short curr_pod = 0;

    for (int i = 0; i < EMULATION_FRAMES; i++) {
        // Move the player
        // Check if the player is in the pod
        if (std::abs(bot.x - next_pod.x) < bot.sqr_area && std::abs(bot.y - next_pod.y) < bot.sqr_area) {
            // If the current pod is the last pod, go back to the first pod
            std::cout << "Player in pod " << curr_pod % 3 << std::endl;
            next_pod = params["pod" + std::to_string(++curr_pod % 3)];
            // Detect the area that is intersecting with the pod
            Area intersection;
            intersection.x = std::max(bot.x - bot.sqr_area, next_pod.x - next_pod.radius);
            intersection.y = std::max(bot.y - bot.sqr_area, next_pod.y - next_pod.radius);
            intersection.width = std::min(bot.x + bot.sqr_area, next_pod.x + next_pod.radius) - intersection.x;
            intersection.height = std::min(bot.y + bot.sqr_area, next_pod.y + next_pod.radius) - intersection.y;
            // Print the area
            std::cout << "Intersection: " << intersection.x << " " << intersection.y << " " << intersection.width << " " << intersection.height << std::endl;
        }

        // Move the player towards the next pod
        if (next_pod.x != 0 && next_pod.y != 0) {
            int distance_x = next_pod.x - bot.x;
            int distance_y = next_pod.y - bot.y;
            int distance = std::sqrt(distance_x * distance_x + distance_y * distance_y);
            int angle = std::atan2(distance_y, distance_x);
            bot.velocity_x = std::cos(angle) * distance / 10;
            bot.velocity_y = std::sin(angle) * distance / 10;
            // Turn the player towards the next pod
            bot.angle = angle;
            // Move towards the angle
            bot.x = bot.x + bot.velocity_x;
            bot.y = bot.y + bot.velocity_y;
        }

        Point bot_position{bot.x, bot.y};
        // If the points vector doesnt have a point at the current position, add it
        if (!std::any_of(points.begin(), points.end(), [bot](Point point) {
            return point.x == bot.x && point.y == bot.y;
        })) {
            points.push_back(bot_position);
        }

        total_points.push_back(bot_position);
    }

    // Print the points
    for (Point point : points) {
        std::cout << "Points captured: " << point.x << " " << point.y << std::endl;
    }

    if (args.find("--save") != args.end()) {
        std::ofstream file(args["--save"]);
        file << "{" << std::endl;
        file << "    \"points\": [" << std::endl;
        for (int i = 0; i < total_points.size(); i++) {
            Point point = total_points[i];
            file << "        {" << std::endl;
            file << "            \"x\": " << point.x << "," << std::endl;
            file << "            \"y\": " << point.y << std::endl;
            file << "        }";
            if (i != total_points.size() - 1) {
                file << ",";
            }
            file << std::endl;
        }
        file << "    ]," << std::endl;
        file << "    \"pods\": [" << std::endl;
        for (int i = 0; i < 3; i++) {
            Pod pod = params["pod" + std::to_string(i)];
            file << "        {" << std::endl;
            file << "            \"x\": " << pod.x << "," << std::endl;
            file << "            \"y\": " << pod.y << "," << std::endl;
            file << "            \"radius\": " << pod.radius << std::endl;
            file << "        }";
            if (i != 2) {
                file << ",";
            }
            file << std::endl;
        }
        file << "    ]," << std::endl;
        file << "    \"player\": {" << std::endl;
        file << "        \"sqr_area\": " << bot.sqr_area << std::endl;
        file << "    }" << std::endl;
        file << "}" << std::endl;
        file.close();
        std::cout << "Saved to " << args["--save"] << std::endl;
    }

    return 0;
}