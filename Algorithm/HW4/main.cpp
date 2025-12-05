#include<iostream>
#include<fstream>
#include<cmath>
#include<vector>
#include<set>

#include "ACO.h"
// -- for gnuplot --
#include<cstdio>
#include<cstdlib>
#include<cerrno>
#include<cstring>

const char* ans_path = "./ans.txt";
const char* fig_path = "./fig.png";
const char* data_path = "./points.txt";


void argvParser(int argc,const char** argv);
void plot(const char*, const std::vector<Point>&, const std::vector<int>&);
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
    int city;
    double x, y;
    std::vector<Point> points;
    while(in >> city >> x >> y){
        Point p;
        p.city  = city;
        p.x     = x;
        p.y     = y;
        points.push_back(p);
    }

    // ----------- Ant colony optimization -----------
    ACO_RET ret = ACO::ACO(points);
    ret.shortest_path.pop_back();

    // ----------- print ans -----------
    out << "mean distance: " << ret.mean_dist << std::endl;
    out << "distance: " << ret.shortest_dist << std::endl;
    for(const auto& k:ret.shortest_path){
        out << points[k].city << std::endl;
    }
    in.close();
    out.close();

    plot("Result", points, ret.shortest_path);
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

void plot(const char* title, const std::vector<Point>& points, const std::vector<int>& order){
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