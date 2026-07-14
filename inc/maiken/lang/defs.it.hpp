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
*/  // Translation gratefully received from https://github.com/eantcal
#ifndef _MAIKEN_DEFS_LANG_HPP_
#define _MAIKEN_DEFS_LANG_HPP_

#include <string>

#include "maiken/lang/err/it.hpp"

namespace maiken::lang::defs {

static std::string const MKN_DEFS_CMD = "Comandi :";
static std::string const MKN_DEFS_BUILD = "   build     | Compila e collega (link)";
static std::string const MKN_DEFS_CLEAN = "   clean     | Cancella i file da ./bin/$profile";
static std::string const MKN_DEFS_COMP = "   compile   | Compila i sorgenti in ./bin/$profile";
static std::string const MKN_DEFS_DBG =
    "   dbg       | Esegue col debugger un programma associato a un profilo di "
    "progetto";
static std::string const MKN_DEFS_INIT = "   init      | Crea un file mkn.yaml minimale in ./";
static std::string const MKN_DEFS_LINK =
    "   link      | Collega (link) i file oggetto all'eseguibile/libreria "
    "(exe/lib)";
static std::string const MKN_DEFS_PACK =
    "   pack      | Copia i file binari e le librerie in bin/$profile/pack";
static std::string const MKN_DEFS_PROFS = "   profiles  | Mostra i profili contenuti in ./mkn.yaml";
static std::string const MKN_DEFS_RUN =
    "   run       | Esegue un programma associato a un profilo di progetto "
    "collegando le librerie dinamiche automaticamente";
static std::string const MKN_DEFS_INC =
    "   inc       | Stampa le cartelle di inclusione (include) sullo standard "
    "output";
static std::string const MKN_DEFS_SRC =
    "   src       | Stampa i file sorgenti trovati sullo standard output "
    "[consente l'opzione -d].";
static std::string const MKN_DEFS_TREE = "   tree      | Display dependency tree";

static std::string const MKN_DEFS_ARG = "Argomenti:";
static std::string const MKN_DEFS_ARGS =
    "   -a/--args $a           | Aggiunge l'argomento $arg in fase di "
    "compilazione, lo passa al processo in fase di esecuzione";
static std::string const MKN_DEFS_ADD =
    "   -A --add $csv          | CSV list of additional sources to "
    "compile/link, disable recursive finding with \"<dir>\\,0\"";
static std::string const MKN_DEFS_BINC =
    "   -b/--binc $s           | Aggiunge cartelle di inclusione in coda al "
    "comando di compilazione, separate secondo il PATH di sistema standard "
    "win=; altri=:";
static std::string const MKN_DEFS_BPATH =
    "   -B/--bpath $s          | Aggiunge una cartella di ricerca librerie in "
    "coda al comando di collegamento (link), separate secondo il PATH di "
    "sistema standard win=; altri=:";
static std::string const MKN_DEFS_DEPS =
    "   -d/--dependencies $d   | Numero di figli da includere o lista dei "
    "progetti/profili separati da virgola, in assenza si assume tutti quelli "
    "esistenti";
static std::string const MKN_DEFS_DIRC =
    "   -C/--directory $d      | Esegue nella directory $d invece che in "
    "quella corrente";

static std::string const MKN_DEFS_DUMP =
    "   -D/--dump              | Write command logs to ./.mkn/logs/$PROFILE";

static std::string const MKN_DEFS_EVSA =
    "   -E/--env $a            | Sovrascrive le variabili di ambiente "
    "specificate come lista di chiave=valore separate da virgola: "
    "\"k1=v1,k2=v2\"";
static std::string const MKN_DEFS_GET =
    "   -G/--get $k            | Returns string value for property K in either "
    "local yaml or settings.yaml, failure is no string 0 exit code";
static std::string const MKN_DEFS_HELP = "   -h/--help              | Mostra l'help sulla console";
static std::string const MKN_DEFS_FINC =
    "   -f/--finc $s           | Aggiunge cartelle di inclusione in testa al "
    "comando di compilazione, separate secondo il PATH di sistema standard "
    "win=; altri=:";
static std::string const MKN_DEFS_FPATH =
    "   -F/--fpath $s          | Aggiunge una cartella di ricerca librerie in "
    "testa al comando di collegamento (link), separate secondo il PATH di "
    "sistema standard win=; altri=:";
static std::string const MKN_DEFS_DEBUG =
    "   -g/--debug [0-9]       | Aggiunge i flag di debug per compilatore/linker";
static std::string const MKN_DEFS_JARG =
    "   -j/--jargs             | Argomenti specifici per tipo di file in "
    "formato JSON, ad es. '{\"c\": \"-DC_ARG1\", \"cpp\": \"-DCXX_ARG1\"}'";
static std::string const MKN_DEFS_LINKER =
    "   -l/--linker $t         | Aggiunge $t per il collegamento (link) del "
    "profilo del progetto radice (root project)";
static std::string const MKN_DEFS_ALINKR =
    "   -L/--all-linker $t     | Aggiunge $t per il collegamento (link) di "
    "tutti i progetti che lo prevedono";
static std::string const MKN_DEFS_MOD =
    "   -m/--mod [$csv]        | Activate modules for selected phase (init/compile/link/pack) ";
static std::string const MKN_DEFS_MAIN =
    "   -M/--main $m           | Sets main linker file to $m, supercedes /main "
    "tag";
static std::string const MKN_DEFS_NODES =
    "   -n/--nodes [$n]        | Attiva la compilazione distribuita con $n "
    "nodi dal file settings.yaml scelto (-x)";
static std::string const MKN_DEFS_OUT =
    "   -o/--out $o            | Sets output binary/library to name $o, "
    "supercedes /out tag";
static std::string const MKN_DEFS_OPTIM =
    "   -O/--optimize [0-9]    | Aggiunge i flag di ottimizzazione per compilatore/linker";
static std::string const MKN_DEFS_PROF = "   -p/--profile $p        | Attiva il profilo $p";
static std::string const MKN_DEFS_PROP =
    "   -P/--property $p       | Sovrascrive le proprieta` specificate come "
    "lista di chiave=valore separate da virgola: \"k1=v1,k2=v2\"";
static std::string const MKN_DEFS_QUIET =
    "   -q/--quiet             | Nasconde il messaggio \"il progetto sembra "
    "obsoleto\" quando si compila senza build/compile";
static std::string const MKN_DEFS_RUN_ARGS =
    "   -r/--run-args $a       | Passes $a to running binary as arguments, "
    "suppercedes -a";
static std::string const MKN_DEFS_DRYR =
    "   -R/--dry-run           | Visualizza i comandi su console senza "
    "realmente compilare, eseguire, ect.";
static std::string const MKN_DEFS_STAT =
    "   -s/--scm-status        | Mostra lo stato dell'SCM della cartella di un "
    "progetto, consente l'opzione -d";
static std::string const MKN_DEFS_THREDS =
    "   -t/--threads $n        | Utilizza $n thread in compilazione dove $n > "
    "0, se n non e` specificato ne viene calcolato uno ottimale.";
static std::string const MKN_DEFS_UPDATE =
    "   -u/--scm-update        | Verifica se ci sono aggiornamenti ai progetti "
    "e chiede se applicarli, consente l'opzione -d";
static std::string const MKN_DEFS_FUPDATE =
    "   -U/--scm-force-update  | Forza l'aggiornamento di un progetto da SCM, "
    "consente l'opzione -d";
static std::string const MKN_DEFS_VERSON =
    "   -v/--version           | Mostra l'attuale versione di maiken, ignora "
    "eventuali altri argomenti specificati";
static std::string const MKN_DEFS_SETTNGS =
    "   -x/--settings $f       | Imposta il file settings.yaml in uso al file "
    "$f, se la cartella di $f e` mancante viene cercato come $(HOME)/maiken/$f";
static std::string const MKN_DEFS_STATIC =
    "   -K/--static            | Effettua il collegamento (link) statico dei "
    "progetti nel caso 'mode' non sia eplicitato";
static std::string const MKN_DEFS_SHARED =
    "   -S/--shared            | Effettua il collegamento (link) dinamico dei "
    "progetti nel caso 'mode' non sia eplicitato";
static std::string const MKN_DEFS_WARN =
    "   -W/--warn [0-9]        | Aggiunge i flag del compilatore per gli avvisi (warning)";
static std::string const MKN_DEFS_WITH =
    "   -w/--with $CSV         | Aggiunge un profilo o una dipendenza dalla riga di comando";
static std::string const MKN_DEFS_WITHOUT =
    "   -T/--without $CSV      | Rimuove una dipendenza dalla build";

static std::string const MKN_DEFS_EXMPL = "Examples:";
static std::string const MKN_DEFS_EXMPL1 =
    "   mkn build -dtKUa             | Forza l'aggiornamento / Compila col "
    "numero ottimale di thread / Collega (link) staticamente - ogni cosa";
static std::string const MKN_DEFS_EXMPL2 =
    "   mkn clean build -dtu         | Aggiorna tutto se necessario, rimuove "
    "ogni file intermedio, costruisce tutto con un numero ottimale di thread";
static std::string const MKN_DEFS_EXMPL3 =
    "   mkn clean build -d 1 -t 2 -u | Aggiorna/Pulisce/Costruisce il progetto "
    "e le dipendenze di primo livello usando due thread";
static std::string const MKN_DEFS_EXMPL4 =
    "   mkn -ds                      | Mostra lo stato di '${scm}' per tutto";

static std::string const MKN_PARENT = "Genitore";
static std::string const MKN_PROFILE = "Profilo";
static std::string const MKN_PROJECT = "Progetto";
static std::string const MKN_PROJECT_NOT_FOUND =
    "Progetto non trovato, tentativo di risoluzione automatica: ";

}  // namespace maiken::lang::defs

#endif /* _MAIKEN_DEFS_LANG_HPP_ */
