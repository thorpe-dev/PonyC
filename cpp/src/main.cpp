#include <iostream>
#include <fstream>
#include <tuple>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#pragma GCC diagnostic ignored "-Wweak-vtables"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconditional-uninitialized"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wc++98-compat"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#pragma GCC diagnostic pop

#include "parser.hpp"
#include "type_checker.hpp"
#include "code_gen.hpp"

#define FILE_EXTENSION ".pony"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

using namespace std;

typedef string program_name;

string read_file(string file_name);
vector<tuple<program_name,string>>* get_files_directory(string dir);

int main(int argc, char** argv) {
    string stages =
    R"(Stage 1: parser
    Stage 2: typer
    Stage 3: code-gen)";

    po::options_description options("Allowed Options");
    options.add_options()
        ("help", "print usage message")
        ("version", "Version")
        ("stage", po::value<int>(), (string("what stage of the compiler to run to (mostly used for debugging)\n") + stages).c_str())
        ("input", po::value< vector<string>>(), "List of files/directories to compile")
        ("output", po::value<string>(), "Output file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);
    

    if (vm.count("help")) {
        cout << options << endl;
        exit(EXIT_SUCCESS);
    }
    
    if (vm.count("version")) {
        cout << "1.0.0 - C++ barebones" << endl;
        exit(EXIT_SUCCESS);
    }
    
    if (!vm.count("input")) {
        cerr << "Error: no input file specified" << endl;
        exit(EXIT_FAILURE);
    }
    
    int stage = INT32_MAX;
    
    if (vm.count("stage"))
        stage = vm["stage"].as<int>();
    
    // Discussion needed on multi-file compilation
    string input = vm["input"].as<vector<string>>().front();
    
    auto program_text = get_files_directory(input);
    auto parsedAST = vector<AST*>();

    for(auto & prog : *program_text) {
            
        Parser* p = new Parser(get<0>(prog),get<1>(prog));
        AST* ast;
        cout << "Parsing file: " << get<0>(prog) << endl;
        ast = p->parse();
        if (p->error_list.size() > 0) {
            cout << "Errors detected, continuing parsing remainder" << endl;
            continue;
        }
        
        parsedAST.push_back(ast);
        delete p;
    }
    
    if (stage == 1)
        return EXIT_SUCCESS;
    
    //Type check!
    auto typeChecker = new TypeChecker(parsedAST);
    typeChecker->typeCheck();
    
    return EXIT_SUCCESS;
}

// TODO:
// handle files being missing
static string read_file(fs::path path) {
    ifstream infile(path.string().c_str());
    stringstream stream;
    stream << infile.rdbuf();
    
    return string(stream.str());
}

static void recurse_dir(fs::path p, vector<tuple<program_name,string>>* vec) {
    
    if (!fs::exists(p)) {
        cout << "Directory " << p.root_path() << " not found" << endl;
        exit(EXIT_FAILURE);
    }
        
    fs::directory_iterator end_it;
    
    for(fs::directory_iterator itr(p); itr != end_it; itr++) {
        if (fs::is_directory(itr->status())) {
            recurse_dir(itr->path(), vec);
        } else if (itr->path().extension() == FILE_EXTENSION) {
            vec->push_back(make_tuple(itr->path().string(),
                                 read_file(itr->path())));
        }
    }
}

vector<tuple<program_name,string>>* get_files_directory(string dir) {
    fs::path p(dir);
    auto vec = new vector<tuple<program_name,string>>();
    
    recurse_dir(p, vec);
    
    return vec;
}


