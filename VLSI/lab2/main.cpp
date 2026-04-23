#include <cstdlib>
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <climits>
#include <map>
#include <algorithm>
using namespace std;

struct cdfg {
	int op;		// 1: ADD, 2: MUL
	int src1;
	int src2;
	int dst;
};

struct readylist {
	int state;	// 0=Not Ready, 1=Ready, 2=Done, 3=Doing
	int p;		// Priority
	int op;		// Operation type
};

void usage(const char** argv){
	cout << "usage: " << argv[0] << " [file]" << endl;
	exit(1);
}

void update_readylist(vector<readylist>& ready_list, const vector<cdfg>& cdfg_list){
    int rsize = ready_list.size();
    int csize = cdfg_list.size();

	for(int i=0; i<rsize; ++i){
		if(ready_list[i].state==0){
			ready_list[i].state = 1;	
		}
	}

	// Revert to unready if source dependencies are not met
	for(int i=0; i<csize; ++i){
		int src1 = cdfg_list[i].src1;
		int src2 = cdfg_list[i].src2;
		int dst  = cdfg_list[i].dst;
		
		if(!(src1<rsize && src2<rsize && dst<rsize)){
			cout << "ERROR: out of index in update_readylist(). ABORTED" << endl;
			exit(3);
		}
		
		// Skip done
		if(ready_list[dst].state==2){		
			continue;
		}

		// If source node is not done, the destination is not ready
		if(ready_list[src1].state!=2){		
			ready_list[dst].state = 0;
		}
		if(ready_list[src2].state!=2){
			ready_list[dst].state = 0;	
		}
	}
}

void longest_path(vector<readylist>& ready_list, const vector<cdfg>& cdfg_list, const int c_add, const int c_mul){
    int rsize = ready_list.size();
    int csize = cdfg_list.size();

    vector<int> out_degree(rsize, 0); 
    
    // Calculate out-degrees
    for(int i=0; i<csize; ++i){
        int src1 = cdfg_list[i].src1;
        int src2 = cdfg_list[i].src2;
        int dst  = cdfg_list[i].dst;
        
        if(!(src1<rsize && src2<rsize && dst<rsize)){
            cout << "ERROR: out of index in longest_path(). ABORTED" << endl;
            exit(6);
        }
        out_degree[src1]++;
        out_degree[src2]++;
    }

    queue<int> q;	
    vector<int> v;	
    
    for(int i=0; i<rsize; ++i){
        if(out_degree[i]==0){
            q.push(i);
            ready_list[i].p = (ready_list[i].op == 1) ? c_add : c_mul;
        }else{
            ready_list[i].p = 0;
        }
    }
    
    // Reverse topological sort
    while(!q.empty()){
        int current = q.front();
        q.pop();
        v.push_back(current);

        for(int i=0; i<csize; ++i){
            if(cdfg_list[i].dst == current){
                int src1 = cdfg_list[i].src1;
                int src2 = cdfg_list[i].src2;
                
                if(!(src1<rsize && src2<rsize && cdfg_list[i].dst<rsize)){
                    cout << "ERROR: out of index in longest_path(). ABORTED" << endl;
                    exit(7);
                }
                
                int delay1 = (ready_list[src1].op == 1) ? c_add : c_mul;
                int delay2 = (ready_list[src2].op == 1) ? c_add : c_mul;

                ready_list[src1].p = max(ready_list[src1].p, ready_list[current].p + delay1);
                ready_list[src2].p = max(ready_list[src2].p, ready_list[current].p + delay2);

                out_degree[src1]--;
                if(out_degree[src1] == 0){
                    q.push(src1);
                } else if(out_degree[src1] < 0){
                    cout << "ERROR: WTH outdegree smaller than 0. in longest_path(). ABORTED" << endl;
                    exit(8);
                }
                
                out_degree[src2]--;
                if(out_degree[src2] == 0){ 
                    q.push(src2);
                } else if(out_degree[src2] < 0){
                    cout << "ERROR: WTH outdegree smaller than 0. in longest_path(). ABORTED" << endl;
                    exit(8);
                }
            }
        }
    }
    if(v.size() < rsize){	
        cout << "ERROR: input file may contain a Cycle. IT SHOULD BE A DAG. in longest_path() ABORTED" << endl;
        exit(9);
    }else if(v.size() > rsize){
        cout << "ERROR: WTH. v.size() > rsize ????. in longest_path. ABORTED" << endl;
        exit(10);
    }
}

void init_readylist(vector<readylist>& ready_list, const vector<cdfg>& cdfg_list, const int c_add, const int c_mul){	
    int rsize = ready_list.size();
    int csize = cdfg_list.size();

	for(int i=0; i<rsize; ++i){
		ready_list[i].state = 0;	
		ready_list[i].op = 0;
	}
	
	// Fill in operation types
	for(int i=0; i<csize; ++i){
		int dst = cdfg_list[i].dst;
		if(!(dst<rsize)){
			cout << "ERROR: out of index in init_readlist(). ABORTED" << endl;
			exit(4);
		}
		ready_list[dst].op = cdfg_list[i].op;
		if(!(ready_list[dst].op == 1 || ready_list[dst].op==2)){
			cout << "ERROR: Undefined Operation " << ready_list[dst].op << " in init_readylist(). ABORTED" << endl;
			exit(13);
		}
	}
	
	// make pure input nodes Done
	for(int i=0; i<rsize; ++i){
		ready_list[i].state = 2;
	}
	for(int i=0; i<csize; ++i){
		int dst = cdfg_list[i].dst;
		ready_list[dst].state = 0;
	}
	
	longest_path(ready_list, cdfg_list, c_add, c_mul); // Calculate priorities
	update_readylist(ready_list, cdfg_list);	       // Unlock initial ready nodes
}

int main(const int argc, const char** argv){
	if(argc!=2){
		usage(argv);
	}
	
	// ----------------- input -----------------
	int n_add = 0; // Number of adders
	int n_mul = 0; // Number of multipliers
	int c_add = 0; // Cycles needed for one ADD
	int c_mul = 0; // Cycles needed for one MUL
	cout << "enter #add: ";
	cin >> n_add;
	cout << "enter #cycle add need: ";
	cin >> c_add;
	cout << "enter #mul: ";
	cin >> n_mul;
	cout << "enter #cycle mul need: ";
	cin >> c_mul;

	ifstream in(argv[1]);
	if(!in.is_open()){
		cout << "Open " << argv[1] << " failed" << endl;
		exit(2);
	}
	
	cout << "Construct CDFG starts..." << endl;
	int n_cdfg = 0;
	int op;			
	int src1, src2, dst;	
	in >> n_cdfg; // Total operations
    
	vector<cdfg> cdfg_list(n_cdfg);	
    
	// ----------------- Remapping -----------------
	// map raw IDs to continuous IDs
	map<int, int> mp;  // Raw ID -> Seq ID
	map<int, int> rmp; // Seq ID -> Raw ID
	for(int i=0; in >> op >> src1 >> src2 >> dst; ++i){	
		if(mp.count(src1)!=0){
			src1 = mp[src1];
		}else{
            int current_id = mp.size();
			rmp.insert(make_pair(current_id, src1));
			mp.insert(make_pair(src1, current_id));
			src1 = current_id;
		}
		if(mp.count(src2)!=0){
			src2 = mp[src2];
		}else{
            int current_id = mp.size();
			rmp.insert(make_pair(current_id, src2));
			mp.insert(make_pair(src2, current_id));
			src2 = current_id;
		}
		if(mp.count(dst)!=0){
			dst = mp[dst];
		}else{
            int current_id = mp.size();
			rmp.insert(make_pair(current_id, dst));
			mp.insert(make_pair(dst, current_id));
			dst = current_id;
		}
		cdfg_list[i].op = op;
		cdfg_list[i].src1 = src1;
		cdfg_list[i].src2 = src2;
		cdfg_list[i].dst = dst;
	}
	int n_readylist = mp.size(); // Total nodes
	in.close();
	cout << "Construct CDFG Done." << endl;
	
	// ----------------- ReadyList -----------------
	cout << "Construct Ready List..." << endl;
	vector<readylist> ready_list(n_readylist);
	init_readylist(ready_list, cdfg_list, c_add, c_mul);
	cout << "Construct Ready List Done." << endl;
	
	cout << "List Scheduling Starts: " << endl;

	// ----------------- Scheduling -----------------
	bool is_done = false;
	int cstep = 0;
	vector<pair<int,int>> add_doing; 
	vector<pair<int,int>> mul_doing;
	while(!is_done){
		++cstep;
		cout << "------------" << endl;
		cout << "time " << cstep << endl;

		vector<pair<int,int>> add_jobs = add_doing;
		vector<pair<int,int>> mul_jobs = mul_doing;
		cout << "Ready/Doing List: ";
		vector<int> add_ready, mul_ready;
		// Find all Ready tasks
		for(int i=0; i<n_readylist; ++i){		
			if(ready_list[i].state != 1){
				continue;
			}else if(ready_list[i].state == 3){
				cout << "D" << rmp[i] << " ";
				continue;
			}
			cout << "R" << rmp[i] << " ";
			if(ready_list[i].op == 1){	
				add_ready.push_back(i);
			}else if(ready_list[i].op == 2){
				mul_ready.push_back(i);
			}else{
				cout << "ERROR: undefined op type. in main(). ABORTED" << endl;
				cout << i << " " << ready_list[i].op << endl;
				exit(12);
			}
		}
		cout << endl;

		// Sort ready tasks by priority
		auto cmp = [&](int s, int t) { 
			return ready_list[s].p > ready_list[t].p; 
		};
		sort(add_ready.begin(), add_ready.end(), cmp);
		sort(mul_ready.begin(), mul_ready.end(), cmp);
		// Allocate tasks to available ALU
		int add_remain = n_add - add_doing.size();
		for(int i = 0; i < min((int)add_ready.size(), add_remain); ++i){
			add_jobs.push_back(make_pair(add_ready[i], c_add));
		}
		int mul_remain = n_mul - mul_doing.size();
		for(int i = 0; i < min((int)mul_ready.size(), mul_remain); ++i){
			mul_jobs.push_back(make_pair(mul_ready[i], c_mul));
		}
		add_doing.clear();
		mul_doing.clear();

		// Output
		cout << "ADD Executing: ";
		for(const auto t:add_jobs){
			int k = t.first;
			cout << "(v" << rmp[k] << "," << t.second << ") ";
			if(t.second-1>0){ 
				add_doing.push_back(make_pair(t.first, t.second-1));
			}
		}
		cout << endl;
		cout << "MUL Executing: ";
		for(const auto t:mul_jobs){
			int k = t.first;
			cout << "(v" << rmp[k] <<  "," << t.second << ") ";
			if(t.second-1>0){
				mul_doing.push_back(make_pair(t.first, t.second-1));
			}
		}
		cout << endl;

		// Update states
		for(const auto t:add_jobs){
			int k = t.first;
			ready_list[k].state = 3;
			if(t.second-1==0){
				ready_list[k].state = 2; // Finished
			}
		}
		for(const auto t:mul_jobs){
			int k = t.first;
			ready_list[k].state = 3;
			if(t.second-1==0){
				ready_list[k].state = 2; // Finished
			}
		}
		update_readylist(ready_list, cdfg_list);
		
		// Termination condition
		is_done = true;
		for(int i=0;i<n_readylist;++i){
			if(ready_list[i].state != 2){
				is_done = false;
			}
		}
	}
	cout << "------------" << endl;
	cout << cstep << " clock cycles in total" << endl;

	return 0;
}