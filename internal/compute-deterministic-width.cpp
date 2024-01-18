// include DFA automaton implementation
#include "automaton.hpp"
#include "hashing_function.hpp"
#include "utils.hpp"

typedef DFA_unidirectional_out_labelled<std::unordered_set<uint>> DFA; // DFA type

// global variables for recursive function
std::vector<uint>* v; // pointer to vector containing all states with the same outgoing label
char label; // label
std::vector< std::pair<uint,uint> > intervals; // intervals vector
DFA A = DFA();
DFA M = DFA();
uint p;

// DFS function to find if a cycle exists
bool isCyclic(uint c, bool visited[], bool* recStack)
{
    if (visited[c] == false)
    {
        // mark state as visited
        visited[c] = true;
        recStack[c] = true;

        // visit all adjacent states
        for (auto& i: A.at(c)->out)
        {
            if (!visited[i.second] && isCyclic(i.second, visited, recStack))
            {
                return true;
            }
            else if (recStack[i.second])
            {
                return true;
            }
        }
    }

    // remove the state from recursion stack
    recStack[c] = false;
    return false;
}

bool DFS_cycle_detection()
{
    // mark all states as not visited
    uint V = A.no_nodes();
    bool* visited = new bool[V];
    bool* recStack = new bool[V];
    for (uint i = 0; i < V; ++i) {
        visited[i] = false;
        recStack[i] = false;
    }

    // recursive function call
    for (uint i = 0; i < V; ++i)
    {
        if (!visited[i] && isCyclic(i, visited, recStack))
        {
            return true;
        }
    }
 
    return false;
}

// recursive p level for loop
void recursive_construction( uint rightLim, uint l ) 
{
    // static vector for loop indexes
    static std::vector<uint> indexes(p,0);
    // set representing states in the pruned automaton
    static std::unordered_set<uint> state;
    // simulate the lth nested cycle
    if( l < p )
    {
        for(uint i=indexes[l-1]+1;i<=(*v).size()-(p-l);++i)
        {
            // check if the current state overlap with the previous one
            if( rightLim <= intervals[(*v)[i-1]].first )
                break;
            // set new index for level l and recurse
            indexes[l] = i;
            recursive_construction(std::min(rightLim,intervals[(*v)[i-1]].second),l+1);
        }
    }
    else
    {
        // last recursion level
        for(uint j=indexes[l-1]+1;j<=(*v).size();++j)
        {
            // check if the current state overlap with the previous one
            if( rightLim <= intervals[(*v)[j-1]].first )
                break;
            // empty set
            state.clear();
            // add an set containing p overlapping states
            // and fill state vector
            for(uint i=1;i<p;++i)
                state.insert((*v)[indexes[i]-1]);
            // insert the last element in set
            state.insert((*v)[j-1]);
            // add the empty state to the pruned automaton
            A.add_state(state,label);
        }
    }
}
   
int main(int argc, char** argv)
{
    // initialize necessary data structures
    std::unordered_map<char,std::vector<uint>> L;
    //std::vector< std::pair <uint,uint> > intervals;
    std::vector<uint> order;
    
    if(argc > 3)
    { 
        // set input arguments
        std::string in_dfa, in_interval;
        p = read_uint<uint>(argv[1]);
        in_dfa = std::string(argv[2]);
        in_interval = std::string(argv[3]);

        // read intervals file
        uint max_beg = read_interval(in_interval,intervals,false);
        uint n = static_cast<uint>(intervals.size());

        #ifdef VERBOSE
        {
            std::cout << "### sorting intervals ###\n";
            std::cout << "-> number of intervals: " << n << "\n";
        }
        #endif

        // sort the intervals and store their ordering in order
        order.resize(n);
        counting_sort(intervals, order, max_beg+1);

        #ifdef VERBOSE
        {
            // print intervals
            for(uint i=0;i<intervals.size();++i)
            {
                std::cout << intervals[order[i]].first << " - " << intervals[order[i]].second << " : " << order[i] << "\n";
            }
            std::cout << "### compute A^" << p << " pruned automaton ###\n";
        }
        #endif
        
        // stop if p is greater than the number of states in the minimum DFA
        if( p > intervals.size() )
        {
            std::cout << "The tested width is greater than the number of states in the minimum DFA\n";
            exit(0);
        }

        // read minimized DFA
        read_min_dfa(in_dfa,M); 

        // create L data structure
        for(uint x=0;x<n;++x)
        { 
            uint i = order[x];
            for (auto& j: M.at(i)->out)
            {
                if(L.find(j.first) != L.end())
                {
                    L[j.first].push_back(i);
                }
                else{ L[j.first] = std::vector<uint>{ i }; }
            }
        }
        // free order vector
        order.clear();

        #ifdef VERBOSE
        {
            std::cout << "-> L data structure\n";
            // print L data structure
            for (auto& j: L)
            {
                std::cout << j.first << " : ";
                for(uint i=0;i<j.second.size();++i)
                {
                    std::cout << j.second[i] << " ";
                }
                std::cout << std::endl;
            }
            std::cout << "-> States in the A^" << p << " pruned automaton\n";
        }
        #endif

        // construct the A^p pruned automaton
        for (auto& j: L)
        {
            if( j.second.size() < p )  
                continue;
            v = &j.second;
            label = j.first;
            // indexes of the p nested loops
            recursive_construction(U_MAX,1);
        }
        // clear intervals vector
        intervals.clear();

        #ifdef VERBOSE
        {
            uint node_count = 0;
            for (auto& m: *A.give_mapping())
            {
                std::cout << node_count++ << ": (";
                for(auto i = m.first.begin(); i != m.first.end(); ++i)
                {
                    if( i != m.first.begin() ){ std::cout << " "; }
                    std::cout << *i;
                }
                std::cout << ") labels: (";
                auto entry = A.at(m.second);
                for(uint i=0;i<entry->labels.size();++i)
                {
                    if( i > 0 ){ std::cout << " "; }
                    std::cout << entry->labels[i];
                }
                std::cout << ")\n";
            }
            std::cout << "Number of states: " << A.no_nodes() << "\n";
            std::cout << "-> Edges in the A^" << p <<" pruned automaton\n";
        }
        #endif

        // compute edges in the A^p squared automaton
        for (auto& m: *A.give_mapping())
        {
            // compute all edges for each label separaterly
            for (auto& j: A.at(m.second)->labels)
            {
                // compute reached state
                //std::vector<uint> state(p,0);
                std::unordered_set<uint> state;
                //for(uint i=0;i<m.first.size();++i)
                for (const auto& e: m.first)
                    state.insert(M.at(e)->out[j]);
                    //state[i] = M.at(m.first[i])->out[j];

                // check if state is present
                auto entry = A.give_mapping()->find(state);
                if( entry != A.give_mapping()->end() )
                {
                    #ifdef VERBOSE
                        std::cout << "(";
                        for(auto i = m.first.begin(); i != m.first.end(); ++i)
                        {
                            if( i != m.first.begin() )
                                std::cout << " ";
                            std::cout << *i;
                        }
                        std::cout << ") ->(" << j << ") (";
                        for(auto i = state.begin(); i != state.end(); ++i)
                        {
                            if( i != state.begin() ){ std::cout << " "; }
                            std::cout << *i;
                        }
                        std::cout << ")\n";
                    #endif
                    // add an edge if the reached state has at least
                    // one outgoing edge.
                    A.add_edge(m.second,j,entry->second);
                }
            }
            // empty labels vector
            A.clear_labels(m.second);
        }

        // check if the A^p automaton has cycles
        #ifdef VERBOSE
        {
            std::cout << "Number of edges: " << A.no_edges() << "\n";
            std::cout << "### check A^" << p << " automaton cyclicity ###\n";
        }
        #endif

        // open output file
        std::ofstream ofile;
        ofile.open("answer");
        if( DFS_cycle_detection() )
        {
            std::cout << p << "-cycle found! The language width is >=(greater or equal than) " << p << ".\n";
            ofile << 0;
        }
        else
        {
            std::cout << "No " << p << "-cycle found! The language width is <(smaller than) " << p << ".\n";
            ofile << 1;
        }
        ofile.close();
    }
    else
    {
        std::cerr << "invalid no. arguments\n";
        std::cerr << "Format your command as follows: .\\p recognizer minimum_dfa dfa_intervals\n";
        exit(1);
    }

  return 0;
}