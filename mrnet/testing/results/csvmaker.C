#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

int main() {

   for (int k = 0; k < 4; k++) {
      std::stringstream s;
      s << "data" << k << ".csv";
      std::ofstream out(s.str().c_str());

      for (unsigned int i = 1; i < 1201; i++) {
         std::stringstream ss;
         ss << "LN_stats_" << k << "_" << i;
         std::string fname = ss.str();
         std::ifstream in(fname.c_str());
         std::string line;
         while (getline(in, line)) {
            out << line.substr(line.find(":") + 2) << ",";
         }
         out << "\n";
         in.close();
      }

      out.close();
   }
   return 0;
}
