#include <cmath>
#include <vector>
#include "ElasticNet.h"
#include <iostream>
#include <string.h>
#include <set>
#include <algorithm>


namespace ElasticNet{
    int run_times, iteration, evaluation_max, eval_count;
    int K_iter;
    double alpha, beta, K, K_prod;
    int N, M;

    std::vector<Point> x;
    std::vector<Point> y;
    std::vector<std::vector<double> > w;    // for wij


    void Initialization(const std::vector<Point>& points){
        // ------------------------- Set parameters -------------------------
        // Total number of algorithm runs
        run_times = 1;
        // Maximum iterations per run
        iteration = 10000;
        // Maximum evaluation times per run
        evaluation_max = 10000*points.size();
        eval_count = 0;
        // number of cities
        N = points.size();
        // number of points on the elastic net
        M = 2.5 * N;    // 論文上 是這樣設定的
        //
        alpha = 0.2;
        beta = 2.0;
        K = 0.2;
        K_iter = N; // K: N iter to lower
        K_prod = 0.99; // K: K*=K_prod every time to lower


        // -------------- 正規化 X --------------
        // 把點都縮到 [0,1] x [0,1] 的範圍內
        double max_x = -1.0/0.0, max_y = -1.0/0.0;
        double min_x = 1.0/0.0, min_y = 1.0/0.0;
        for(const auto& k:points){
            max_x = std::max(max_x, k.x);
            max_y = std::max(max_y, k.y);
            min_x = std::min(min_x, k.x);
            min_y = std::min(min_y, k.y);
        }
        double nor_de = std::max(max_x-min_x, max_y-min_y); 

        x = points;
        for(int i=0;i<N;++i){
            x.at(i).x = (x.at(i).x-min_x)/nor_de;
            x.at(i).y = (x.at(i).y-min_y)/nor_de;
        }

        // -------------- y 用圓形分佈 --------------
        y.clear();
        double radius = 0.1;        // 半徑
        Point center;
        center.x = center.y = 0.5;
        double detla_angle = 2*M_PI/M;
        for(int i=0;i<M;++i){
            double angle = detla_angle*i;
            double delta_x = radius*cos(angle);
            double delta_y = radius*sin(angle);
            Point p = center;
            p.x += delta_x;
            p.y += delta_y;
            y.push_back(p);
        }

        // -------------- w init to 0 --------------
        for(int i=0;i<N;++i){
            std::vector<double> temp;
            for(int j=0;j<M;++j){
                temp.push_back(0);
            }
            w.push_back(temp);
        }
    }

    double ecuild_distance(const Point& A, const Point& B){
        return sqrt(pow(A.x-B.x, 2) + pow(A.y-B.y, 2));
    }

    // gaussian function
    double phi(double d){
        return exp((-d*d)/(2*K*K));
    }

    void Compute_weight(){
        for(int i=0;i<N;++i){
            double de = 0;
            for(int k=0;k<M;++k){
                de += phi(ecuild_distance(x[i], y[k]));
            }
            if(de==0){
                throw std::runtime_error("In Compute_weight: divide by zero");
            }
            for(int j=0;j<M;++j){
                double nu = phi(ecuild_distance(x[i], y[j]));
                w.at(i).at(j) = nu/de;
            }
        }
    }

    void Update(){
        Compute_weight();
        for(int j=0;j<M;++j){
            Point delta;

            // left part of formula
            delta.x = delta.y = 0;
            for(int i=0;i<N;++i){
                delta.x += w.at(i).at(j) *(x[i].x-y[j].x);
                delta.y += w.at(i).at(j) *(x[i].y-y[j].y);
            }
            delta.x *= alpha;
            delta.y *= alpha;

            // right part of formula
            int idx1 = j+1 % M;
            int idx2 = (j+M-1) % M;   // 哭阿 C 的 -1 % 2 == -1
            delta.x += beta*K*(y[idx1].x+y[idx2].x-2*y[j].x);
            delta.y += beta*K*(y[idx1].y+y[idx2].y-2*y[j].y);

            // apply cahnge
            y[j].x += delta.x;
            y[j].y += delta.y;
        }
    }

    void Attach(std::vector<int>& path, double& path_dist, const std::vector<Point>& points){     // attach
        std::set<int> ys;
        for(int j=0;j<M;++j){
            ys.insert(j);
        }
        // map idx between x and y;
        std::vector<std::pair<int, int> > xy;       // map idx between x and y;
        for(int i=0;i<N;++i){
            int cloest_idx = -1;
            double cloest_dist = 1.0/0.0;
            for(const auto& k:ys){
                double temp = ecuild_distance(x[i], y[k]);
                if(temp<cloest_dist){
                    cloest_dist = temp;
                    cloest_idx = k;
                }
            }
            ys.erase(cloest_idx);
            xy.push_back(std::make_pair(i, cloest_idx));
        }
        sort(xy.begin(), xy.end(), [](const std::pair<int, int>& s, const std::pair<int, int> t){return s.second<t.second;});

        // append to path & calculate distance
        // x is isomorphism to points
        path_dist = 0;
        path.push_back(xy.at(0).first); 
        for(int i=1;i<N;++i){
            const Point& from = points.at(xy.at(i-1).first);
            const Point& to   = points.at(xy.at(i).first);
            path_dist += ecuild_distance(from, to);
            path.push_back(xy.at(i).first);
        }
        const Point& from = points.at(xy.at(path.size()-1).first);
        const Point& to   = points.at(xy.at(0).first);
        path_dist += ecuild_distance(from, to);
        path.push_back(path.at(0));
    }

    
    void plot(const char* title, const int run, const int iter){
        // ------------------------ open pipe with gnuplot ------------------------
        FILE *gp = NULL;
        // popen: fork() and pipe()
        // 透過 pipe 跟 gnuplot 打交道
        if((gp = popen("gnuplot -p", "w")) == NULL){
            fprintf(stderr, "[plot] Error: popen - %s\n", strerror(errno));
            exit(errno);
        }
        
        // ------------------------ plot ------------------------
        fprintf(gp, "set terminal push\n");     // 儲存當前設定
        fprintf(gp, "set terminal pngcairo size 600, 600\n");
        char fig_path[256];
        sprintf(fig_path, "./output/%s_%02d_%05d.png", title, run, iter);
        fprintf(gp, "set output '%s'\n", fig_path);
        fprintf(gp, "set title '%s'\n", title);
        fprintf(gp, "set xlabel 'x軸'\n");
        fprintf(gp, "set ylabel 'y軸'\n");
        fprintf(gp, "set xrange [0:1]\n");
        fprintf(gp, "set yrange [0:1]\n");

        fprintf(gp, "plot '-' with points title 'x', '-' with linespoints title 'y'\n");   // 下了這個指令後 下面就要輸入數據
        for(const auto& k:x){
            fprintf(gp, "%lf %lf\n", k.x, k.y);
        }
        fprintf(gp, "e\n"); // 資料x結束
        for(const auto& k:y){
            fprintf(gp, "%lf %lf\n", k.x, k.y);
        }
        fprintf(gp, "%lf %lf\n", y[0].x, y[0].y);
        fprintf(gp, "e\n"); // 資料y結束

        fprintf(gp, "unset output\n");
        // ------------------------ flush and close ------------------------
        fflush(gp);     // flush buffer
        pclose(gp);
        return;
    }

    ElasticNet_RET ElasticNet(const std::vector<Point>& points){
        ElasticNet_RET ret;
        ret.shortest_dist = 1.0/0.0;

        run_times = 30;
        for(int r=0; r<run_times; ++r){
            Initialization(points);
            for(int i=0;i<iteration && eval_count<evaluation_max; ++i){
                if(i%100==0){
                    std::cout << "[Round: " << r << "/" << run_times << ", iter: " << i << "/" << iteration << "] " << ret.shortest_dist << " " << K << std::endl;
                    plot("TSP", r, i);
                }
                Update();
                if(i%K_iter==K_iter-1){     // to lower K
                    K *= K_prod;
                }
                ++eval_count;
            }
            std::vector<int> path;
            double path_dist = 0;
            Attach(path, path_dist, points);
            if(path_dist<ret.shortest_dist){
                ret.shortest_dist = path_dist;
                ret.shortest_path = path;
            }
        }

        return ret;
    }

}