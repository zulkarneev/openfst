// See www.openfst.org for extensive documentation on this weighted
// finite-state transducer library.
//
// Creates binary FSTs from simple text format used by AT&T.

#include <cstring>

#include <fstream>
#include <istream>
#include <memory>
#include <string>

#include <fst/log.h>
#include <fst/script/compile.h>

#include <kaldi-crypt.h>

DEFINE_bool(acceptor, false, "Input in acceptor format");
DEFINE_string(arc_type, "standard", "Output arc type");
DEFINE_string(fst_type, "vector", "Output FST type");
DEFINE_string(isymbols, "", "Input label symbol table");
DEFINE_string(osymbols, "", "Output label symbol table");
DEFINE_string(ssymbols, "", "State label symbol table");
DEFINE_bool(keep_isymbols, false, "Store input label symbol table with FST");
DEFINE_bool(keep_osymbols, false, "Store output label symbol table with FST");
DEFINE_bool(keep_state_numbering, false, "Do not renumber input states");
DEFINE_bool(allow_negative_labels, false,
            "Allow negative labels (not recommended; may cause conflicts)");


int main(int argc, char **argv) {
  namespace s = fst::script;
  using fst::SymbolTable;
  using fst::SymbolTableTextOptions;

  string usage = "Creates binary FSTs from simple text format.\n\n  Usage: ";
  usage += argv[0];
  usage += " [text.fst [binary.fst]]\n";

  std::set_new_handler(FailedNewHandler);
  SET_FLAGS(usage.c_str(), &argc, &argv, true);
  if (argc > 3) {
    ShowUsage();
    return 1;
  }

  string source = "standard input";

  crypt_ifstream<char> fstrm;

  if (argc > 1 && strcmp(argv[1], "-") != 0) {
    fstrm.open(argv[1]);
    if (!fstrm) {
      LOG(ERROR) << argv[0] << ": Open failed, file = " << argv[1];
      return 1;
    }
    source = argv[1];
  }

  std::istream &istrm = fstrm.is_open() ? static_cast<std::istream&>(fstrm) : static_cast<std::istream&>(*(new crypt_stdistream<char>()));
  // if (!fstrm.is_open()) {
  //   crypt_stdistream<char> crypt_stdin;
  //   istrm = static_cast<std::istream&>(crypt_stdin);
  // }

  const SymbolTableTextOptions opts(FLAGS_allow_negative_labels);

  std::unique_ptr<const SymbolTable> isyms;
  if (!FLAGS_isymbols.empty()) {
    isyms.reset(SymbolTable::ReadText(FLAGS_isymbols, opts));
    if (!isyms) return 1;
  }

  std::unique_ptr<const SymbolTable> osyms;
  if (!FLAGS_osymbols.empty()) {
    osyms.reset(SymbolTable::ReadText(FLAGS_osymbols, opts));
    if (!osyms) return 1;
  }

  std::unique_ptr<const SymbolTable> ssyms;
  if (!FLAGS_ssymbols.empty()) {
    ssyms.reset(SymbolTable::ReadText(FLAGS_ssymbols));
    if (!ssyms) return 1;
  }

  const string dest = argc > 2 ? argv[2] : "";

  s::CompileFst(istrm, source, dest, FLAGS_fst_type, FLAGS_arc_type,
                isyms.get(), osyms.get(), ssyms.get(), FLAGS_acceptor,
                FLAGS_keep_isymbols, FLAGS_keep_osymbols,
                FLAGS_keep_state_numbering, FLAGS_allow_negative_labels);

  return 0;
}
