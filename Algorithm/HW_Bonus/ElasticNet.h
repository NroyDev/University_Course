#ifndef ElasticNet_H
#define ElasticNet_H

#include <vector>

struct Point{
    int city;
    double x;
    double y;
};

struct ElasticNet_RET{
    std::vector<int> shortest_path;
    double shortest_dist;
};

namespace ElasticNet{
    ElasticNet_RET ElasticNet(const std::vector<Point>& points);
}
#endif