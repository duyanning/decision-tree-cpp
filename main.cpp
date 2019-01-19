#include "std.h" // precompile

using namespace std;
using namespace boost;

// 决策树的节点
struct DecisionNode {
    int col;
    boost::any value;
    //unordered_map<boost::any, int> results; // 字典。无法通过编译。改用map
    map<any, int> results; // 字典
    DecisionNode* tb;
    DecisionNode* fb;

    DecisionNode(int col, any value, map<any, int> results, DecisionNode* tb, DecisionNode* fb)
    {
        this->col = col;
        this->value = value;
        this->results = results;
        this->tb = tb;
        this->fb = fb;
    }
};

int main()
{
    return 0;
}
