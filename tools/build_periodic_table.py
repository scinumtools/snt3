"""
Run as:
python3 tools/build_materials.py
from the main directory
"""
from dotenv import load_dotenv
import numpy as np
import os, sys
import requests
from bs4 import BeautifulSoup
from pathlib import Path
import json
import re

env_path = Path(__file__).parent.parent / "settings.env"
load_dotenv(dotenv_path=env_path)

def build_periodic_table():
    
    def load_elements(url):
        response = requests.get(url)
        if response.status_code == 200:
            return response.text
        else:
            return None
            
    html_data = load_elements("https://physics.nist.gov/cgi-bin/Compositions/stand_alone.pl")
    if html_data:
        
        text = ["""
#ifndef MAT_PERIODIC_TABLE_H
#define MAT_PERIODIC_TABLE_H

namespace mat {

  class PeriodicTable {
  };
  // https://physics.nist.gov/cgi-bin/Compositions/stand_alone.pl #

  static constexpr std::array<std::string_stream, 2> PT_HEADER = {"Z", "A"}; 

  struct Isotope {
    std::string_view symbol;
    unsigned int protons;
    unsigned int neutrons;
    double atomic_number;
    double natural_abundance;
  };

  static constexpr std::array<Isotope, 360> PT_DATA = {{
        """]
        
        soup = BeautifulSoup(html_data, "html.parser")
        table = soup.find("table")
        rows = table.find_all("tr")
        Z, A, M, NA, symbol = 0, 0, 0, 0, None
        numRows = 0
        for row in rows:
            dts = row.find_all('td',{'align':'right'})
            if len(dts)==1:
                A = int(dts[0].get_text().strip())
            elif len(dts)==2:
                Z = int(dts[0].get_text().strip())
                A = int(dts[1].get_text().strip())
                symbol = row.find('td',{'align':'center'}).get_text().strip()
            else:
                continue
            dts = row.find_all('td',{'align':None})
            # relative atomic masses
            M = float(re.sub(r"(\s+|\([0-9#]+\))", '', dts[0].get_text()))
            # isotopic composition
            if ic := dts[1].get_text().strip():
                NA = float(re.sub(r"(\s+|\([0-9#]+\))", '', ic))
            else:
                NA = 0
            symbols, Zs, As, Ms, NAs = f"\"{symbol}\",", f"{Z},", f"{A},", f"{M},", f"{NA}"
            text.append(f"     {{{symbols:7s}{Zs:6s}{As:6s}{Ms:16s}{NAs:13s}}},")
            numRows += 1;
        text = "\n".join(text)

        text += """
  }};
  static constexpr size_t PT_NUM_DATA = """+str(numRows)+""";
  
} // namespace mat

#endif // MAT_PERIODIC_TABLE_H        
        """
        
        # save the new version of the code
        path_units = os.environ['DIR_SOURCE']+"/snt/mat"
        path_list = f'{path_units}/periodic_table.h'
        assert os.path.isdir(path_units)
        with open(path_list,'w') as f:
            f.write(text)
        print(path_list)

if __name__=="__main__":
    build_periodic_table()
