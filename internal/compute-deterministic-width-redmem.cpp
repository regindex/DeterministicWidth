#include "automaton.hpp"
#include "utils.hpp"
#include "hashing_function.hpp"

// DFA type
typedef DFA_unidirectional_out_labelled<std::unordered_set<uint>> DFA; // DFA type

// GLOBAL VARIABLES FOR RECURSIVE FUNCTION //
std::vector<uint>* v; // pointer to vector in L data structure
char label; // label
uint p, N = 0; // language width and number of states
std::vector< std::pair<uint,uint> > intervals; // intervals vector
DFA::mapt mapping; // mapping between states and ids
std::vector<uint> freq(128,0); // character frequencies
std::vector<char> alph; // aphabet vector
DFA M = DFA(); // global minimum DFA
std::string* visited; // visited states bitvector
std::string*  recStack; // recursion stack bitvector

// recursive p level for loop
void recursive_construction(uint rightLim, uint l) 
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
            // check the overlap
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
            // insert last element in set
            state.insert((*v)[j-1]);
            // add the empty state to the pruned automaton
            if(mapping.find(state) == mapping.end())
            {
                mapping.insert({state,N++});
                freq[uint(label)]++;
            }
        }
    }
}

// DFS function to find if a cycle exists
bool isCyclicSim(uint c, const std::unordered_set<uint>& s)
{
    // define global state set
    static std::unordered_set<uint> curr;

    if(!iget(visited,c))
    {
        // mark state as visited
        bset1(visited,c);
        bset1(recStack,c);

        // visit all adjacent states
        for(uint i=0;i<alph.size();++i)
        {
            // clear current state set
            curr.clear();
            // fill state set
            for (const auto& e: s)
                if( M.at(e)->out.find(alph[i]) != M.at(e)->out.end() )
                    curr.insert(M.at(e)->out[alph[i]]);

            // skip if we map to an illegal state
            if(curr.size() < p)
                continue;
            // check if the new node is present
            auto F = mapping.find(curr);
            if(F != mapping.end())
            {
                // recursive call
                if (!iget(visited,F->second) && isCyclicSim(F->second, F->first))
                    return true;
                else if (iget(recStack,F->second))
                    return true;
            }
        }
    }

    // remove the state from recursion stack
    bset0(recStack,c);
    return false;
}

bool DFS_cycle_detection_sim()
{
    // mark all states as not visited
    visited = new std::string((N/8)+1,0);
    recStack = new std::string((N/8)+1,0);

    // recursive function call
    for (auto& m: mapping)
    {
        // start visit in the current state
        if (!iget(visited,m.second) && isCyclicSim(m.second, m.first))
            return true;
    }
    // if no cycle has been detected return false
    return false;
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
        uint max_beg = read_interval(in_interval,intervals,true);
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
        // find alphabet
        for(uint i=0;i<128;++i)
            if(freq[i] > 0)
                alph.push_back(char(i));
        // clear frequency vector
        freq.clear();

        #ifdef VERBOSE
        {
            uint node_count = 0;
            for (auto& m: mapping)
            {
                std::cout << node_count++ << ": (";
                for (auto i = m.first.begin(); i != m.first.end(); ++i) {
                    if( i != m.first.begin() ){ std::cout << " "; }
                    std::cout << *i;
                }
                std::cout << ")\n";
            }
            std::cout << "Number of states: " << N << "\n";
            std::cout << "Alphabet size: " << alph.size() << "\n";
        }
        #endif

        // open output file
        std::ofstream ofile;
        ofile.open("answer");
        if( DFS_cycle_detection_sim() )
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