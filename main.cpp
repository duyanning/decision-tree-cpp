#include "std.h" // precompile

using namespace std;
using namespace boost;

typedef vector<any> Row;
typedef vector<Row> Table;

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
    return 0;
}
