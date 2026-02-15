#!/opt/homebrew/bin/python3

import os
from pathlib import Path

def getContent(name):
    header_flag = "DIP_"+name.replace(" ","_").upper()+"_H"
    return f"""#ifndef {header_flag}
#define {header_flag}

namespace snt::dip {{


}}

#endif // {header_flag}
    """
    
if __name__=="__main__":

    dir_nodes = Path("src/snt/dip/nodes")
    
    for file_node in dir_nodes.glob("*.cpp"):
        file_header = Path(file_node.parent/file_node.stem).with_suffix('.h')
        with open(file_header, 'w') as f:
            f.write(getContent(file_node.stem))
            
    
