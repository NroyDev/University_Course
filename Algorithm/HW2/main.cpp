#include<iostream>
#include<fstream>
#include<cmath>
#include<vector>
#include<algorithm>

// -- for gnuplot --
#include<cstdio>
#include<cstdlib>
#include<cerrno>
#include<cstring>

const char* ans_path = "./ans.txt";
const char* fig_path = "./fig.png";
const char* data_path = "./points.txt";

struct Point{
    long long city;
    double x;
    double y;
};

void argvParser(int argc,const char** argv);
void plot(const char*, const std::vector<Point>&, const std::vector<long long>&);
void usage(const char** argv);

int main(int argc, const char** argv){
    argvParser(argc, argv);

    // ----------- open file -----------
    std::ifstream in;
    in.open(data_path,std::ifstream::in);
    if(!in.is_open()){
        std::cout << "Error: open " << data_path << " failed" << std::endl;
        exit(2);
    }
    std::ofstream out;
    out.open(ans_path, std::ofstream::out | std::ofstream::trunc);
    if(!out.is_open()){
        std::cout << "Error: open " << ans_path << " failed" << std::endl;
        exit(3);
    }

    // ----------- input -----------
    long long city;
    double x, y;
    std::vector<Point> points;
    while(in >> city >> x >> y){
        Point p;
        p.city  = city;
        p.x     = x;
        p.y     = y;
        points.push_back(p);
    }
    std::vector<long long> idxs;
    for(long long i=0; i<(long long)points.size(); ++i){
        idxs.push_back(i);
    }
    // pre calculate distance
    double** dist = new double*[points.size()];   // dist[a][b] = distance from a to b
    for(long long i=0;i<(long long)points.size();++i){
        dist[i] = new double[points.size()];
        dist[i][i] = 0;
    }
    for(long long i=0;i<(long long)points.size();++i){
        Point from = points[i];
        for(long long j=i+1;j<(long long)points.size();++j){
            Point to   = points[j];
            dist[i][j] = dist[j][i] = sqrt((from.x-to.x)*(from.x-to.x) + (from.y-to.y)*(from.y-to.y));
        }
    }

    // ----------- Greedy -----------
    double shortest_dist = 0;
    std::vector<long long> shortest_path;
    shortest_path.push_back(0);
    long long from = 0;
    idxs.erase(idxs.begin());
    while(!idxs.empty()){
        auto next = idxs.begin();
        const long long to = *next;
        // double next_dist = sqrt((to.x-from.x)*(to.x-from.x) + (to.y-from.y)*(to.y-from.y)); //到下個點最短距離;
        double next_dist = dist[from][to]; //到下個點最短距離;
        for(auto it = idxs.begin()+1; it!=idxs.end(); ++it){
            const long long to = *it;
            // double dist = sqrt((to.x-from.x)*(to.x-from.x) + (to.y-from.y)*(to.y-from.y));
            double distance = dist[from][to];
            if(distance < next_dist){
                next = it;
                next_dist = distance;
            }
        }
        shortest_dist += next_dist;
        shortest_path.push_back(*next);
        from = *next;
        idxs.erase(next);
    }
    long long to = 0;
    // shortest_dist += sqrt((to.x-from.x)*(to.x-from.x) + (to.y-from.y)*(to.y-from.y));
    shortest_dist += dist[from][to];
    // 回收空間
    for(long long i=0;i<(long long)points.size();++i){
        delete[] dist[i];
    }
    delete[] dist;

    // ----------- print ans -----------
    out << "distance: " << shortest_dist << std::endl;
    for(const auto& k:shortest_path){
        out << points[k].city << std::endl;
    }
    in.close();
    out.close();

    plot("Result", points, shortest_path);
    return 0;
}

// parse the argv and set the fig_path ans_path data_path
void argvParser(int argc,const char** argv){
    if(argc < 2){
        usage(argv);
        exit(1);
    }else if(argc == 2){
        data_path = argv[1];


    }else if(argc == 4){
        data_path = argv[3];
        if(strcmp(argv[1], "-oi") == 0){
            fig_path = argv[2];
        }else if(strcmp(argv[1], "-ot") == 0){
            ans_path = argv[2];
        }


    }else if(argc == 6){
        data_path = argv[5];
        if(strcmp(argv[1], "-oi") == 0){
            fig_path = argv[2];
        }else if(strcmp(argv[1], "-ot") == 0){
            ans_path = argv[2];
        }else{
            usage(argv);
            exit(1);
        }

        if(strcmp(argv[3], "-oi") == 0 && strcmp(argv[1], "-oi") != 0){
            fig_path = argv[4];
        }else if(strcmp(argv[3], "-ot") == 0 && strcmp(argv[1], "-ot") != 0){
            ans_path = argv[4];
        }else{
            usage(argv);
            exit(1);
        }
    }else{
        usage(argv);
        exit(1);
    }
    
}

void plot(const char* title, const std::vector<Point>& points, const std::vector<long long>& order){
    if(order.size() <= 0 || order.size() != points.size()){
        fprintf(stderr, "[plot] Error: size not match\n");
        exit(-1);
    }
    
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
    fprintf(gp, "set output '%s'\n", fig_path);
    fprintf(gp, "set title '%s'\n", title);
    fprintf(gp, "set xlabel 'x軸'\n");
    fprintf(gp, "set ylabel 'y軸'\n");

    fprintf(gp, "plot '-' with linespoints title '%s'\n", "TSP Path");   // 下了這個指令後 下面就要輸入數據
    
    for(const auto& k:order){
        const Point& current = points[k];
        fprintf(gp, "%lf %lf\n", current.x, current.y); 
    }
    if(!order.empty() && !points.empty()){
        const Point& current = points[order[0]];
        fprintf(gp, "%lf %lf\n", current.x, current.y);
    }

    fprintf(gp, "e\n"); // 資料結束

    fprintf(gp, "unset output\n");
    // 上面的存png 下面的跳出視窗
    fprintf(gp, "set terminal wxt size 600, 600\n");
    fprintf(gp, "replot\n");    // 剛剛plot的資料直接用來在畫一遍
    fprintf(gp, "unset terminal\n");

    // ------------------------ flush and close ------------------------
    fflush(gp);     // flush buffer
    pclose(gp);
    return;
}

void usage(const char** argv){
    std::cout << "用法: " << argv[0] << " [input data path]" << std::endl;
    std::cout << "用法: " << argv[0] << " -oi [output image path] [input data path]" << std::endl;
    std::cout << "用法: " << argv[0] << " -ot [output txt path] [input data file]" << std::endl;
    std::cout << "用法: " << argv[0] << " -oi [output image path] -ot [output txt path] [input data file]" << std::endl;
    std::cout << "用法: " << argv[0] << " -ot [output txt path] -oi [output image path] [input data file]" << std::endl;
    return;
}
