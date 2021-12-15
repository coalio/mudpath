struct Point {
    int x;
    int y;
};

struct Circle {
    Point center;
    int radius;
};

struct Checkpoint {
    int x;
    int y;
    static const int radius = 600;
};
