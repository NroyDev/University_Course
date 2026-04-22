#include<cstdlib>
#include<iostream>
#include<fstream>
#include<queue>
#include<vector>
#include<climits>
using namespace std;

struct cdfg{
	int op;
	int src1;
	int src2;
	int dst;
};

struct readylist{
	int state;	// 0: not ready, 1: ready, 2: complete
	int p;		// the priority that breaks the tie (longest path)
	int op;		// do what type operation to make it complete
};

void usage(const char** argv){
	cout << "usage: " << argv[0] << " [file]" << endl;
	exit(1);
}

void update_readylist(struct readylist* ready_list, const struct cdfg* cdfg, const int rsize, const int csize){
	// phase1: mark all unready as ready
	for(int i=0;i<rsize;++i){
		if(ready_list[i].state==0){
			ready_list[i].state = 1;	// mark as ready at first
		}
	}

	// phase2: unmark the unready ones
	for(int i=0;i<csize;++i){
		int src1 = cdfg[i].src1;
		int src2 = cdfg[i].src2;
		int dst = cdfg[i].dst;
		if(!(src1<rsize && src2<rsize && dst<rsize)){
			cout << "ERROR: out of index in update_readylist(). ABORTED" << endl;
			exit(3);
		}
		if(ready_list[dst].state==2){		// if the operation has been complete => ignore it
			continue;
		}

		if(ready_list[src1].state!=2){		// if its source hasn't been completed => it should be unready
			ready_list[dst].state = 0;
		}
		if(ready_list[src2].state!=2){
			ready_list[dst].state = 0;	// similar to the previous reason
		}
	}
}

void longest_path(struct readylist* ready_list, const struct cdfg* cdfg, const int rsize, const int csize){
	// use topo sort
	// by topo order, compute dist[v] = max(dist[v], dist[u]+w[u][v]);
	
	// topo sort
	int* in_degree = new int[rsize];
	for(int i=0;i<rsize;++i){
		in_degree[i] = 0;
	}
	for(int i=0;i<csize;++i){
		int src1 = cdfg[i].src1;
		int src2 = cdfg[i].src2;
		//int dst  = cdfg[i].dst;
		if(!(src1<rsize && src2<rsize)){
			cout << "ERROR: out of index in longest_path(). ABORTED" << endl;
			exit(6);
		}
		++in_degree[src1];
		++in_degree[src2];
	}

	queue<int> q;	// store current indegree = 0;
	vector<int> v;	// store the topo order (val: node ID AKA idx of ready_list)
	for(int i=0;i<rsize;++i){
		if(in_degree[i]==0){
			q.push(i);
			ready_list[i].p = 0;
		}else{
			ready_list[i].p = INT_MIN;
		}
	}
	while(!q.empty()){
		int current = q.front();
		q.pop();
		v.push_back(current);

		for(int i=0;i<csize;++i){
			int src1 = cdfg[i].src1;
			int src2 = cdfg[i].src2;
			int dst  = cdfg[i].dst;
			if(!(src1<rsize && src2<rsize && dst<rsize)){
				cout << "ERROR: out of index in longest_path(). ABORTED" << endl;
				exit(7);
			}
			if(dst==current){
				--in_degree[src1];
				--in_degree[src2];
				if(in_degree[src1]==0){
					q.push(src1);
				}else if(in_degree[src1]<0){
					cout << "ERROR: WTH indegree smaller than 0. in longest_path(). ABORTED" << endl;
					exit(8);
				}
				if(in_degree[src2]==0){
					q.push(src2);
				}else if(in_degree[src2]<0){
					cout << "ERROR: WTH indegree smaller than 0. in longest_path(). ABORTED" << endl;
					exit(8);
				}

			}
		}
	}
	if(v.size() < rsize){	// check
		cout << "ERROR: input file may contain a Cycle. IT SHOULD BE A DAG. in longest_path() ABORTED" << endl;
		exit(9);
	}else if(v.size() > rsize){
		cout << "ERROR: WTH. v.size() > rsize ????. in longest_path. ABORTED" << endl;
		exit(10);
	}

	// compute longest dist
	for(int i=0;i<rsize;++i){
		int current = v[i];
		for(int j=0;j<csize;++j){
			int src1 = cdfg[j].src1;
			int src2 = cdfg[j].src2;
			int dst  = cdfg[j].dst;
			if(!(src1<rsize && src2<rsize && dst<rsize)){
				cout << "ERROR: out of index in longest_path(). ABORTED" << endl;
				exit(11);
			}
			if(current == dst){
				ready_list[src1].p = max(ready_list[src1].p, ready_list[current].p+1);
				ready_list[src2].p = max(ready_list[src2].p, ready_list[current].p+1);
			}
		}
	}

	delete [] in_degree;
}

void init_readylist(struct readylist* ready_list, const struct cdfg* cdfg, const int rsize, const int csize){	
	// init
	for(int i=0;i<rsize;++i){
		// undefined state, used for check the input is vaild (ensure there's no unreferenced node)
		ready_list[i].state = -1;	// remember we used max to take n_readylist right?
		ready_list[i].op = 0;
	}
	// check there's no unreferenced node & fill in op type
	for(int i=0;i<csize;++i){
		int src1 = cdfg[i].src1;
		int src2 = cdfg[i].src2;
		int dst = cdfg[i].dst;
		if(!(src1<rsize && src2<rsize && dst<rsize)){
			cout << "ERROR: out of index in init_readlist(). ABORTED" << endl;
			//cout << src1 << " " << src2 << " " << dst << " " << rsize << " " << csize << endl;
			exit(4);
		}
		// mark referenced to defined state, undefined state stands for unreferenced node
		ready_list[src1].state = 0;
		ready_list[src2].state = 0;
		ready_list[dst].state = 0;
		// fill in op type
		ready_list[dst].op = cdfg[i].op;
		if(!(ready_list[dst].op == 1 || ready_list[dst].op==2)){
			cout << "ERR:" << endl;
			exit(13);
		}
	}
	for(int i=0;i<rsize;++i){
		if(ready_list[i].state != 0){	// if the node state not equl to zero, it means there's no referenced
			cout << "ERROR: input file has unreferenced node. in init_readlist(). ABORTED" << endl;
			exit(5);
		}
	}
	// ensure which completed
	for(int i=0;i<rsize;++i){
		ready_list[i].state = 2;
	}
	for(int i=0;i<csize;++i){
		int dst = cdfg[i].dst;
		ready_list[dst].state = 0;
	}
	
	// compute longest path for each node => priority
	longest_path(ready_list, cdfg, rsize, csize);
	update_readylist(ready_list, cdfg, rsize, csize);	
}

int main(const int argc, const char** argv){
	if(argc!=2){
		usage(argv);
	}
	ifstream in(argv[1]);
	if(!in.is_open()){
		cout << "Open " << argv[1] << " failed" << endl;
		exit(2);
	}
	
	// ------------ Construct CDFG ------------
	cout << "Construct CDFG starts..." << endl;
	int n_cdfg = 0;
	int op;			// operation type
	int src1, src2, dst;	// nodes
	while(in >> op >> src1 >> src2 >> dst){	// phase1: read how many operations
		++n_cdfg;
	}
	in.close();
	
	
	struct cdfg* cdfg = new struct cdfg[n_cdfg];	// the operations list
	int n_readylist = 0;
	in.open(argv[1]);
	if(!in.is_open()){
		cout << "Open " << argv[1] << " failed" << endl;
		exit(3);
	}
	for(int i=0;in >> op >> src1 >> src2 >> dst; ++i){	// phase2: store the operations to list
		cdfg[i].op = op;
		cdfg[i].src1 = src1-1;
		cdfg[i].src2 = src2-1;
		cdfg[i].dst = dst-1;

		n_readylist = max(n_readylist, src1);
		n_readylist = max(n_readylist, src2);
		n_readylist = max(n_readylist, dst);
	}
	in.close();
	cout << "Construct CDFG Done." << endl;
	
	// ------------ Construct Ready List ------------
	cout << "Construct Ready List..." << endl;
	struct readylist* ready_list = new struct readylist[n_readylist];
	init_readylist(ready_list, cdfg, n_readylist, n_cdfg);
	cout << "Construct Ready List Done." << endl;
	
	// ------------ List Scheduling ------------
	cout << "List Scheduling Starts: " << endl;
	int n_add = 0;
	int n_mul = 0;
	cout << "enter #add: ";
	cin >> n_add;
	cout << "enter #mul: ";
	cin >> n_mul;

	bool is_done = false;
	int cstep = 0;
	while(!is_done){
		++cstep;
		cout << "------------" << endl;
		cout << "time " << cstep << endl;

		vector<int> add_jobs;
		vector<int> mul_jobs;
		cout << "Ready List: {";
		for(int i=0;i<n_readylist;++i){		// take ready jobs
			if(ready_list[i].state != 1){	// skip unready or completed
				continue;
			}
			cout << "(" << i+1 << "," << ready_list[i].p << "," << ready_list[i].op << "), ";

			if(ready_list[i].op == 1){	// 1 for add
				if(add_jobs.size() < n_add){		// not full => add it
					add_jobs.push_back(i);
				}else{					// full => replace the lowest priority
					int min_idx = 0;
					for(int j=1;j<n_add;++j){
						if(ready_list[add_jobs[min_idx]].p > ready_list[add_jobs[j]].p){
							min_idx = j;
						}
					}
					if(ready_list[add_jobs[min_idx]].p < ready_list[i].p){
						add_jobs[min_idx] = i;
					}
				}

			}else if(ready_list[i].op == 2){// 2 for mul
				if(mul_jobs.size() < n_mul){
					mul_jobs.push_back(i);
				}else{
					int min_idx = 0;
					for(int j=1;j<n_mul;++j){
						if(ready_list[mul_jobs[min_idx]].p > ready_list[add_jobs[j]].p){
							min_idx = j;
						}
					}
					if(ready_list[mul_jobs[min_idx]].p < ready_list[i].p){
						add_jobs[min_idx] = i;
					}
				}

			}else{
				cout << "ERROR: undefined op type. in main(). ABORTED" << endl;
				cout << i << " " << ready_list[i].op << endl;
				exit(12);
			}
		}
		cout << "}" << endl;
		cout << "Chosen ADD op: ";
		for(const auto k:add_jobs){
			cout << "(" << k+1 << "," << ready_list[k].p << "), ";
		}
		cout << endl;
		cout << "Chosen MUL op: ";
		for(const auto k:mul_jobs){
			cout << "(" << k+1 << "," << ready_list[k].p << "), ";
		}
		cout << endl;

		// update
		for(const auto k:add_jobs){
			ready_list[k].state = 2;
		}
		for(const auto k:mul_jobs){
			ready_list[k].state = 2;
		}
		update_readylist(ready_list, cdfg, n_readylist, n_cdfg);
		is_done = true;
		for(int i=0;i<n_readylist;++i){
			if(ready_list[i].state != 2){
				is_done = false;
			}
		}
	}


	delete [] cdfg;
	delete [] ready_list;
	return 0;
}
