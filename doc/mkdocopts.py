import sys, re

if __name__ == "__main__":

    if len(sys.argv) != 2:
        raise Exception("Invalid number of arguments")

    opts = []

    with open(sys.argv[1], "r") as file:
        desc_open = False
        desc = []
        linenum = 0
        for line in file:
            linenum += 1
            if not desc_open \
               and line.strip().startswith("/*!"):  # start doc string
                   if line.strip() != "/*!":
                       raise Exception(
                               "Invalid start of docstring at line {}. "\
                               "Docstring must start with "\
                               "'/*!' only".format(linenum))
                   desc_open = True
            elif desc_open \
                 and line.strip().startswith("*/"):  # end doc string
                     if line.strip() != "*/":
                         raise Exception(
                                 "Invalid end of docstring at line {}. "\
                                 "Docstring must end with "\
                                 "'*/' only".format(linenum))
                     opts.append(desc)
                     desc = []
                     desc_open = False
            elif desc_open:  # collect opt desc
                desc.append(line)


        
    with open('cboolector_options.rst', 'w') as file:
        #file.write("Options\n")
        #file.write("-------\n\n")
        for o in opts:
            for l in o:
                file.write(l)
            file.write("\n\n")


#    with open('pyboolector_options.rst', 'w') as file:
