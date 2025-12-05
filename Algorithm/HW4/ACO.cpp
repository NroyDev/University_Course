#include <set>
#include <cmath>
#include <algorithm>
#include "ACO.h"

#include <iostream>
#include <stdio.h>
#include <unistd.h>
// Algorithm 1 The Ant Colony Optimization Metaheuristic
// -----------------------------------------------------
//  Set parameters, initialize pheromone trails
//  while termination condition not met do
//      𝐶𝑜𝑛𝑠𝑡𝑟𝑢𝑐𝑡𝐴𝑛𝑡𝑆𝑜𝑙𝑢𝑡𝑖𝑜𝑛𝑠
//      𝐴𝑝𝑝𝑙𝑦𝐿𝑜𝑐𝑎𝑙𝑆𝑒𝑎𝑟𝑐ℎ (𝑜𝑝𝑡𝑖𝑜𝑛𝑎𝑙)
//      𝑈𝑝𝑑𝑎𝑡𝑒𝑃ℎ𝑒𝑟𝑜𝑚𝑜𝑛𝑒𝑠
//  endwhile


// HW4 - Ant colony optimization (9/11)
// 1: for 0 ≤ r < run_times
// 2:   Initialization()
// 3:   for 0 ≤ i < iterations
// 4:       for 0 ≤ j < population_size
// 5:           ConstructAntSolution()
// 6:           Evaluation() // eval_count ++
// 7:       Update_pheromones()


namespace ACO{
    // parameter ppt defined
    int run_times, iteration, population_size, evaluation_max, eval_count;
    double alpha, beta, rho, Q;
    // other variable
    int size;
    double** tau = nullptr;
    double** dist = nullptr;

    void Initialization(const std::vector<Point>& points){
        // ------------------------- Set parameters -------------------------
        // Total number of algorithm runs
        run_times = 30;
        // Maximum iterations per run
        iteration = 1000;
        // Number of ants (Population size)
        population_size = points.size();
        // Pheromone importance factor
        alpha = 1;
        // Heuristic Factor (1/distance)
        beta = 5;
        // Pheromone evaporation rate
        rho = 0.5;
        // Constant
        Q = 1;
        // Maximum evaluation times per run
        evaluation_max = 10000*points.size();
        eval_count = 0;

        // my constant
        size = points.size();
        double C = dist[size-1][0];
        for(int i=1;i<size;++i){
            C += dist[i-1][i];
        }
        C = Q/C;

        // ------------------------- initialize pheromone trails table -------------------------
        for(int i=0;i<size;++i){
            for(int j=0;j<size;++j){
                tau[i][j] = C;
            }
        }
    }

    int randSelect(const std::vector<std::pair<int, double> >& P){  // P : <idx, probility>
        std::vector<double> CP;
        CP.push_back(0);
        for(const auto& k:P){
            CP.push_back(k.second+CP.back());
        }

        if(CP.back() <= 0){
            std::cout << P.size() << std::endl;
            for(const auto& k:P){
                std::cout << "("<<k.first<<","<<k.second<<")\t";
            }
            std::cout << std::endl;
            throw std::runtime_error("In randSelect: No elements in P is larger than zero");
        }

        double r = rand();
        r /= RAND_MAX;
        auto it = std::upper_bound(CP.begin(), CP.end(), r);
        int idx = it-CP.begin()-1;

        if(idx==(int)P.size()){ // fix precision error
            --idx;
        }
        return P.at(idx).first;
    }

    void ConstructAntSolution(std::vector<int>& ant_path, double& path_dist, int ant_id){
        path_dist = 0;
        std::set<int> non_visited;
        for(int i=0;i<size;++i){
            non_visited.insert(i);
        }

        int i = ant_id % size;          // 起點
        ant_path.push_back(i);
        non_visited.erase(i);
        while(!non_visited.empty()){
            // 先計算出 機律 分母的部份
            double de = 0;
            for(const auto& l:non_visited){
                de += pow(tau[i][l], alpha) * pow(1/dist[i][l], beta);
            }
            if(de==0){
                throw std::runtime_error("In ConstructAntSolution: divide zero. (de)");
            }

            std::vector<std::pair<int, double> > P;     // 用來存每個可走的 city 接下來走過去的機率為何，格式 (可走City, 機率)
            double nu = 1;                              // 每個 city 機率分子部份
            for(const auto& j:non_visited){
                nu = pow(tau[i][j], alpha) * pow(1/dist[i][j], beta);
                P.push_back(std::make_pair(j, nu/de));
            }

            int next = randSelect(P);                   // 透過輪盤法選到的 city
            ant_path.push_back(next);
            non_visited.erase(next);
            path_dist += dist[i][next];

            i = next;
        }
        path_dist += dist[i][ant_path[0]];
        ant_path.push_back(ant_path[0]);
    }

    void Update_pheromones(std::vector<int>* ant_paths, std::vector<double>& L){
        for(int i=0;i<size;++i){
            for(int j=0;j<size;++j){
                tau[i][j] = (1-rho)*tau[i][j];
                for(int m=0;m<population_size;++m){
                    tau[i][j] += Q/L[m];
                }
            }
        }
    }

    void Allocate(const std::vector<Point>& points){
        int size = points.size();

        // allocate space
        dist = new double*[size];
        tau = new double*[size];
        for(int i = 0; i < size; ++i){
            tau[i] = new double[size];
            dist[i] = new double[size];
            dist[i][i] = 0;
        }
        

        // init dist
        for(int i=0;i<size;++i){
            const Point& from = points[i];
            for(int j=i+1;j<size;++j){
                const Point& to = points[j];
                dist[i][j] = dist[j][i] = sqrt((from.x-to.x)*(from.x-to.x) + (from.y-to.y)*(from.y-to.y));
            }
        }
    }

    void DisAllocate(){
        for(int i=0;i<size;++i){
            delete[] tau[i];
            tau[i] = nullptr;
            delete[] dist[i];
            dist[i] = nullptr;
        }
        delete[] tau;
        delete[] dist;

        tau   = nullptr;
        dist    = nullptr;
    }

    ACO_RET ACO(const std::vector<Point>& points){
        ACO_RET ret;
        ret.shortest_dist = 1.0/0.0;
        ret.mean_dist = 0;

        srand(time(NULL));
        Allocate(points);
        // =====================
        // for 0 ≤ r < run_times
        // =====================
        run_times = 1;
        for(int r=0; r<run_times; ++r){
            // ==============
            // Initialization
            // ==============
            Initialization(points);
            double shortest_dist_thisrun = 1.0/0.0;             // 用來存這次 run 跑出的最短路徑
            std::vector<int> shortest_path_thisrun;           // 用來存這次 run 跑出的最短路徑

            // ======================
            // for 0 ≤ i < iterations
            // ======================
            for(int i=0;i<iteration && eval_count<evaluation_max; ++i){
                if(i%100==0){
                    std::cout << "[Round: " << r << "/" << run_times << ", iter: " << i << "/" << iteration << "] " << ret.shortest_dist << std::endl;
                }
                std::vector<int> ant_paths[population_size];    // 用來存每隻螞蟻的路徑
                std::vector<double> ant_dist;                   // 用來存每支螞蟻路徑長 
                // ===========================
                // for 0 ≤ j < population_size
                // ===========================
                for(int j=0;j<population_size && eval_count<evaluation_max; ++j){
                    double path_dist = 0;
                    // ====================
                    // ConstructAntSolution
                    // ====================
                    ConstructAntSolution(ant_paths[j], path_dist, j);

                    // ==========
                    // Evaluation
                    // ==========
                    ant_dist.push_back(path_dist);
                    if(path_dist<shortest_dist_thisrun){
                        shortest_path_thisrun = ant_paths[j];
                        shortest_dist_thisrun = path_dist;
                    }
                    ++eval_count;
                }
                // =================
                // Update_pheromones
                // =================
                Update_pheromones(ant_paths, ant_dist);
            }

            // 在每個 round 結束後，這次Run跑出的path 看看是否更短，並更新mean dist的資料
            // ==========
            // Update RET
            // ==========
            if(shortest_dist_thisrun < ret.shortest_dist){
                ret.shortest_dist = shortest_dist_thisrun;
                ret.shortest_path = shortest_path_thisrun;
            }
            ret.mean_dist += shortest_dist_thisrun/(double)run_times;
        }
        DisAllocate();

        return ret;
    }

}