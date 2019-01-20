#include "std.h" // precompile

using namespace std;
using namespace boost;

typedef vector<any> Row;
typedef vector<Row> Table;
typedef map<string, int> Result;


// 决策树的节点
struct DecisionNode {
    int col;
    boost::any value;
    //unordered_map<boost::any, int> results; // 字典。无法通过编译。改用map
    map<string, int> results; // 字典
    DecisionNode* tb;
    DecisionNode* fb;

    DecisionNode(int col, any value, map<string, int> results, DecisionNode* tb, DecisionNode* fb)
    {
        this->col = col;
        this->value = value;
        this->results = results;
        this->tb = tb;
        this->fb = fb;
    }

    DecisionNode() = default;

    bool is_leaf()
    {
        return !results.empty();
    }
};

bool is_numeric(any value)
{
    if (value.type() == typeid(int) || value.type() == typeid(double))
        return true;
    else
        return false;
}

pair<Table, Table> divide_set(Table rows, int column, any value)
{
    function<bool (const Row&)> split_function;
    if (value.type() == typeid(int) || value.type() == typeid(double)) { // numeric 
        split_function = [&column, &value](Row row) { return any_cast<int>(row[column]) >= any_cast<int>(value); };
    }
    else {
        //split_function = [&column, &value](Row row) { return any_cast<string>(row[column]) == any_cast<string>(value); };
        split_function = [&column, &value](Row row) { return any_cast<const char*>(row[column]) == any_cast<const char*>(value); };
        //split_function = [&column, &value](Row row) { return row[column] == value; };
    }
    

    Table set1;
    Table set2;

    for (auto row : rows) {
        if (split_function(row)) {
            set1.push_back(row);            
        }
        else {
            set2.push_back(row);            
        }
    }

    return make_pair(set1, set2);
}

/*

def uniquecounts(rows):
  results={}
  for row in rows:
    # The result is the last column
    r=row[len(row)-1]
    if r not in results: results[r]=0
    results[r]+=1
  return results

 */
map<string, int> unique_counts(Table rows)
{
    map<string, int> results;   // 最后一列是结果，是字符串
    for (auto row : rows) {
        string r = any_cast<const char*>(row[row.size() -1]);
        if (results.find(r) == results.end()) {
            results[r] = 0;
        }
        results[r] += 1;
    }

    return results;
}

/*
def entropy(rows):
  from math import log
  log2=lambda x:log(x)/log(2)
  results=uniquecounts(rows)
  # Now calculate the entropy
  ent=0.0
  for r in results.keys( ):
    p=float(results[r])/len(rows)
    ent=ent-p*log2(p)
  return ent
 */
double entropy(Table rows)
{
    //function<double (double)> log2 = [](double x) { return log(x) / log(2); };
    auto log2 = [](double x) { return log(x) / log(2); };
    auto results = unique_counts(rows);
    // 计算熵
    double ent = 0;

    for (auto r : results) {
        //cout << r.second << endl;
        double p = double(r.second) / rows.size();
        ent = ent - p * log2(p);
    }

    return ent;
}

void show_table(const Table& rows)
{
    cout << "\ntable begin\n";
    for (auto row : rows) {
        cout << "[";
        for (auto c : row) {
            if (is_numeric(c)) {
                //cout << "i ";
                cout << any_cast<int>(c);
                cout << ", ";
            }
            else {
                //cout << "s ";
                cout << any_cast<const char*>(c);
                cout << ", ";
            }
        }
        cout << "]\n";
    }
    cout << "table end\n";
}

// bool operator==(const any& a, const any& b)
// {
//     if (a.type() != b.type())
//         return false;
//     if (is_numeric(a)) {
//         return any_cast<int>(a) == any_cast<int>(b);
//     }

//     return string(any_cast<const char*>(a)) == any_cast<const char*>(b);
// }

namespace boost {

// 为了在std::set以及std::map中存储boost::any，需要能比较两个any
// 因为any在boost这个名字空间里，所以提供的operator<也得放在boost这个名字空间中。放在全局名字空间中找不到。
bool operator<(const any a, const any b)
{
    if (a.type() != b.type())
        throw "type mismatch!";
    if (is_numeric(a)) {
        return any_cast<int>(a) < any_cast<int>(b);
    }

    return string(any_cast<const char*>(a)) < any_cast<const char*>(b);
}

bool operator>=(const any a, const any b)
{
    return !(a < b);
}
    
bool operator==(const any a, const any b)
{
    if (a.type() != b.type())
        throw "type mismatch!";
    if (is_numeric(a)) {
        return any_cast<int>(a) == any_cast<int>(b);
    }

    return string(any_cast<const char*>(a)) == any_cast<const char*>(b);
}
    
} // namespace boost


ostream& operator<<(ostream& os, const any& a)
{
    if (is_numeric(a)) {
        os << any_cast<int>(a);
    }
    else {
        os << any_cast<const char*>(a);
    }
    return os;
}

ostream& operator<<(ostream& os, const Result& r)
{
    os << "{";
    for (auto i : r) {
        cout << i.first << " : " << i.second;
    }
    os << "}" << endl;

    return os;
}


DecisionNode* build_tree(Table rows, function<double (Table rows)> scoref = entropy)
{
    if (rows.empty())
        return new DecisionNode;
    double current_score = scoref(rows);

    // Set up some variables to track the best criteria
    double best_gain = 0.0;
    pair<int, any> best_criteria;
    pair<Table, Table> best_sets;

    // 用各个列的各种值对表格进行划分，试出效果最好的(列号，值)
    double column_count = rows[0].size() - 1;
    for (int col = 0; col < column_count; ++col) { // 各个列
        // 把这一列各种不同的值收集到一个集合中(元素不重复)
        set<any> column_values;
        for (auto row : rows) {
            column_values.insert(row[col]);
        }

        for (auto value : column_values) { // 此列的各种值
            // 用此列与此值划分一下
            auto set1set2 = divide_set(rows, col, value);
            Table set1 = set1set2.first;
            Table set2 = set1set2.second;
            
            // 看看划分之后两个分表的熵的加权平均是不是比原来的表的熵降低了，是的话就划分
            double p = double(set1.size()) / rows.size();
            double gain = current_score - p * scoref(set1) - (1 - p) * scoref(set2);
            if (gain > best_gain && set1.size() > 0 && set2.size() > 0) {
                best_gain = gain;
                best_criteria = make_pair(col, value);
                best_sets = set1set2;
            }
        }
    }

    // 构造当前节点
    if (best_gain > 0) {        // 如果可以继续划分，构造两棵子树
        DecisionNode* trueBranch = build_tree(best_sets.first);
        DecisionNode* falseBranch = build_tree(best_sets.second);
        return new DecisionNode(best_criteria.first, best_criteria.second, map<string, int>(), trueBranch, falseBranch);
    }
    else {                      // 如果无法继续划分，就作为叶子节点
        return new DecisionNode(0, 0, unique_counts(rows), nullptr, nullptr);
    }

}

/*
def printtree(tree,indent=''):
  # Is this a leaf node?
  if tree.results!=None:
    print str(tree.results)
  else:
    # Print the criteria
    print str(tree.col)+':'+str(tree.value)+'? '
    # Print the branches
    print indent+'T->',
    printtree(tree.tb,indent+' ')
    print indent+'F->',
    printtree(tree.fb,indent+' ')
 */
void print_tree(DecisionNode* tree, string indent = " ")
{
    if (!tree->results.empty()) { // 叶子节点
        assert(tree->results.size() == 1); 
        // cout << "{";
        // for (auto i : tree->results) {
        //     cout << i.first << " : " << i.second;
        // }
        // cout << "}" << endl;
        cout << tree->results << endl;
    }
    else {
        // 打印criteria
        cout << tree->col << " : " << tree->value << " ? " << endl;
        // 打印真假两个分支
        cout << indent << "T-> ";
        print_tree(tree->tb, indent + " ");
        cout << indent << "F-> ";
        print_tree(tree->fb, indent + " ");
        
    }
}


/*
def classify(observation,tree):
  if tree.results!=None:
    return tree.results
  else:
    v=observation[tree.col]
    branch=None
    if isinstance(v,int) or isinstance(v,float):
      if v>=tree.value: branch=tree.tb
      else: branch=tree.fb
    else:
      if v==tree.value: branch=tree.tb
      else: branch=tree.fb
  return classify(observation,branch)
 */

map<string, int> classify(Row observation, DecisionNode* tree)
{
    DecisionNode* branch = nullptr;
    if (tree->is_leaf()) {
        return tree->results;
    }
    else {
        auto v = observation[tree->col];
        if (is_numeric(v)) {
            //if (any_cast<int>(v) >= tree.value)
            if (v >= tree->value)
                branch = tree->tb;
            else
                branch = tree->fb;
        }
        else {
            if (v == tree->value)
                branch = tree->tb;
            else
                branch = tree->fb;
        }
    }

    return classify(observation, branch);
}

class A {
    
};


int main()
{
    Table my_data = {
        {"slashdot", "USA", "yes", 18, "None"}, 
        {"google", "France", "yes", 23, "Premium"}, 
        {"digg", "USA", "yes", 24, "Basic"}, 
        {"kiwitobes", "France", "yes", 23, "Basic"}, 
        {"google", "UK", "no", 21, "Premium"}, 
        {"(direct)", "New Zealand", "no", 12, "None"}, 
        {"(direct)", "UK", "no", 21, "Basic"}, 
        {"google", "USA", "no", 24, "Premium"}, 
        {"slashdot", "France", "yes", 19, "None"}, 
        {"digg", "USA", "no", 18, "None"}, 
        {"google", "UK", "no", 18, "None"}, 
        {"kiwitobes", "UK", "no", 19, "None"}, 
        {"digg", "New Zealand", "yes", 12, "Basic"}, 
        {"slashdot", "UK", "no", 21, "None"}, 
        {"google", "UK", "yes", 18, "Basic"}, 
        {"kiwitobes", "France", "yes", 19, "Basic"}
    };

    show_table(my_data);
    auto set1set2 = divide_set(my_data, 2, "yes");

    show_table(set1set2.first);
    show_table(set1set2.second);

    cout << entropy(my_data) << endl;
    cout << entropy(set1set2.first) << endl;
    //cout << entropy(set1set2.second) << endl;


    // any a = 1;
    // any b = "abc";
    // a == b; // error
    // a < b;

    //set<A> s;
    //s.insert(A());

    auto tree = build_tree(my_data);
    print_tree(tree);

    Row observation = {"(direct)", "USA", "yes", 5};
    auto r = classify(observation, tree);
    cout << r << endl;
    return 0;
}
