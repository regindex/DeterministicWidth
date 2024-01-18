#ifndef UTILS_HPP_
#define UTILS_HPP_

#include "automaton.hpp"

// DFA type
//typedef DFA_unidirectional_out_labelled DFA;

// mask vector
const unsigned char mask[]={0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}; 
// return bit in position i
bool iget(const std::string* s, uint i){ return ((*s)[(i)/8]&mask[(i)%8]) ? 1 : 0; }
// set bit in position i
inline void bset1(std::string* s, uint i){ (*s)[(i)/8]=(1) ? char(mask[(i)%8]|(*s)[(i)/8]) : char((~mask[(i)%8])&(*s)[(i)/8]); }
inline void bset0(std::string* s, uint i){ (*s)[(i)/8]=(0) ? char(mask[(i)%8]|(*s)[(i)/8]) : char((~mask[(i)%8])&(*s)[(i)/8]); }

template <typename U>
U read_uint(std::string str)
{
    return static_cast<U>(std::stoull(str));
}

void tokenize(std::string const &str, const char delim, 
            std::vector<std::string> &out) 
{ 
    // construct a stream from the string 
    std::stringstream ss(str); 

    // clear out vector
    out.clear();
    
    // segment str using delim
    std::string s; 
    while (std::getline(ss, s, delim)) { 
        out.push_back(s); 
    } 
}

// simple parser for intermediate file; it reads line by line origin \t destination \t label \n
uint read_interval(std::string input_file, std::vector< std::pair <uint,uint> >& intervals,
                   bool remove_inf_eq_sup = false)
{
    // open stream to input
    std::ifstream input(input_file);
    std::string line;
    const char delim = '\t'; 
    uint beg, end;
    uint max_beg = 0;
    std::vector<std::string> out; 

    while(true)
    {
        std::getline(input, line);
        tokenize(line, delim, out);

        if(out.size() != 2){ break; };

        beg = read_uint<uint>(out[0]);
        end = read_uint<uint>(out[1]);

        if( remove_inf_eq_sup && (beg == end) )
        {
            intervals.push_back(std::make_pair(0,0));
            continue; 
        }
        if( beg > max_beg ){ max_beg = beg; }

        intervals.push_back(std::make_pair(beg,end));
    }

    // close stream to input file
    input.close();  
    // return maximum beginning value
    return max_beg;
}  

// simple parser for intermediate file; it reads line by line origin \t destination \t label \n
template <typename DFA>
void read_min_dfa(std::string input_file, DFA &A)
{
    // open stream to input
    std::ifstream input(input_file);
    std::string line;
    const char delim = ' '; 
    std::vector<std::string> out; 

    // remove first line
    uint origin, dest, label;
    //int label;  
    std::getline(input, line);
    tokenize(line, delim, out);
    A = DFA(read_uint<uint>(out[0]));
    
    // read all edges
    while(true)
    {
        std::getline(input, line);
        tokenize(line, delim, out);

        if(out.size() != 3){ break; };

        origin = read_uint<uint>(out[0]);
        label = read_uint<uint>(out[1]);
        dest = read_uint<uint>(out[2]);

        A.at(origin)->out.insert({label,dest});
    }

    // close stream to input file
    input.close();
} 

void counting_sort(std::vector< std::pair <uint,uint> > &vec, std::vector<uint> &out, uint m)
{
    std::vector<uint> count(m,0);
    for(uint i=0; i<vec.size(); ++i)
    {   
        uint cs = vec[i].first;
        count[cs]++;
    }  
    uint prev_c = count[0];
    count[0] = 0;
    for(uint i=1; i<count.size(); ++i)
    {
        uint tmp = count[i];
        count[i] = count[i-1] + prev_c;
        prev_c = tmp;
    }  
    for (uint i = 0; i < vec.size(); ++i)
    {
        uint cs = vec[i].first;
        uint index = count[cs]++;
        out[index] = i;
    }
}

#endif