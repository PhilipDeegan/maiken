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
// Prebuilt dependency binaries: a project named in the settings' bin: map is downloaded from its
//  GitHub release's mkn.manifest.yml into its build directory instead of being built
#include "maiken/app.hpp"
#include "maiken/defs.hpp"
#include "maiken/global.hpp"
#include "maiken/settings.hpp"
#include "mkn/kul/cli.hpp"
#include "mkn/kul/except.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/proc.hpp"
#include "mkn/kul/string.hpp"
#include "yaml-cpp/yaml.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

class Unusable : public std::runtime_error {  // why a prebuilt binary is not used
 public:
  using std::runtime_error::runtime_error;
};

std::string capture(std::string const& cmd, std::vector<std::string> const& args) {
  mkn::kul::Process p(cmd);
  mkn::kul::ProcessCapture pc(p);
  for (auto const& a : args) p << a;
  try {
    p.start();
  } catch (mkn::kul::Exception const&) {
    throw Unusable(cmd + " failed: " + pc.errs());
  }
  auto out = pc.outs();
  while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) out.pop_back();
  return out;
}

void download(std::string const& url, std::filesystem::path const& to) {
  capture("curl", {"-fsSL", "--retry", "3", "-A", "mkn", "-o", mkn::kul::fsys::esc(to), url});
}

// https://github.com/<owner>/<repo> of a git@github.com:<owner>/<repo>(.git) or
//  https://[user@]github.com/<owner>/<repo>(.git) remote
std::string github(std::string url) {
  auto const at = url.find("github.com");
  if (at == std::string::npos) throw Unusable("not a GitHub remote: " + url);
  url = url.substr(at + 11);  // past "github.com/" or "github.com:"
  if (url.size() > 4 && url.substr(url.size() - 4) == ".git") url.resize(url.size() - 4);
  while (!url.empty() && url.back() == '/') url.pop_back();
  return "https://github.com/" + url;
}

std::string sha256(std::filesystem::path const& file) {
  static constexpr std::array<uint32_t, 64> k{
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4,
      0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe,
      0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f,
      0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
      0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
      0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
      0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116,
      0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
      0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7,
      0xc67178f2};
  std::array<uint32_t, 8> h{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  auto const rotr = [](uint32_t x, int n) { return (x >> n) | (x << (32 - n)); };
  auto const block = [&](unsigned char const* p) {
    std::array<uint32_t, 64> w{};
    for (int i = 0; i < 16; ++i)
      w[i] = uint32_t(p[4 * i]) << 24 | uint32_t(p[4 * i + 1]) << 16 | uint32_t(p[4 * i + 2]) << 8 |
             uint32_t(p[4 * i + 3]);
    for (int i = 16; i < 64; ++i) {
      auto const s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
      auto const s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
      w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    auto a = h;
    for (int i = 0; i < 64; ++i) {
      auto const t1 = a[7] + (rotr(a[4], 6) ^ rotr(a[4], 11) ^ rotr(a[4], 25)) +
                      ((a[4] & a[5]) ^ (~a[4] & a[6])) + k[i] + w[i];
      auto const t2 = (rotr(a[0], 2) ^ rotr(a[0], 13) ^ rotr(a[0], 22)) +
                      ((a[0] & a[1]) ^ (a[0] & a[2]) ^ (a[1] & a[2]));
      for (int j = 7; j > 0; --j) a[j] = a[j - 1];
      a[4] += t1;
      a[0] = t1 + t2;
    }
    for (int i = 0; i < 8; ++i) h[i] += a[i];
  };

  std::ifstream in(file, std::ios::binary);
  if (!in) throw Unusable("cannot read " + file.string());
  std::vector<unsigned char> data((std::istreambuf_iterator<char>(in)), {});
  uint64_t const bits = uint64_t(data.size()) * 8;
  data.push_back(0x80);
  while (data.size() % 64 != 56) data.push_back(0);
  for (int i = 7; i >= 0; --i) data.push_back(static_cast<unsigned char>(bits >> (8 * i)));
  for (size_t i = 0; i < data.size(); i += 64) block(&data[i]);

  std::stringstream ss;
  for (auto const v : h) ss << std::hex << std::setw(8) << std::setfill('0') << v;
  return ss.str();
}

}  // namespace

void maiken::Application::fetchPrebuilt(std::string const& tag) {
  std::string const mode = m == compiler::Mode::STAT   ? "static"
                           : m == compiler::Mode::SHAR ? "shared"
                                                       : "";
  if (mode.empty()) throw Unusable("no prebuilt binaries without a static or shared mode");
  auto const key = AppVars::INSTANCE().properkeys().at("mkn.bin.key") + ";mode=" + mode;

  auto const dir = mkn::kul::fsys::escaped_real(project().dir().path());
  if (!capture("git", {"-C", dir, "status", "--porcelain", "--untracked-files=no"}).empty())
    throw Unusable("local changes in " + dir);
  auto const commit = capture("git", {"-C", dir, "rev-parse", "HEAD"});
  auto const url = github(capture("git", {"-C", dir, "remote", "get-url", "origin"})) +
                   "/releases/download/" + tag + "/";

  std::filesystem::path const out = buildDir().path();
  auto const marker = out / ".mkn.prebuilt";
  auto const stamp = tag + " " + commit + " " + key;
  auto const name = project().root()[STR_NAME].Scalar();
  if (std::ifstream m(marker); m) {  // already in place for this tag, commit and key
    std::string s;
    std::getline(m, s);
    if (s == stamp) {
      KOUT(NON) << name << ": prebuilt " << tag << " (" << key << ")";
      return;
    }
  }

  if (AppVars::INSTANCE().dryRun()) {
    KOUT(NON) << name << ": prebuilt " << url << " (" << key << ")";
    return;
  }

  std::filesystem::create_directories(out);
  auto const manifest = out / "mkn.manifest.yml";
  download(url + "mkn.manifest.yml", manifest);
  YAML::Node const mf = YAML::LoadFile(manifest.string());
  if (mf["commit"].as<std::string>() != commit)
    throw Unusable("clone at " + commit.substr(0, 7) + ", " + tag + " built from " +
                   mf["commit"].as<std::string>().substr(0, 7));
  auto const entry = mf["bins"][key];
  if (!entry) throw Unusable(tag + " has no binary for " + key);

  auto const pre = AppVars::INSTANCE().envVar("MKN_LIB_PRE");
  for (auto const& f : entry["files"]) {
    auto const file = f["file"].as<std::string>();
    auto const ext = std::filesystem::path(file).extension().string();
    auto const to = out / (pre + baseLibFilename() + ext);  // where building would put it
    download(url + file, to);
    if (sha256(to) != f["sha256"].as<std::string>()) {
      std::filesystem::remove(to);
      throw Unusable(file + " does not match its sha256");
    }
  }
  std::ofstream(marker) << stamp << "\n";
  KOUT(NON) << name << ": prebuilt " << tag << " (" << key << ")";
}

bool maiken::Application::usePrebuilt() {
  auto const name = project().root()[STR_NAME].Scalar();
  auto const& bins = Settings::INSTANCE().binaries();
  if (!bins.count(name)) return false;
  try {
    fetchPrebuilt(bins.at(name));
    return true;
  } catch (std::exception const& e) {
    std::stringstream ss;
    ss << name << ": prebuilt binary not used - " << e.what();
    if (AppVars::INSTANCE().quiet()) {
      KOUT(NON) << ss.str() << ", building it";
      return false;
    }
    ss << std::endl << "\tWould you like to build it from source (Y/n) - -q builds without asking";
    if (!mkn::kul::String::BOOL(mkn::kul::cli::receive(ss.str())))
      KEXIT(1, name, ": prebuilt binary not used, not building it");
    return false;
  }
}
