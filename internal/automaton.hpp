#ifndef AUTOMATON_HPP_
#define AUTOMATON_HPP_

#include <cstdint>
#include <cassert>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <stack>
#include <fstream>
#include <sstream>
// hash function for unordered sets
#include "hashing_function.hpp"

#ifndef M64
    typedef uint32_t uint;
    typedef int32_t int_t;
    #define U_MAX   UINT32_MAX
    #define I_MAX   INT32_MAX
#else
    typedef uint64_t uint;
    typedef int64_t int_t;
    #define U_MAX   UINT64_MAX
    #define I_MAX   INT64_MAX
#endif

// container for an order agnostic hash function
template <typename Container>
struct container_hash {
    std::size_t operator()(Container const& c) const {
        return hash_unordered_range(c.begin(),c.end());
    }
};

/*
	node of the DFA
*/
struct node
{
	node(){}
	node(char label){ labels.push_back(label); }
	node(char label, uint value){ out.insert({label,value}); }

	void add_edge(char label, uint value){ out.insert({label,value}); }
	void add_label(char label){ labels.push_back(label); }
	
	// out edges
	std::unordered_map<char,uint> out;
	// labels list
	std::vector<char> labels;
};

// class for directed bidirectional unlabeled NFA
template <typename Container>
class DFA_unidirectional_out_labelled{

public:

	// define unordered map type
	typedef std::unordered_map<Container,uint,container_hash<Container>> mapt;
	// empty constructor
	DFA_unidirectional_out_labelled(){ nodes = edges = 0; }
	// constructor 
	DFA_unidirectional_out_labelled(uint n)
	{
		nodes = n;
		//initialize DFA
		DFA.resize(nodes);
	}
	// destructor
	// ~DFA_unidirectional_out_labelled(){}

	node* at(uint i){

		assert(i < nodes);
		// return pointer to node
		return &DFA[i];
	}

	uint no_nodes(){ return nodes; }
	uint no_edges(){ return edges; }

	void add_state(Container& v)
	{
		if (M.find(v) == M.end())
		{
			M.insert({v,nodes++});
			DFA.push_back(node());
		}
	}

	void add_state(Container& v, char label)
	{
		auto entry = M.find(v);
		if (entry == M.end())
		{
			M.insert({v,nodes++});
			DFA.push_back(node(label));
		}
		else
		{
			DFA[entry->second].add_label(label);
		}
	}

	mapt* give_mapping()
	{
		return &M;
	}

	void add_edge(uint i,char key,uint value)
	{
		assert(i < nodes);
		// increase edge number
		edges++;
		// insert edge
		DFA[i].out.insert({key,value});
	}

	void remove_label(uint i,char key)
	{
		assert(i < nodes);
		// delete label
		DFA[i].out.erase(key);
	}

	void clear()
	{
		DFA.resize(0);
		DFA.shrink_to_fit();
		M.clear();
	}

	void clear_nodes()
	{
		DFA.resize(0);
		DFA.shrink_to_fit();
	}

	void clear_labels(uint i)
	{
		assert(i < nodes);
		// clear label vector
		DFA[i].labels.clear();
	}

private:
	// number of nodes in the DFA
	uint nodes;
	// number of edges in the DFA
	uint edges;
	// vector containing all edges
	std::vector<node> DFA;
	// mapping vector -> position in DFA vector
	mapt M;
};

#endif