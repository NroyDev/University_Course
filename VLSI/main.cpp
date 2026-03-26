#include<bits/stdc++.h>
using namespace std;

void maxCover(const vector<vector<bool>>& mat, vector<int>& ans, vector<int>& sel, int lv){
    if(mat.size() <1){  // prevent mat is empty cause an error
        return;
    }
    const int n = mat.size();
    const int m = mat[0].size();
    // terminate condition
    if(lv==n){
        set<int> s;
        for(const auto& k:sel){     // check the coverage of selection
            for(int i=0;i<m;++i){
                if(mat[k][i]){
                    s.insert(i);
                }
            }
        }
        if(s.size()!=m){            // didn't cover all
            return;
        }

        if(sel.size()<ans.size()){  // the selection is better
            ans = sel;
        }
        return;
    }

    // Recursive
    sel.push_back(lv);
    maxCover(mat, ans, sel, lv+1);  // take
    sel.pop_back();
    maxCover(mat, ans, sel, lv+1);  // not take
}

bool build_state(int input, int state){
    input |= (!(input&(1<<3) && (input&(1<<2))))<<4;    // n1 = !(a3 && a2);
    input |= (!(input&(1<<1)))<<5;                      // n2 = !a1;
    input |= (!(input&(1<<5) || (input&(1<<0))))<<6;    // n3 = !(n2 || a0);
    bool y = (!(input&(1<<4) && (input&(1<<6))));       // y = !(n1 && n3);
    bool y2;

    input &= ((1<<4)-1);
    bool fault_val = !(state&1);
    int fault_pos = state>>1;
    if(fault_val){
        input |= 1<<fault_pos;
        input |= (!(input&(1<<3) && (input&(1<<2))))<<4;    // n1 = !(a3 && a2);
        input |= 1<<fault_pos;
        input |= (!(input&(1<<1)))<<5;                      // n2 = !a1;
        input |= 1<<fault_pos;
        input |= (!(input&(1<<5) || (input&(1<<0))))<<6;    // n3 = !(n2 || a0);
        input |= 1<<fault_pos;
        input |= (!(input&(1<<4) && (input&(1<<6))))<<7;    // y = !(n1 && n3);
        input |= 1<<fault_pos;
        y2 = input>>7;
    }else{
        input &= ~(1<<fault_pos);
        input |= (!(input&(1<<3) && (input&(1<<2))))<<4;    // n1 = !(a3 && a2);
        input &= ~(1<<fault_pos);
        input |= (!(input&(1<<1)))<<5;                      // n2 = !a1;
        input &= ~(1<<fault_pos);
        input |= (!(input&(1<<5) || (input&(1<<0))))<<6;    // n3 = !(n2 || a0);
        input &= ~(1<<fault_pos);
        input |= (!(input&(1<<4) && (input&(1<<6))))<<7;    // y = !(n1 && n3);
        input &= ~(1<<fault_pos);
        y2 = input>>7;
    }
    return y!=y2;
}

int main(){
    // 存每個 state 可以 cover 到的 stuct at faults 的 matrix
    vector<vector<bool>> mat;

    // 建造 matrix  row/col: input/state
    for(int input=0;input<(1<<4);++input){  // input bit: A3A2A1A0
        vector<bool>states;
        for(int state=0;state<16;++state){
            states.push_back(build_state(input,state));
        }
        mat.push_back(states);
        cout << endl;
    }
    
    // 顯示造出的 matrix
    int n = mat.size();
    int m = mat[0].size();
    cout<<"State\\i\t";
    for(int i=0;i<m;++i){
        cout << i%10 << " ";
    }
    cout << endl;
    for(int i=0;i<m;++i){
        cout << "S"<<i+1 << "\t";
        for(int j=0;j<n;++j){
            cout << mat[j][i] << " ";
        }
        cout << endl;
    }

    // 找 minial set 並顯示出來
    vector<int> ans,sel;
    for(int i=0;i<16;++i){
        ans.push_back(i);
    }
    maxCover(mat,ans,sel,0);
    cout << "Result:" << endl;
    for(const int& k:ans){
        cout << k << " ";
    }
    cout << endl;

    return 0;
}







// vector<int> maxCover(vector<vector<bool>>& mat){
//     vector<int> ans;
//     if(mat.size() <1){  // 防止
//         return {};
//     }
//     const int n = mat.size();
//     const int m = mat[0].size();
    
//     set<int> s, s2;
//     for(int i=0;i<16;++i){
//         s.insert(i);
//     }
//     for(int i=0;i<16;++i){
//         int max_idx = *(s.begin());
//         int maxn = 0;
//         for(int k:s){
//             int cnt = 0;
//             for(int col=0;col<16;++col){
//                 if(mat[k][col] && s2.count(col)==0){
//                     ++cnt;
//                 }
//             }
//             if(cnt>maxn){
//                 maxn = cnt;
//                 max_idx = k;
//             }
//         }
//         s.erase(max_idx);

//         ans.push_back(max_idx);
//         for(int col = 0; col<16;++col){
//             if(mat[max_idx][col]){
//                 s2.insert(col);
//             }
//         }
//         if(s2.size() == m){
//             break;
//         }
//     }

//     return ans;
// }



/*      old build state
        // bool a3 = i&(1<<3);
        // bool a2 = i&(1<<2);
        // bool a1 = i&(1<<1);
        // bool a0 = i&(1<<0);
        // cout << a3<<a2<<a1<<a0 << endl;
        // bool n1,n2,n3,y,y2;
        // n1 = !(a3 && a2);
        // n2 = !a1;
        // n3 = !(n2 || a0);
        // y = !(n1 && n3);
        
        // n1 = !(1 && a2);
        // n2 = !a1;
        // n3 = !(n2 || a0);
        // y2 = !(n1 && n3);
        // bool s1 = y!=y2;
        
        // n1 = !(0 && a2);
        // n2 = !a1;
        // n3 = !(n2 || a0);
        // y2 = !(n1 && n3);
        // bool s2 = y!=y2;
        
        // n1 = !(a3 && 1);
        // n2 = !a1;
        // n3 = !(n2 || a0);
        // y2 = !(n1 && n3);
        // bool s3 = y!=y2;
        
        // n1 = !(a3 && 0);
        // n2 = !a1;
        // n3 = !(n2 || a0);
        // y2 = !(n1 && n3);
        // bool s4 = y!=y2;
        
        // n1 = !(a3 && a2);
        // n2 = !1;
        // n3 = !(n2 || a0);
        // y2 = !(n1 && n3);
        // bool s5 = y!=y2;

        // n1 = !(a3 && a2);
        // n2 = !0;
        // n3 = !(n2 || a0);
        // y2 = !(n1 && n3);
        // bool s6 = y!=y2;
        
        // n1 = !(a3 && a2);
        // n2 = !a1;
        // n3 = !(n2 || 1);
        // y2 = !(n1 && n3);
        // bool s7 = y!=y2;
        
        // n1 = !(a3 && a2);
        // n2 = !a1;
        // n3 = !(n2 || 0);
        // y2 = !(n1 && n3);
        // bool s8 = y!=y2;
        
        // n1 = !(a3 && a2);
        // n2 = !a1;
        // n3 = !(n2 || a0);
        // y2 = !(1 && n3);
        // bool s9 = y!=y2;
        
        // n1 = !(a3 && a2);
        // n2 = !a1;
        // n3 = !(n2 || a0);
        // y2 = !(0 && n3);
        // bool s10 = y!=y2;
        
        // n1 = !(a3 && a2);
        // n2 = !a1;
        // n3 = !(1 || a0);
        // y2 = !(n1 && n3);
        // bool s11 = y!=y2;
        
        // n1 = !(a3 && a2);
        // n2 = !a1;
        // n3 = !(0 || a0);
        // y2 = !(n1 && n3);
        // bool s12 = y!=y2;
        
        // n1 = !(a3 && a2);
        // n2 = !a1;
        // n3 = !(n2 || a0);
        // y2 = !(n1 && 1);
        // bool s13 = y!=y2;
        
        // n1 = !(a3 && a2);
        // n2 = !a1;
        // n3 = !(n2 || a0);
        // y2 = !(n1 && 0);
        // bool s14 = y!=y2;
        // bool s15 = y!=0;
        // bool s16 = y!=1;
        // mat.push_back({s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16});
*/