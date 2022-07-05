#include <iostream>
#include <iomanip>
#include <assert.h>
#include <string.h>

using namespace std;

int *P, *L;

void get_table_256(void){
    int T = 256, PX=285;
    P = new int [T]; L = new int [T];
    memset(P, 0, sizeof(int)*T);
    memset(L, 0, sizeof(int)*T);
    int n = 1; 
    for(int i=0; i<T; i++){
        P[i] = n; L[n] = i;
        n*=2;
        if(n >= T) n = n ^ PX;
    }
    L[1] = 0;
    return;
}

void destory_table_256(void){
    delete [] P;
    delete [] L;
    return;
}

void print_table_256(void){
    int X = 8;
    int sqrtT=1<<(X/2);
    cout << "power" << endl;
    for(int i=0; i<sqrtT; i++){
        for(int j=0; j<sqrtT; j++){
            cout << setw(2) << hex << P[i*sqrtT+j] << " ";
        }
        cout << endl;
    }
    cout << "log" << endl;
    for(int i=0; i<sqrtT; i++){
        for(int j=0; j<sqrtT; j++){
            cout << setw(3) << hex << L[i*sqrtT+j] << " ";
        }
        cout << endl;
    }
}


class GF{
private:
    int V;  // value
    int X;  // 8
    int T;  // 256 

public:
    GF(int v=0, int x=8){
        V = v; X = x; T = 1 << X;
        // 创建时V [0, T)
        assert(V < T && V >= 0);
        // 本项目目前只实现GF8
        assert(X == 8);
    }
    ~GF(){
    }

    int get_value()const{
        return V;
    }
    int get_x_field()const{
        return X;
    }

    // 重定义操作符
    GF operator+(const GF& b){   
        GF c(
            (this->V ^ b.get_value()) % (this->T-1)
        ); 
        return c;
    }
    GF operator-(const GF& b){   
        GF c(
            (this->V ^ b.get_value()) % (this->T-1)
        ); 
        return c;
    }
    GF operator*(const GF& b){
        if(b.get_value() == 0 || V == 0){
            GF zero(0);
            return zero;
        }   
        GF c(
            P[(L[this->V] + L[b.get_value()]) % (this->T-1)] 
        );
        return c;
    }
    GF operator/(const GF& b){
        assert(b.get_value() != 0);
        if(V == 0){
            GF zero(0);
            return zero;
        }
        GF c(
            P[(L[this->V] - L[b.get_value()] + (this->T-1)) % (this->T-1)] 
        ); 
        return c;
    }
    GF pow(const int p){
        GF res(1); GF me(this->V);
        for(int i=0; i<p; i++){
            res = res * me;
        }
        return res;
    }
    bool operator==(const GF& b){
        return b.V == this->V;
    }
    bool operator!=(const GF& b){
        return b.V != this->V;
    }
};

class GFM{
public:
    int R, C;
    GF **M;
    bool created=false;
    GFM(){}
    void create(int r, int c){
        R = r; C = c;
        M = new GF* [R];
        for(int i=0; i<R; i++){
            M[i] = new GF [C];
        }
        created = true;
    }
    ~GFM(){
        if(created){
            for(int i=0; i<R; i++) delete [] M[i];
            delete [] M;
        }
    }
    void show(){
        assert(created);
        for(int i=0; i<R; i++){
            for(int j=0; j<C; j++){
                cout << setw(3) << M[i][j].get_value() << " "; 
            }
            cout << endl;
        }
        return;
    }

    GFM select_rows(const int* selected_rows, const int nums){
        assert(created);
        for(int i=0; i<nums; i++){
            for(int j=i+1; j<nums; j++){
                assert(i != j);
            }
            assert(selected_rows[i] < R && selected_rows[i] >= 0);
        }
        GFM res; res.create(nums, C);
        for(int i=0; i<nums; i++){
            for(int j=0; j<C; j++){
                res.M[i][j] = M[selected_rows[i]][j];
            }
        }
        return res;
    }

    void add_row(const int row, const GFM* add){
        assert(add->R==1 && add->C==C);
        assert(row < R && row >=0);
        for(int j=0; j<C; j++){
            M[row][j] = M[row][j] + add->M[0][j];
        }
        return;
    }

    void mul_row(const int row, const GF mul){
        assert(row < R && row >=0);
        for(int j=0; j<C; j++){
            M[row][j] = M[row][j] * mul;
        }
        return;
    }

    void inverse(){
        assert(created);
        if(R!=C){cout << "R!=C"; return;}
        int N=R;
        // 伴随矩阵
        GFM adjoint; adjoint.create(N, 2*N);
        for(int i=0; i<N; i++){
            for(int j=0; j<N; j++){
                adjoint.M[i][j]  = M[i][j];
            }
            for(int j=0; j<N; j++){
                if(i==j){GF one(1); adjoint.M[i][N+j] = one;}
            }
        }

        cout << "adjoint" << endl;
        adjoint.show();

        GF zero(0); GF one(1);

        for(int i=0; i<N; i++){
            cout << endl << "====Enter i = " << i << "=====" << endl;
            if(adjoint.M[i][i] == zero){
                cout << "adjoint.M[" << i << "][ " << i << " ] == zero" << endl;
                for(int k=i+1; k<N; k++){
                    if(k==i) continue;
                    if(adjoint.M[k][i] != zero){
                        cout << "found adjoint.M[" << k << "][" << i << "] != zero" << endl;
                        int sl[1] = {k};
                        GFM sgfm = adjoint.select_rows(sl, 1);
                        sgfm.mul_row(0, one / adjoint.M[k][i]);
                        cout << "padding sgfm = " << endl;
                        sgfm.show();
                        adjoint.add_row(i, &sgfm);
                        break;
                    }
                }
            }
            if(adjoint.M[i][i] == zero){
                cout << "error padding" << endl;
                assert(adjoint.M[i][i] == zero);
            }
            cout << "after padding" << endl;
            adjoint.show();
            for(int k=0; k<N; k++){
                if(k==i) continue;
                if(adjoint.M[k][i] == zero) continue;
                int sl[1] = {i};
                GFM sgfm = adjoint.select_rows(sl, 1);
                sgfm.mul_row(0, adjoint.M[k][i] / adjoint.M[i][i]);
                // cout << "sgfm k=" << k << endl;
                // sgfm.show();
                adjoint.add_row(k, &sgfm);

            }

            adjoint.mul_row(i, one/adjoint.M[i][i]);

            cout << "i = " << i << " final" << endl;
            adjoint.show();
            cout << "=====end of i = " << i << "=====" << endl << endl;

        }

        cout << "final" << endl;
        adjoint.show();
    }

};

class EC{
private:
    int N, K;
    GFM M;

public:
    EC(int n, int k){
        N=n; K=k;
        assert(N<100 && K<50);
        // 分配内存
        M.create(N+K, N);
        // 初始化前N行为单位矩阵
        for(int i=0; i<N; i++){
            for(int j=0; j<N; j++){
                if(i==j){GF one(1); M.M[i][j] = one;}
            }
        }
        // 初始化后K行为范德蒙德矩阵
        for(int i=0; i<K; i++){
            for(int j=0; j<N; j++){
                GF alpha(i+1);
                M.M[N+i][j] = alpha.pow(j);
            }
        }
        cout << "M = " << endl;
        M.show();
        int sl[12] = {0,1,3,4,6,7,8,10,11, 12,14,15};
        GFM sgfm = M.select_rows(sl, 12);
        cout << "sgfm = " << endl;
        sgfm.show();
        
        // M.add_row(27, &sgfm);
        // cout << "add sgfm to row[27]"<< endl;
        // M.show();
        // GF a(2);
        // M.mul_row(27, a);
        // cout << "row[27]*=2"<< endl;
        // M.show();
        cout << "inverse" << endl;
        sgfm.inverse();
    }

    ~EC(){
    }


};

int main(){
    get_table_256();

    GF a(1);
    GF b(1);
     
    cout << a.get_value() << " + " << b.get_value() << " = " << (a+b).get_value() << endl;
    cout << a.get_value() << " - " << b.get_value() << " = " << (a-b).get_value() << endl;
    cout << a.get_value() << " * " << b.get_value() << " = " << (a*b).get_value() << endl;
    cout << a.get_value() << " / " << b.get_value() << " = " << (a/b).get_value() << endl;

    EC ec(12, 5);

    destory_table_256();
    return 0;
}
