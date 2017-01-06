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
// Translation gratefully received from https://github.com/eantcal
#ifndef _MAIKEN_DEFS_LANG_HPP_
#define _MAIKEN_DEFS_LANG_HPP_

#define MKN_DEFS_CMD        "Comandi :"
#define MKN_DEFS_BUILD      "   build    | Compila e collega (link)"
#define MKN_DEFS_CLEAN      "   clean    | Cancella i file da ./bin/$profile"
#define MKN_DEFS_COMP       "   compile  | Compila i sorgenti in ./bin/$profile"
#define MKN_DEFS_DBG        "   dbg      | Esegue col debugger un programma associato a un profilo di progetto"
#define MKN_DEFS_INIT       "   init     | Crea un file mkn.xml minimale in ./"
#define MKN_DEFS_LINK       "   link     | Collega (link) i file oggetto all'eseguibile/libreria (exe/lib)"
#define MKN_DEFS_PACK       "   pack     | Copia i file binari e le librerie in bin/$profile/pack"
#define MKN_DEFS_PROFS      "   profiles | Mostra i profili contenuti in ./mkn.xml"
#define MKN_DEFS_RUN        "   run      | Esegue un programma associato a un profilo di progetto collegando le librerie dinamiche automaticamente"
#define MKN_DEFS_INC        "   inc      | Stampa le cartelle di inclusione (include) sullo standard output"
#define MKN_DEFS_SRC        "   src      | Stampa i file sorgenti trovati sullo standard output [consente l'opzione -d]."
#define MKN_DEFS_TRIM       "   trim     | Rimuove gli spazi finali dai file sorgenti (inc/src) sotto la cartella di progetto"

#define MKN_DEFS_ARG        "Argomenti:"
#define MKN_DEFS_ARGS       "   -a/--args $a          | Aggiunge l'argomento $arg in fase di compilazione, lo passa al processo in fase di esecuzione"
#define MKN_DEFS_DEPS       "   -d/--dependencies $d  | Numero di figli da includere o lista dei progetti/profili separati da virgola, in assenza si assume tutti quelli esistenti"
#define MKN_DEFS_DIRC       "   -C/--directory $d     | Esegue nella directory $d invece che in quella corrente"
#define MKN_DEFS_EVSA       "   -E/--env $a           | Sovrascrive le variabili di ambiente specificate come lista di chiave=valore separate da virgola: \"k1=v1,k2=v2\""
#define MKN_DEFS_HELP       "   -h/--help             | Mostra l'help sulla console"
#define MKN_DEFS_JARG       "   -j/--jargs            | Argomenti specifici per tipo di file in formato JSON, ad es. '{\"c\": \"-DC_ARG1\", \"cpp\": \"-DCXX_ARG1\"}'"
#define MKN_DEFS_LINKER     "   -l/--linker $t        | Aggiunge $t per il collegamento (link) del profilo del progetto radice (root project)"
#define MKN_DEFS_ALINKR     "   -L/--all-linker $t    | Aggiunge $t per il collegamento (link) di tutti i progetti che lo prevedono"
#define MKN_DEFS_PROF       "   -p/--profile $p       | Attiva il profilo $p"
#define MKN_DEFS_PROP       "   -P/--property $p      | Sovrascrive le proprieta` specificate come lista di chiave=valore separate da virgola: \"k1=v1,k2=v2\""
#define MKN_DEFS_DRYR       "   -R/--dry-run          | Visualizza i comandi su console senza realmente compilare, eseguire, ect."
#define MKN_DEFS_STAT       "   -s/--scm-status       | Mostra lo stato dell'SCM della cartella di un progetto, consente l'opzione -d"
#define MKN_DEFS_THREDS     "   -t/--threads $n       | Utilizza $n thread in compilazione dove $n > 0, se n non e` specificato ne viene calcolato uno ottimale."
#define MKN_DEFS_UPDATE     "   -u/--scm-update       | Verifica se ci sono aggiornamenti ai progetti e chiede se applicarli, consente l'opzione -d"
#define MKN_DEFS_FUPDATE    "   -U/--scm-force-update | Forza l'aggiornamento di un progetto da SCM, consente l'opzione -d"
#define MKN_DEFS_VERSON     "   -v/--version          | Mostra l'attuale versione di maiken, ignora eventuali altri argomenti specificati"
#define MKN_DEFS_SETTNGS    "   -x/--settings $f      | Imposta il file settings.xml in uso al file $f, se la cartella di $f e` mancante viene cercato come $(HOME)/maiken/$f"
#define MKN_DEFS_STATIC     "   -K/--static           | Effettua il collegamento (link) statico dei progetti nel caso 'mode' non sia eplicitato"
#define MKN_DEFS_SHARED     "   -S/--shared           | Effettua il collegamento (link) dinamico dei progetti nel caso 'mode' non sia eplicitato"

#define MKN_DEFS_EXMPL      "Examples:"
#define MKN_DEFS_EXMPL1     "   mkn build -dtKUa             | Forza l'aggiornamento / Compila col numero ottimale di thread / Collega (link) staticamente - ogni cosa"
#define MKN_DEFS_EXMPL2     "   mkn clean build -dtu         | Aggiorna tutto se necessario, rimuove ogni file intermedio, costruisce tutto con un numero ottimale di thread"
#define MKN_DEFS_EXMPL3     "   mkn clean build -d 1 -t 2 -u | Aggiorna/Pulisce/Costruisce il progetto e le dipendenze di primo livello usando due thread"
#define MKN_DEFS_EXMPL4     "   mkn -ds                      | Mostra lo stato di '${scm}' per tutto"

#define MKN_PARENT              "Genitore"
#define MKN_PROFILE             "Profilo"
#define MKN_PROJECT             "Progetto"
#define MKN_PROJECT_NOT_FOUND   "Progetto non trovato, tentativo di risoluzione automatica: "

#endif /* _MAIKEN_DEFS_LANG_HPP_ */
