/**
Copyright (c) 2026, Philip Deegan.
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
#include "maiken.hpp"
#include "maiken/defs.hpp"

maiken::CompileDAO maiken::CompilationUnit::dao() const {
  return CompileDAO{app.context(), compiler, in, out, args, mode, dryRun};
}

maiken::ContextState maiken::ApplicationContext::state() const {
  ContextState s;
  s.projectDir = app.project().dir().real();
  s.includes = app.includes();
  for (auto* dep : app.revendencies()) s.dependents.push_back(&dep->context());
  s.buildMode = app.mode();
  for (auto const& byType : app.sourceMap())
    for (auto const& byDir : byType.second)
      for (auto const& src : byDir.second) s.sourceFiles.push_back(src.in);
  s.compileEnv = CompileEnv{app.envVars(), app.compilationInfo()};
  return s;
}

void maiken::ApplicationContext::add(AbstractCompilerInput const& input) {
  if (auto const* i = dynamic_cast<IncludeInput const*>(&input))
    app.addInclude(i->path, i->is_public);
  else if (auto const* i = dynamic_cast<LibPathInput const*>(&input))
    app.addLibpath(i->path);
  else if (auto const* i = dynamic_cast<LibInput const*>(&input))
    app.addLib(i->name);
  else if (auto const* i = dynamic_cast<LinkPrefixInput const*>(&input))
    app.prependLinkString(i->value);
  else if (auto const* i = dynamic_cast<BuildModeInput const*>(&input))
    app.mode(i->mode);
  else if (auto const* i = dynamic_cast<CompilationInfoInput const*>(&input))
    app.compilationInfo(i->info);
  else
    KEXCEPT(Exception, "Unrecognised compiler input");
}

// Generates the real CompileDAO for every source file (same resolution logic
// real compilation uses), translates it to the stable CompileCommand shape,
// and hands each one to hook without executing anything - e.g. clang-tidy
// reading actual compile flags.
void maiken::ApplicationContext::per_compiler_command(CompileHook hook) {
  mkn::kul::hash::set::String objects;
  std::vector<mkn::kul::File> cacheFiles;
  auto const src_objs = all_sources_from(app, app.sourceMap(), objects, cacheFiles);
  auto const incs = resolve_includes(app);

  for (auto const& so : src_objs) {
    auto const unit = build_compilation_unit(app, incs, so);
    auto const d = unit.dao();
    hook(CompileCommand{d.compiler, d.in, d.out, d.args, /*dryRun=*/true});
  }
}

std::string maiken::ApplicationContext::compileCommandFor(std::string const& in) const {
  auto const incs = resolve_includes(app);
  return build_compilation_unit(app, incs, {Source(in), "obj"}).compileString();
}
