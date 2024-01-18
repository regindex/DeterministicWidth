#!/usr/bin/env python3

import sys, time, argparse, subprocess, os.path, os, glob, math

Description = """
Tool to compute the deterministic width of a regular language.
"""

def main():
    parser = argparse.ArgumentParser(description=Description, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('input', help='a regular expression (optionally a DFA)', type=str)
    parser.add_argument('--DFA', help='take in input a DFA instead of a regexp', action='store_true')
    parser.add_argument('--Wheeler',  help='check if the language is Wheeler (def. False)',action='store_true')
    parser.add_argument('--p', help='check if the deterministic width is < (smaller) or >= (greater equal) than p', type=int)
    parser.add_argument('--exact',  help='find the exact deterministic width (def. False)',action='store_true')
    parser.add_argument('--memory',  help='minimize memory footprint (def. False)',action='store_true')
    parser.add_argument('--keep',  help='keep intermediate files (def. False, debug only)',action='store_true')
    #parser.add_argument('--stats',  help='print stats to file (def. False)',type=str, default="empty")
    parser.add_argument('--verbose',  help='activate verbose mode (def. False)',action='store_true')
    args = parser.parse_args()
    # Wheeler option has been activated
    if args.Wheeler:
        args.p = 2
    # check selected width
    if((not args.exact) and (args.p < 2)):
        print("The width to check must be >= 2.")
        exit(1)

    logfile_name = args.input + ".log"
    if( not args.DFA ):
        logfile_name = "data/regexp.log"
    # get main directory
    args.main_dir = os.path.split(sys.argv[0])[0]
    logfile_name = os.path.join(args.main_dir, logfile_name)
    if args.verbose:
        print("Sending logging messages to file:", logfile_name)

    # set-up executables
    dfamin_exe    = os.path.join(args.main_dir, "external/dfaMinimizationComparison/Cpp/Modified/minimizer.x")  
    pruning_exe   =  os.path.join(args.main_dir, "build/prune.x")
    partref_exe   =  os.path.join(args.main_dir, "external/finite-automata-partition-refinement/build/part-ref32.x")
    doublingmerge_exe =  os.path.join(args.main_dir, "external/DFAgen-suffixdoubling/suffix-doubling-on-pruned-graphs")
    partrefmerge_exe =  os.path.join(args.main_dir, "external/finite-automata-partition-refinement/build/merge32.x")
    reg_to_dfa_exe =  os.path.join(args.main_dir, "external/RegexpToAutomaton/regToAutomaton")
    det_width_exe = os.path.join(args.main_dir, "build/det-width.x")
    det_width_mem_exe = os.path.join(args.main_dir, "build/det-width-rm.x")

    if args.memory:
        det_width_exe = det_width_mem_exe
    
    with open(logfile_name,"a") as logfile:

        start0 = start = time.time()

        if( not args.DFA ):

            # add dollar to regexp
            args.input = "($)" + "(" + args.input + ")"
            if args.verbose:
                print("Computing the minimum DFA of the regexp")
            # compute minimum DFA
            command = "{exe} {regexp} --DFAmin".format(exe=reg_to_dfa_exe,regexp=args.input)
            with open(os.path.join(args.main_dir, "data/regexp.mdfa"),"w+") as dfa_file:
                subprocess.call(command.split(), stdout=dfa_file)
            # set the resulting DFA as the input
            #args.input = "data/regexp.mdfa"
            args.input = os.path.join(args.main_dir, "data/regexp.mdfa")

            if args.verbose:
                print("Elapsed time: {0:.4f}".format(time.time()-start))

        ############################################

        if( args.DFA ):

            start = time.time()

            # compute the minimum dfa        
            command = "{exe} -in < {input} > {output}".format(exe=dfamin_exe, input=args.input, output=args.input+".min")
            if args.verbose:
                print("==== computing minimum DFA. Command: ", command)

            with open(args.input, 'rb', 0) as a, open(args.input+".min", 'w') as b:
                rc = subprocess.call(dfamin_exe, stdin=a, stdout=b)

            if args.verbose:
                print("Elapsed time: {0:.4f}".format(time.time()-start))
            args.input = args.input+".min"

        ############################################

        start = time.time()

        command = "{exe} {input} {mindfa} {maxdfa}".format(exe=pruning_exe, input=args.input, mindfa=args.input+".prmin", maxdfa=args.input+".prmax")

        if args.verbose:
            print("==== pruning minimum DFA. Command: ", command)
        if(execute_command(command,logfile,logfile_name)!=True):
            return
        if args.verbose:
            print("Elapsed time: {0:.4f}".format(time.time()-start))

        ############################################

        start = time.time()

        no_nodes = 0
        source = 0
        with open(args.input+".prmin", "rb") as file:
            try:
                file.seek(-2, os.SEEK_END)
                while file.read(1) != b'\n':
                    file.seek(-2, os.SEEK_CUR) 
            except OSError:
                file.seek(0)
            last_line = file.readline().decode()
            no_nodes = int(last_line.split(" ")[0])
            source = int(last_line.split(" ")[2])

        command = "{exe} {input} {output} {nodes} {source} 0 1 0 1 0 1".format(exe=partref_exe, input=args.input+".prmin", output=args.input+".infima", nodes=no_nodes, source=source)

        if args.verbose:
            print("==== compute infima strings DFA. Command: ", command)
        if(execute_command(command,logfile,logfile_name)!=True):
            return

        command = "{exe} {input} {output} {nodes} {source} 0 1 1 1 0 1".format(exe=partref_exe, input=args.input+".prmax", output=args.input+".suprema", nodes=no_nodes, source=source)

        if args.verbose:
            print("==== compute suprema strings DFA. Command: ", command)
        if(execute_command(command,logfile,logfile_name)!=True):
            return

        if args.verbose:
            print("Elapsed time: {0:.4f}".format(time.time()-start))

        ############################################

        start = time.time()

        '''
        command = "{exe} {input1} {input2}".format(exe=doublingmerge_exe, input1=args.input+".infima", input2=args.input+".suprema")
        print("==== merging infima and suprema pruned DFAs. Command: ", command)

        with open(args.input+".interval", 'w') as b:
            rc = subprocess.call(command.split(), stdout=b)
        '''
        no_states = source = 0
        with open(args.input,'r') as fp:
            header = fp.readline().split()
            no_states = header[0]
            source = header[2]

        command = "{exe} {input1} {input2} {output} {states} {source_state} 0 1".format(exe=partrefmerge_exe, input1=args.input+".infima",
                                                                                        input2=args.input+".suprema", output=args.input+".interval",
                                                                                        states = no_states, source_state = source)
        if args.verbose:
            print("==== merging infimum and supremum pruned DFAs. Command: ", command)
        if(execute_command(command,logfile,logfile_name)!=True):
            return

        if args.verbose:
            print("Elapsed time: {0:.4f}".format(time.time()-start))

        ############################################

        start = time.time()

        if not args.exact:

            if args.Wheeler:

                command = "{exe} {p} {dfa} {interval}".format(exe=det_width_exe, p=args.p, dfa=args.input, interval=args.input+".interval")

                if args.verbose:
                    print("==== compute A^2 pruned automaton and check language Wheelerness. Command: ", command)
                if(execute_command(command,logfile,logfile_name)!=True):
                    return

                if args.verbose:
                    print("Elapsed time: {0:.4f}".format(time.time()-start))

                with open("answer", 'rb', 0) as a:
                    answer = a.readline().decode()
                    if(answer == "1"):
                        print("#########")
                        print('\033[95m' + "    The regular language is Wheeler")
                        print('\033[0m' + "#########")
                    else:
                        print("#########")
                        print('\033[95m' + "    The regular language is NOT Wheeler")
                        print('\033[0m' + "#########")

            else:

                command = "{exe} {p} {dfa} {interval}".format(exe=det_width_exe, p=args.p, dfa=args.input, interval=args.input+".interval")

                if args.verbose:
                    print("==== compute A^p pruned automaton and check language width. Command: ", command)
                if(execute_command(command,logfile,logfile_name)!=True):
                    return

                if args.verbose:
                    print("Elapsed time: {0:.4f}".format(time.time()-start))

                with open("answer", 'rb', 0) as a:
                    answer = a.readline().decode()
                    if(answer == "1"):
                        print("#########")
                        print('\033[95m' + "    The deterministic width is <(smaller than)",args.p)
                        print('\033[0m' + "#########")
                    else:
                        print("#########")
                        print('\033[95m' + "    The deterministic width is  >=(greater or equal than)",args.p)
                        print('\033[0m' + "#########")
        else:

            no_states = 0
            with open(args.input+".interval",'r') as fp:
                no_states = sum(1 for line in fp)

            language_width = 0
            high = int(no_states)
            low = 1
            middle = 2

            while True:

                command = "{exe} {p} {dfa} {interval}".format(exe=det_width_exe, p=middle, dfa=args.input, interval=args.input+".interval")
                #print(command)
                if(execute_command(command,logfile,logfile_name)!=True):
                    return

                with open("answer", 'rb', 0) as a:
                    answer = a.readline().decode()
                    if(answer == "1"):
                        high = middle - 1
                        break
                        #print("*** The width p of the language recognized by the input is <(smaller than)",args.width)
                    else:
                        low = middle
                        #print("*** The width p of the language recognized by the input is >=(greater or equal than)",args.width)

                middle *= 2
                if middle > no_states:
                    high = no_states
                    break

            #if args.verbose:
            #    print("p is between:",low,"and",high)

            while low != high:

                middle = math.ceil((low+high)/2)

                command = "{exe} {p} {dfa} {interval}".format(exe=det_width_exe, p=middle, dfa=args.input, interval=args.input+".interval")
                #print(command)
                if(execute_command(command,logfile,logfile_name)!=True):
                    return

                with open("answer", 'rb', 0) as a:
                    answer = a.readline().decode()
                    if(answer == "1"):
                        high = middle - 1
                    else:
                        low = middle

            print("#########")
            print("    Regular language deterministic width:",'\033[95m' + "p =",low)
            print('\033[0m' + "#########")
            #language_width = low

            #if args.stats != "empty":
                # print stats
            #    with open(os.path.join(args.main_dir,args.stats),"w+") as fp:
            #        fp.write(str(language_width))

        ############################################

        print("### Total elapsed time: {0:.4f}".format(time.time()-start0))

        ############################################

        if not args.keep:
            # remove intermediate files
            for filename in glob.glob(os.path.join(args.main_dir, "data/regexp.mdfa*")):
                os.remove(filename) 
            os.remove(os.path.join(args.main_dir, logfile_name))


# execute command: return True is everything OK, False otherwise
def execute_command(command,logfile,logfile_name,env=None):
  try:
    #subprocess.run(command.split(),stdout=logfile,stderr=logfile,check=True,env=env)
    subprocess.check_call(command.split(),stdout=logfile,stderr=logfile,env=env)
  except subprocess.CalledProcessError:
    print("Error executing command line:")
    print("\t"+ command)
    print("Check log file: " + logfile_name)
    return False
  return True

if __name__ == '__main__':
    main()