#include <iostream>
#include <fstream>
#include <tuple>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include "parser.hpp"
#include "code_gen.hpp"

#define FILE_EXTENSION ".pony"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

using namespace std;

string stages = 
R"(Stage 1: parser
Stage 2: typer
Stage 3: code-gen)";

string read_file(string file_name);
vector<tuple<string,string*>>* get_files_directory(string dir);

int main(int argc, char** argv) {
    
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
    
    vector<tuple<string,string*>>* program_text = get_files_directory(input);
    vector<AST*>* parsedAST = new vector<AST*>(program_text->size());
    
    for(auto & prog : *program_text) {
            
        Parser* p = new Parser(get<1>(prog));
        AST* ast;
        cout << "Parsing file: " << get<0>(prog) << endl;
        ast = p->parse();
        if (p->error_list->size() > 0)
            cout << "Errors detected" << endl;
        
        parsedAST->push_back(ast);
        delete p;
    }
    
    if (stage == 1)
        return EXIT_SUCCESS;
    
    //Type check!
    
    
    
    

        // If there's a stage given, dump it to stdin
        // Perhaps output to a .parser/.typer/.code_gen or similar file instead?
        if (vm.count("stage")) {
            switch(vm["stage"].as<int>()) {
                case 2:
                    cout << "Not yet implemented" << endl;
    //                cout << typer::type(parser::parse(program_text));
                    break;
                case 3:
                    cout << "Not yet implemented" << endl;
    //                cout << code_gen::generate(typer::type(parser::parse(program_text)));
                    break;
                default:
                    cerr << "Use of undefined stage" << endl;
                    exit(EXIT_FAILURE);
            }
        }
        else {
            // Else compile the code! Lets do this!
            ofstream outfile;
            if (vm.count("output")) {
                outfile.open(vm["output"].as<string>().c_str());
            }
            else {
                // TODO:
                // This is not what we want - we'll be outputting llvm IR here
                // a.out is what gcc/clang uses for binary files
                outfile.open("a.out");
            }
            
    //        outfile << code_gen::generate(typer::type(parser::parse(program_text)));
        }
        
//        delete p;
    
    return EXIT_SUCCESS;
}

// TODO:
// handle files being missing
string* read_file(fs::path path) {
    ifstream infile(path.string().c_str());
    stringstream stream;
    stream << infile.rdbuf();
    
    return new string(stream.str());
}

void recurse_dir(fs::path p, vector<tuple<string,string*>>* vec) {
    if (!fs::exists(p)) {
        cout << "Directory " << p.root_path() << " not found" << endl;
        exit(EXIT_FAILURE);
    }
    
    if (fs::is_regular(p)) {
        vec->push_back(make_tuple(p.string(),
                                  read_file(p)));
        return;
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

vector<tuple<string,string*>>* get_files_directory(string dir) {
    fs::path p(dir);
    auto vec = new vector<tuple<string,string*>>();
    
    recurse_dir(p, vec);
    
    return vec;
}


