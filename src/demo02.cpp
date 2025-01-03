#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

int main(){
    int n,bagWeight;
    cin>>n>>bagWeight;
    cout<<"n="<<n<<" "<<"bagWeight="<<bagWeight<<endl;
    vector<int> weight(n,0);
    vector<int> value(n,0);
    //输入
    for(int i=0;i<n;i++){
        cin>>weight[i];
    }
    for(int i=0;i<n;i++){
        cin>>value[i];
    }
    vector<int> dp(bagWeight+1,0);

    for(int i=1;i<n;i++){
        for(int j=1;j<bagWeight+1;j++){
            if(j<weight[i]){
                dp[j]=dp[j];
            }else {
                dp[j]=max(dp[j],dp[j-weight[i]]+value[i]);
            }
        }
    }
    cout<<"res="<<dp[bagWeight]<<endl;
    return dp[bagWeight];
}
