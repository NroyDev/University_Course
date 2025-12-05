#ifndef ACO_H
#define ACO_H

#include <vector>

struct Point{
    int city;
    double x;
    double y;
};

struct ACO_RET{
    std::vector<int> shortest_path;
    double shortest_dist;
    double mean_dist;
};

namespace ACO{
    ACO_RET ACO(const std::vector<Point>& points);
}
#endif