#include "rete_parser.h"

namespace Rete {

  bool rete_get_exit() {
    return g_rete_exit;
  }

  int rete_parse_file(Carli::Agent &agent, const string &filename, const std::string &source_path) {
    if(filename.empty())
      return -1;

    string source_path_full = source_path;
    string filename_part = filename;
    string filename_full;
    const size_t filename_slash_pos = filename.find_last_of("/\\");
    if(filename_slash_pos != string::npos) {
      if(filename[0] == '/' || filename[0] == '\\')
        source_path_full.clear();
      source_path_full += filename.substr(0, filename_slash_pos + 1);
      filename_part = filename.substr(filename_slash_pos + 1);
    }
    filename_full = source_path_full + filename_part;

    //cerr << "Sourcing '" << filename_part << "' from '" << source_path_full << '\'' << endl;

    FILE * file = fopen(filename_full.c_str(), "r");
    if(!file) {
      std::cerr << "Failed to open file '" << filename_full << "' for parsing." << std::endl;
      return -1;
    }

    yyscan_t yyscanner;
    if(retelex_init(&yyscanner)) {
      fclose(file);
      return -1;
    }

    int rv = 0;

    reterestart(file, yyscanner);
    reteset_lineno(1, yyscanner);

    Rete::Agenda::Locker locker(agent.get_agenda());

    // retelex();
    do {
      rv = reteparse(yyscanner, agent, filename_full, source_path_full);
      if(rv)
        break;
    } while (!feof(reteget_in(yyscanner)));

    retelex_destroy(yyscanner);
    fclose(file);

    return rv;
  }

  int rete_parse_string(Carli::Agent &agent, const string &str, int &line_number) {
    int rv = 0;

    yyscan_t yyscanner;
    if(retelex_init(&yyscanner))
      return -1;

    YY_BUFFER_STATE buf = rete_scan_string(str.c_str(), yyscanner);
    reteset_lineno(line_number, yyscanner);

    // retelex();
    rv = reteparse(yyscanner, agent, "", "");

    line_number = reteget_lineno(yyscanner) + 1;

    rete_delete_buffer(buf, yyscanner);
    retelex_destroy(yyscanner);

    Rete::Agenda::Locker locker(agent.get_agenda());

    return rv;
  }

  void rete_set_exit() {
    g_rete_exit = true;
  }

}
