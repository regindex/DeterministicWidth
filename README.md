# DeterministicWidth

### Description

This tool implements a software to compute the deterministic co-lex width of a regular language.

### Requirements

* A modern C++17 compiler such as `g++` version 8.4 or higher.
* The cmake build system, version \geq 3.10.
* A Linux 64-bit operating system.
* A modern Python 3 release.

### Download

To clone the repository, run the following commands:

```console
git clone https://github.com/regindex/DeterministicWidth
git submodule update --init --recursive
```

### Compile

You can compile the code using the following python3 script:

```console
python3 compile.py
```

### Input

The tool can take in input a regular expression; the supported operators are "()?+|*",
```
"a(ac*a)|(bc*b)"
```
The tool can also take in input a DFA. In the first line, the file has to contain information about the number of states, number of edges, source state, and number of accepting states. The rest of the file contains an edge per line (origin - label - destination) and a list of accepting states separated by newline characters. We further ask that the source has a single incoming edge labeled by the smallest label in the automaton (see below for an example).
```
9 12 0 2
0 0 1
1 1 2
2 1 3
2 2 4
3 1 5
3 3 6
4 2 7
4 3 8
6 3 6
6 1 5
8 3 8
8 2 7
5
7
```

### Usage

```
usage: determWidth.py [-h] [--DFA] [--Wheeler] [--p P] [--exact] [--memory] [--keep] [--verbose] input

Tool to compute the deterministic width of a regular language.

positional arguments:
  input       a regular expression (optionally a DFA)

optional arguments:
  -h, --help  show this help message and exit
  --DFA       take in input a DFA instead of a regexp
  --Wheeler   check if the language is Wheeler (def. False)
  --p P       check if the deterministic width is < (smaller) or >= (greater equal) than p
  --exact     find the exact deterministic width (def. False)
  --memory    minimize memory footprint (def. False)
  --keep      keep intermediate files (def. False, debug only)
  --verbose   activate verbose mode (def. False)
```

### Run on example data

```console
// take in input a regular expression
python3 determWidth.py --exact "a(ac*a)|(bc*b)"
// take in input a DFA
python3 determWidth.py --DFA --exact data/regexp.dfa 
```

### External resources

* [finite-automata-partition-refinement](https://github.com/regindex/finite-automata-partition-refinement.git)
* [DFA-suffix-doubling](https://github.com/regindex/DFA-suffix-doubling.git)
* [regexpToAutomaton](https://github.com/regindex/RegexpToAutomaton.git)
* [dfaMinimizationComparison](https://github.com/WalkerCodeRanger/dfaMinimizationComparison.git)
* [malloc_count](https://github.com/bingmann/malloc_count)
* [boost](https://www.boost.org/)

### References and Citations 

[1] Ruben Becker, Davide Cenzato, Sung-Hwan Kim, Bojana Kodric, Alberto Policriti, Nicola Prezza: Optimal Wheeler Language Recognition. SPIRE 2023: 62-74 ([go to the paper](https://link.springer.com/chapter/10.1007/978-3-031-43980-3_6))

[2] Ruben Becker, Davide Cenzato, Sung-Hwan Kim, Bojana Kodric, Alberto Policriti, Nicola Prezza: Optimal Wheeler Language Recognition. CoRR abs/2306.04737 (2023) ([go to the paper](https://arxiv.org/abs/2306.04737))

Please, if you use this tool in an academic setting, cite the following papers:

    @inproceedings{BeckerCKKPP23,
      author       = {Ruben Becker and
                      Davide Cenzato and
                      Sung{-}Hwan Kim and
                      Bojana Kodric and
                      Alberto Policriti and
                      Nicola Prezza},
      title        = {Optimal {Wheeler} Language Recognition},
      booktitle    = {In Proceedings of 30th International Symposium on String Processing and 
                      Information Retrieval, {SPIRE} 2023},
      series       = {Lecture Notes in Computer Science},
      volume       = {14240},
      pages        = {62--74},
      year         = {2023},
      doi          = {10.1007/978-3-031-43980-3\_6},
    }

   	@article{BeckerCKKPP23arXiv,
	  author       = {Ruben Becker and
	                  Davide Cenzato and
	                  Sung{-}Hwan Kim and
	                  Bojana Kodric and
	                  Alberto Policriti and
	                  Nicola Prezza},
	  title        = {Optimal Wheeler Language Recognition},
	  journal      = {CoRR},
	  volume       = {abs/2306.04737},
	  year         = {2023},
	  doi          = {10.48550/ARXIV.2306.04737},
	}

### Funding

This project has received funding from the European Research Council (ERC) under the European Union’s Horizon Europe research and innovation program, project REGINDEX, grant agreement No 101039208
