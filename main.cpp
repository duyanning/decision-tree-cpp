#include "std.h" // precompile

using namespace std;
using namespace boost;


// 我们使用boost::any作为值的类型，为了在std::map、std::set等容器中存储any，需要为any提供一些支持。
namespace boost {

// 为了在std::set以及std::map中存储boost::any，需要能比较两个any
// 因为any在boost这个名字空间里，所以提供的operator<也得放在boost这个名字空间中。放在全局名字空间中找不到。
bool operator<(const any a, const any b)
{
    if (a.type() != b.type()) {
        assert(false);
        //throw "type mismatch!";
    }

    if (a.type() == typeid(int)) {
        return any_cast<int>(a) < any_cast<int>(b);
    }
    else if (a.type() == typeid(const char*)) {
        return string(any_cast<const char*>(a)) < any_cast<const char*>(b);
    }
    else if (a.type() == typeid(string)) {
        return any_cast<string>(a) < any_cast<string>(b);
    }
    else {
        assert(false);
    }
}

bool operator>=(const any a, const any b)
{
    return !(a < b);
}
    
bool operator==(const any a, const any b)
{
    if (a.type() == b.type()) {
        if (a.type() == typeid(int))
            return any_cast<int>(a) == any_cast<int>(b);
        if (a.type() == typeid(const char*))
            return string(any_cast<const char*>(a)) == any_cast<const char*>(b);
        if (a.type() == typeid(string))
            return any_cast<string>(a) == any_cast<string>(b);
        
        cout << a.type().name() << endl;
        cout << b.type().name() << endl;
        assert(false);
    }
    else {
        if (a.type() == typeid(const char*)) {
            if (b.type() == typeid(string)) {
                return string(any_cast<const char*>(a)) == any_cast<string>(b);
            }
        }

        if (a.type() == typeid(string)) {
            if (b.type() == typeid(const char*)) {
                return any_cast<string>(a) == any_cast<const char*>(b);
            }
        }

        return false;
        
        // cout << a.type().name() << endl;
        // cout << b.type().name() << endl;
        // //throw "type mismatch!";
        // assert(false);
    }
}
    
} // namespace boost

ostream& operator<<(ostream& os, const any& a)
{
    if (a.type() == typeid(int)) {
        os << any_cast<int>(a);
    }
    else if (a.type() == typeid(const char*)) {
        os << any_cast<const char*>(a);
    }
    else if (a.type() == typeid(string)) {
        os << any_cast<string>(a);
    }
    else {
        assert(false);
    }

    return os;
}


typedef vector<any> Row;      
typedef vector<Row> Table;
//typedef map<string, int> Result;
typedef map<string, double> Result;


// 决策树的节点
struct DecisionNode {
    int col;
    boost::any value;
    //unordered_map<boost::any, int> results; // 无法通过编译。改用map
    Result results; 
    DecisionNode* tb;
    DecisionNode* fb;

    DecisionNode(int col, any value, Result results, DecisionNode* tb, DecisionNode* fb)
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
Result unique_counts(Table rows)
{
    Result results;   // 最后一列是结果，是字符串
    for (auto row : rows) {
        //string r = any_cast<const char*>(row[row.size() -1]);
        // todo: 下面这段太丑了
        string r;
        if (row[row.size() -1].type() == typeid(const char*)) {
            r = any_cast<const char*>(row[row.size() -1]);
        }
        else if (row[row.size() -1].type() == typeid(string)) {
            r = any_cast<string>(row[row.size() -1]);
        }
        else {
            assert(false);
        }
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
    cout << "\n================\n";
    for (auto row : rows) {
        cout << "[";
        // for (auto c : row) {
        //     cout << c << ", ";
        // }

        // 相比上面被注释掉的这段代码，下边的代码可以更好地输出逗号
        for (auto iter = row.begin(); iter != row.end(); iter++) {
            if (iter != row.begin())
                cout << ", ";

            bool isStr = false; // 控制输出字符串的引号
            isStr = iter->type() == typeid(const char*) || iter->type() == typeid(string);
            if (isStr)
                cout << "'";
            cout << *iter;
            if (isStr)
                cout << "'";
        }

        cout << "]\n";
    }
    cout << "^^^^^^^^^^^^^^^^" << " size: " << rows.size() << "\n";
}

ostream& operator<<(ostream& os, const Result& r)
{
    os << "{";
    // for (auto i : r) {
    //     cout << i.first << " : " << i.second;
    // }
    for (auto iter = r.begin(); iter != r.end(); iter++) {
        if (iter != r.begin())
            cout << ", ";

        // bool isStr = false; // 控制输出字符串的引号
        // isStr = iter->first.type() == typeid(const char*) || iter->first.type() == typeid(string);
        // if (isStr)
        //     cout << "'";
        cout << iter->first << " : " << iter->second;
        // if (isStr)
        //     cout << "'";
    }
    os << "}";

    return os;
}


DecisionNode* build_tree(Table rows, function<double (Table rows)> scoref = entropy)
{
    if (rows.empty())
        return new DecisionNode;
    double current_score = scoref(rows);

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
        return new DecisionNode(best_criteria.first, best_criteria.second, Result(), trueBranch, falseBranch);
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
        //assert(tree->results.size() == 1); 
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

Result classify(Row observation, DecisionNode* tree)
{
    DecisionNode* branch = nullptr;
    if (tree->is_leaf()) {
        return tree->results;
    }
    else {
        auto v = observation[tree->col];
        if (is_numeric(v)) {
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


/*
def prune(tree,mingain):
    # If the branches aren't leaves, then prune them
    if tree.tb.results==None:
        prune(tree.tb,mingain)
    if tree.fb.results==None:
        prune(tree.fb,mingain)
    # If both the subbranches are now leaves, see if they
    # should merged
    if tree.tb.results!=None and tree.fb.results!=None:
        # Build a combined dataset
        tb,fb=[],[]
        for v,c in tree.tb.results.items( ):
            tb+=[[v]]*c
        for v,c in tree.fb.results.items( ):
            fb+=[[v]]*c
        # Test the reduction in entropy
        delta=entropy(tb+fb)-(entropy(tb)+entropy(fb)/2)
        if delta<mingain:
            # Merge the branches
            tree.tb,tree.fb=None,None
            tree.results=uniquecounts(tb+fb)
 */
void prune(DecisionNode* tree, double mingain)
{
    // If the branches aren't leaves, then prune them
    // if tree.tb.results==None:
    //     prune(tree.tb,mingain)
    if (tree->tb->results.empty())
        prune(tree->tb, mingain);
    // if tree.fb.results==None:
    //     prune(tree.fb,mingain)
    if (tree->fb->results.empty())
        prune(tree->fb, mingain);
    // If both the subbranches are now leaves, see if they should merged
    // if tree.tb.results!=None and tree.fb.results!=None:
    if (!tree->tb->results.empty() && !tree->fb->results.empty()) {
        // Build a combined dataset
    //     tb,fb=[],[]
        vector<vector<any>> tb, fb;
    //     for v,c in tree.tb.results.items( ):
    //         tb+=[[v]]*c  // 注意[1]*3为[1, 1, 1]，而[[1]]*3为[[1], [1], [1]]。
        // 此外，此处认定c是整数，但Result这个map中的value，自从引入mdclassify之后，就是double，可以是小数了。
        for (auto i : tree->tb->results) {
            auto v = i.first;
            auto c = i.second;
            for (int n = 0; n < c; ++n) {
                vector<any> row;
                row.push_back(v);
                tb.push_back(row);
            }
        }
    //     for v,c in tree.fb.results.items( ):
    //         fb+=[[v]]*c
        for (auto i : tree->fb->results) {
            auto v = i.first;
            auto c = i.second;
            for (int n = 0; n < c; ++n) {
                vector<any> row;
                row.push_back(v);
                fb.push_back(row);
            }
        }
        // Test the reduction in entropy
    //     delta=entropy(tb+fb)-(entropy(tb)+entropy(fb)/2)
        vector<vector<any>> tbfb = tb;
        tbfb.insert(tbfb.end(), fb.begin(), fb.end()); 
        double delta = entropy(tbfb) - (entropy(tb) + entropy(fb)) / 2;
    //     if delta<mingain:
        if (delta < mingain) {
            // Merge the branches
    //         tree.tb,tree.fb=None,None
            tree->tb = nullptr;
            tree->fb = nullptr;
    //         tree.results=uniquecounts(tb+fb)
            tree->results = unique_counts(tbfb);
        }
    }
    
}

// classify的修改版，可以处理缺失某些字段的observation
// 对于一个observation，给出的结果可能是多个类型
Result mdclassify(Row observation, DecisionNode* tree)
{
    DecisionNode* branch = nullptr;
    if (tree->is_leaf()) {
        return tree->results;
    }
    else {
        auto v = observation[tree->col];
        //cout << "compare " << tree->col << endl;
        if (v == "None") {      // observation的该字段缺失。(在observation用字符串"None"表示)
            //cout << "haha: " << tree->col << "=None" << endl;
            // tr,fr=mdclassify(observation,tree.tb),mdclassify(observation,tree.fb)
            auto tr = mdclassify(observation, tree->tb);
            auto fr = mdclassify(observation, tree->fb);
            // tcount=sum(tr.values( ))
            double tcount = accumulate(tr.begin(), tr.end(), 0, [](double value, pair<string, double> x) { return value + x.second; });
            // fcount=sum(fr.values( ))
            double fcount = accumulate(fr.begin(), fr.end(), 0, [](double value, pair<string, double> x) { return value + x.second; });
            // tw=float(tcount)/(tcount+fcount)
            double tw = double(tcount) / (tcount + fcount);
            // fw=float(fcount)/(tcount+fcount)
            double fw = double(fcount) / (tcount + fcount);
            // result={}
            Result result;
            // for k,v in tr.items( ): result[k]=v*tw
            for (auto i : tr) {
                string k = i.first;
                double v = i.second;
                result[k] = v * tw;
            }
            // for k,v in fr.items( ): result[k]=v*fw
            for (auto i : fr) {
                string k = i.first;
                double v = i.second;
                result[k] += v * fw;
            }
            // return result
            return result;
        }
        else {
            if (is_numeric(v)) {
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
    }

    return mdclassify(observation, branch);
}


void test_any()
{
    any a1 = "abc";
    any a2 = string("abc");
    const string abc = "abc";
    any a3 = abc;

    a1 == a2; // const char* and string
    a2 == a3;                   // string and const string, const好像不会被any记录
    
}

int main()
{

    //test_any();
    
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

    auto tree = build_tree(my_data);
    print_tree(tree);

    Row observation = {"(direct)", "USA", "yes", 5};
    auto r = classify(observation, tree);
    cout << r << endl;

    // prune(tree, 1.0);
    // print_tree(tree);

    // r = mdclassify(observation, tree); // 看下mdclassify在处理无缺失字段时的表现是否跟classify一致
    // cout << r << endl;

    r = mdclassify({"google", "None", "yes", "None" }, tree); // "None"代表字段缺失
    cout << r << endl;

    r = mdclassify({"google", "France", "None", "None" }, tree); // "None"代表字段缺失
    cout << r << endl;
}


// 下边这段代码可以从map中提取出所有keys，将first改为second就可以提取所有values。可以完美模拟python
template<template <typename...> class MAP, class KEY, class VALUE>
std::vector<KEY>
keys(const MAP<KEY, VALUE>& map)
{
    std::vector<KEY> result;
    result.reserve(map.size());
    for(const auto& it : map){
        result.emplace_back(it.first);
    }
    return result;
}


/*
typid乱码
#include <cxxabi.h>
realname = abi::__cxa_demangle(e.what(), 0, 0, &status);
参考 https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html

或者用工具
c++filt -t symbol
 */
