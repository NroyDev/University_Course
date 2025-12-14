#ifndef ACO_H
#define ACO_H

#include <vector>

struct Point{
    int city;
    double x;
    double y;
};

struct ACO_HYPERPARAM{
    int run_times;
    int evaluation_max;
    int population_size;
    double alpha;
    double beta;
    double rho;
    double Q;
};

struct ACO_RET{
    std::vector<int> shortest_path;
    double shortest_dist;
    double mean_dist;
};

namespace ACO{
    ACO_RET ACO(const std::vector<Point>& points, ACO_HYPERPARAM* hyper_param);
}
#endif