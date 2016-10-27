/**
Copyright (c) 2013, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "kul/code/compilers.hpp"
#include "maiken/project.hpp"

std::unique_ptr<maiken::Settings> maiken::Settings::instance;

maiken::Settings::Settings(const std::string& s) : kul::yaml::File(s){
    if(root()[LOCAL] && root()[LOCAL][REPO]){
        kul::Dir d(root()[LOCAL][REPO].Scalar());
        if(!d.is() && !d.mk()) KEXCEPT(SettingsException, "settings.yaml local/repo is not a valid directory");
    }
    if(root()[REMOTE] && root()[REMOTE][REPO])
        for(const auto& s : kul::String::SPLIT(root()[REMOTE][REPO].Scalar(), ' '))
            rrs.push_back(s);
    else{
        const std::string& rr = _MKN_REMOTE_REPO_;
        for(const auto& s : kul::String::SPLIT(rr, ' '))
            rrs.push_back(s);
    }
    if(root()[SUPER]){
        kul::File f(root()[SUPER].Scalar());
        if(!f) KEXCEPT(SettingsException, "Super in settings does not exist\n"+file());
    }
    if(root()[COMPILER] && root()[COMPILER][MASK])
        for(const auto& k : kul::code::Compilers::INSTANCE().keys())
            for(const auto& m : root()[COMPILER][MASK][k])
                kul::code::Compilers::INSTANCE().addMask(m.Scalar(), k);
}

maiken::Settings& maiken::Settings::INSTANCE(){
    if(!instance.get()){
        const kul::File f("settings.yaml", kul::user::home("maiken"));
        if(!f.dir().is()) f.dir().mk();
        if(!f.is()){ write(f);}
        instance = std::make_unique<Settings>(kul::yaml::File::CREATE<Settings>(f.full()));
    }
    return *instance.get();
}

bool maiken::Settings::SET(const std::string& s){
    if(kul::File(s).is())           instance = std::make_unique<Settings>(s);
    else
    if(kul::File(s+".yaml").is())   instance = std::make_unique<Settings>(s+".yaml");
    else
    if(kul::File(s, kul::user::home("maiken")).is())
        instance = std::make_unique<Settings>(kul::user::home("maiken").join(s));
    else
    if(kul::File(s+".yaml", kul::user::home("maiken")).is())
        instance = std::make_unique<Settings>(kul::user::home("maiken").join(s+".yaml"));
    else
        return 0;
    return 1;
}

const kul::yaml::Validator maiken::Settings::validator() const{
    using namespace kul::yaml;

    std::vector<NodeValidator> masks;
    for(const auto& s : kul::code::Compilers::INSTANCE().keys())
        masks.push_back(NodeValidator(s, {}, 0, NodeType::LIST));

    NodeValidator compiler("compiler", {
        NodeValidator("mask", masks, 0, NodeType::MAP)
    }, 0, NodeType::MAP);

    return Validator({
        NodeValidator("inc"),
        NodeValidator("path"),
        NodeValidator("local", {
            NodeValidator("repo"),
            NodeValidator("debugger"),
            NodeValidator("lib"),
            NodeValidator("bin")
        }, 0, NodeType::MAP),
        NodeValidator("remote", {
            NodeValidator("repo")
        }, 0, NodeType::MAP),
        NodeValidator("env", {
            NodeValidator("name", 1),
            NodeValidator("mode", 1),
            NodeValidator("value", 1)
        }, 0, NodeType::LIST),
        NodeValidator("file", {
            NodeValidator("type", 1),
            NodeValidator("compiler", 1),
            NodeValidator("linker"),
            NodeValidator("archiver")
        }, 1, NodeType::LIST),
        compiler
    });
}

void maiken::Settings::write(const kul::File& f){

    kul::io::Writer w(f);
    w.write("\n", true);

    w.write("#local:", true);
    w.write("# Optionnaly override local repository directory", true);
    w.write("#    repo: <directory>", true);

    w.write("# Add include directories to every compilation", true);
    w.write("#inc: <directory>", true);
    w.write("# Add library paths when linking every binary", true);
    w.write("#path:    <directory>\n", true);

    w.write("# Modify environement variables for application commands - excludes run", true);
    w.write("#env:", true);
    w.write("#  - name: VAR", true);
    w.write("#    mode: prepend", true);
    w.write("#    value: value", true);

    w.write("file:", true);
#ifdef _WIN32
    w.write("  - type: cpp:cxx:cc:c", true);
    w.write("    archiver: lib", true);
    w.write("    compiler: cl", true);
    w.write("    linker: link", true);
#else
    bool c = kul::env::WHICH("clang");
    w.write("  - type: c", true);
    w.write("    archiver: ar -cr", true);
    w << "    compiler: " << (c?"clang":"gcc") << kul::os::EOL();
    w << "    linker: "   << (c?"clang":"gcc") << kul::os::EOL();
    w.write("  - type: cpp:cxx:cc", true);
    w.write("    archiver: ar -cr", true);
    w << "    compiler: " << (c?"clan":"") << "g++" << kul::os::EOL();
    w << "    linker: "   << (c?"clan":"") << "g++" << kul::os::EOL();
#endif

    w.write("# Other examples", true);
    w.write("#  - type: cu", true);
    w.write("#    archiver: ar -cr", true);
    w.write("#    compiler: nvcc", true);
    w.write("#    linker: nvcc", true);

    w.write("#  - type: m", true);
    w.write("#    archiver: ar -cr", true);
    w.write("#    compiler: g++ -lobjc", true);
    w.write("#    linker: g++", true);

    w.write("#  - type: cs", true);
    w.write("#    compiler: csc", true);
    w.write("#    linker: csc", true);

}
