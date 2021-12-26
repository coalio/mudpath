class Point {
    public:
        Point() {}
        Point(float _x, float _y) : x(_x), y(_y) {}

        float x;
        float y;
        Point closest(Point a, Point b);
};

Point Point::closest(Point a, Point b) {
    float da = b.y - a.y;
    float db = a.x - b.x;
    float c1 = da * a.x + db * a.y;
    float c2 = -db * this->x + da * this->y;
    float det = da * da + db * db;
    float cx = 0;
    float cy = 0;

    if (det != 0) {
        cx = (da*c1 - db*c2) / det;
        cy = (da*c2 + db*c1) / det;
    } else {
        // The point is already on the line
        cx = this->x;
        cy = this->y;
    }

    return Point(cx, cy);
}

struct Circle {
    Point center;
    int radius;
};

class Checkpoint {
    public:
        Checkpoint(int _x, int _y)
            : x(_x), y(_y) {}

        int x;
        int y;
        static const int radius = 600;
};

// Quick binary search implementation
int binary_search(
    float a[], int item,
    int low, int high
);

int binary_search(
    float a[], int item,
    int low, int high
) {
    if (high <= low) {
        return
            (item > a[low]) ?
            (low + 1) : low;
    }

    int mid = (low + high) / 2;

    if (item == a[mid]) {
        return mid + 1;
    }

    if (item > a[mid]) {
        return binary_search(
            a, item,
            mid + 1, high
        );
    }

    return binary_search(
        a, item,
        low, mid - 1
    );
}
