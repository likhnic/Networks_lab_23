#include<bits/stdc++.h>
using namespace std;
#define MOD 1000000007
#define pb push_back
#define vlli vector<long long int>
typedef long long int lli;
typedef unsigned long long int ulli;
#define vvlli vector<vector<lli> >
#define vvp vector<vector<pair<lli,lli> > >

long cnt=0,ans=0;
vector<int>sol;
void rec(int lvl,int payoff,int mx,int red,int black)
{
    mx=max(mx,payoff);
    if(lvl==8)
    {
        ans+=payoff;
        cnt++;
        sol.push_back(mx);
        // cnt+=payoff;
        return ;
    }
    if(red)rec(lvl+1,payoff+1,mx,red-1,black);
    if(black)rec(lvl+1,payoff-1,mx,red,black-1);
    cnt+=1;
    if(payoff>=0){
        ans+=payoff;
    }
    return ;
}
int main(){

    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    ans=0;
    cnt=0;
    rec(0,0,0,4,4);
    cout<<"Expected value="<<(ans/cnt)<<endl;
    cout<<"sol=\n";
    cout<<sol.size()<<"\n";
    for(auto x:sol)cout<<x<<" ";cout<<"\n";
}