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
#ifndef _MAIKEN_PROJECT_HPP_
#define _MAIKEN_PROJECT_HPP_

#include "kul/os.hpp"
#include "kul/log.hpp"
#include "kul/yaml.hpp"

#include "maiken/defs.hpp"

namespace maiken{

class Application;

class ProjectException : public kul::Exception{
    public:
        ProjectException(const char*f, const uint16_t& l, const std::string& s) : kul::Exception(f, l, s){}
};

class Projects;
class KUL_PUBLISH Project : public kul::yaml::File, public Constants{
    friend class Projects;
    private:
        const kul::Dir m_dir;
    protected:      
    public: 
        Project(const kul::Dir& d) : kul::yaml::File(kul::Dir::JOIN(d.real(), "mkn.yaml")), m_dir(d.real()){}
        Project(const Project& p) : kul::yaml::File(p), m_dir(p.m_dir){}
        const kul::Dir& dir() const { return m_dir; }
        const kul::yaml::Validator validator() const;
        friend class maiken::Application;
        friend class kul::yaml::File;
};

class Projects{
    private:
        std::vector<std::unique_ptr<Project>> m_pps;
        kul::hash::set::String m_reloaded;
        kul::hash::map::S2T<Project*> m_projects;

    public:
        static Projects& INSTANCE(){
            static Projects p;
            return p;
        }
        const Project* getOrCreate(const kul::Dir& d){
            if(!d) KEXCEPT(ProjectException, "Directory does not exist:\n" + d.path());
            kul::File f("mkn.yaml", d);
            if(!f.is()) KEXCEPT(ProjectException, "project file does not exist:\n" + f.full());
            if(!m_projects.count(d.real())){
                auto project = std::make_unique<Project>(d);
                try{
                    kul::yaml::Item::VALIDATE(project->root(), project->validator().children());
                }catch(const kul::yaml::Exception& e){
                    KEXCEPT(ProjectException, "YAML error encountered in file: " + f.real());
                }
                auto pp = project.get();
                m_pps.push_back(std::move(project));
                m_projects.insert(f.real(), pp);
            }
            return m_projects[f.real()];
        }
        void reload(const Project& proj){
            if(!m_reloaded.count(proj.file())){
                m_projects[proj.file()]->reload();
                m_reloaded.insert(proj.file());
            }
        }
};

class NewProject{
    private:
        kul::File f;
        void write();
        const kul::File& file() const { return f; }
    public:
        NewProject() KTHROW(ProjectException) : f("mkn.yaml", kul::env::CWD()){
            if(!f.is())
                write();
            else
                KEXCEPT(ProjectException, "mkn.yaml already exists");
        }
};

}
#endif /* _MAIKEN_PROJECT_HPP_ */
