
#include "maiken/compiler/cpp.hpp"

#if !defined(_WIN32)

void maiken::cpp::CL_DEF::make_def(const std::vector<std::string> &files){}
void maiken::cpp::CL_DEF::dump(const kul::File &out){}
void maiken::cpp::CL_DEF::dump_object_exports(const std::vector<std::string> &files, const kul::File &out){}

#else

void maiken::cpp::CL_DEF::dump(const kul::File &out){

  std::string  Fname(out.escm()), mode("w+");
  std::wstring Wname(Fname.begin(), Fname.end()),
               wode(mode.begin(), mode.end());

  FILE* file = _wfopen(Wname.c_str(), wode.c_str());
  fprintf(file, "EXPORTS \n");
  for ( const auto & ds : this->DataSymbols)
    fprintf(file, "\t%s \t DATA\n", ds.c_str());

  for ( const auto & s : this->Symbols)
    fprintf(file, "\t%s\n", s.c_str());

  fclose(file);
}
void maiken::cpp::CL_DEF::dump_object_exports(const std::vector<std::string> &files, const kul::File &out){
  make_def(files);
  dump(out);
}


typedef struct cmANON_OBJECT_HEADER_BIGOBJ
{
  /* same as ANON_OBJECT_HEADER_V2 */
  WORD Sig1;    // Must be IMAGE_FILE_MACHINE_UNKNOWN
  WORD Sig2;    // Must be 0xffff
  WORD Version; // >= 2 (implies the Flags field is present)
  WORD Machine; // Actual machine - IMAGE_FILE_MACHINE_xxx
  DWORD TimeDateStamp;
  CLSID ClassID;        // {D1BAA1C7-BAEE-4ba9-AF20-FAF66AA4DCB8}
  DWORD SizeOfData;     // Size of data that follows the header
  DWORD Flags;          // 0x1 -> contains metadata
  DWORD MetaDataSize;   // Size of CLR metadata
  DWORD MetaDataOffset; // Offset of CLR metadata

  /* bigobj specifics */
  DWORD NumberOfSections; // extended from WORD
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
} cmANON_OBJECT_HEADER_BIGOBJ;

typedef struct _cmIMAGE_SYMBOL_EX
{
  union
  {
    BYTE ShortName[8];
    struct
    {
      DWORD Short; // if 0, use LongName
      DWORD Long;  // offset into string table
    } Name;
    DWORD LongName[2]; // PBYTE  [2]
  } N;
  DWORD Value;
  LONG SectionNumber;
  WORD Type;
  BYTE StorageClass;
  BYTE NumberOfAuxSymbols;
} cmIMAGE_SYMBOL_EX;
typedef cmIMAGE_SYMBOL_EX UNALIGNED* cmPIMAGE_SYMBOL_EX;

PIMAGE_SECTION_HEADER GetSectionHeaderOffset(
  PIMAGE_FILE_HEADER pImageFileHeader)
{
  return (PIMAGE_SECTION_HEADER)((DWORD_PTR)pImageFileHeader +
                                 IMAGE_SIZEOF_FILE_HEADER +
                                 pImageFileHeader->SizeOfOptionalHeader);
}

PIMAGE_SECTION_HEADER GetSectionHeaderOffset(
  cmANON_OBJECT_HEADER_BIGOBJ* pImageFileHeader)
{
  return (PIMAGE_SECTION_HEADER)((DWORD_PTR)pImageFileHeader +
                                 sizeof(cmANON_OBJECT_HEADER_BIGOBJ));
}

template <
  // cmANON_OBJECT_HEADER_BIGOBJ or IMAGE_FILE_HEADER
  class ObjectHeaderType,
  // cmPIMAGE_SYMBOL_EX or PIMAGE_SYMBOL
  class SymbolTableType>
class DumpSymbols
{
public:
  /*
   *----------------------------------------------------------------------
   * Constructor --
   *
   *     Initialize variables from pointer to object header.
   *
   *----------------------------------------------------------------------
   */

  DumpSymbols(ObjectHeaderType* ih, std::set<std::string>& symbols,
              std::set<std::string>& dataSymbols, bool isI386)
    : Symbols(symbols)
    , DataSymbols(dataSymbols)
  {
    this->ObjectImageHeader = ih;
    this->SymbolTable =
      (SymbolTableType*)((DWORD_PTR)this->ObjectImageHeader +
                         this->ObjectImageHeader->PointerToSymbolTable);
    this->SectionHeaders = GetSectionHeaderOffset(this->ObjectImageHeader);
    this->SymbolCount = this->ObjectImageHeader->NumberOfSymbols;
    this->IsI386 = isI386;
  }

  /*
   *----------------------------------------------------------------------
   * DumpObjFile --
   *
   *      Dump an object file's exported symbols.
   *----------------------------------------------------------------------
   */
  void DumpObjFile() { this->DumpExternalsObjects(); }

  /*
   *----------------------------------------------------------------------
   * DumpExternalsObjects --
   *
   *      Dumps a COFF symbol table from an OBJ.
   *----------------------------------------------------------------------
   */
  void DumpExternalsObjects()
  {
    unsigned i;
    PSTR stringTable;
    std::string symbol;
    DWORD SectChar;
    /*
     * The string table apparently starts right after the symbol table
     */
    stringTable = (PSTR) & this->SymbolTable[this->SymbolCount];
    SymbolTableType* pSymbolTable = this->SymbolTable;
    for (i = 0; i < this->SymbolCount; i++) {
      if (pSymbolTable->SectionNumber > 0 &&
          (pSymbolTable->Type == 0x20 || pSymbolTable->Type == 0x0)) {
        if (pSymbolTable->StorageClass == IMAGE_SYM_CLASS_EXTERNAL) {
          /*
           *    The name of the Function entry points
           */
          if (pSymbolTable->N.Name.Short != 0) {
            symbol.clear();
            symbol.insert(0, (const char*)pSymbolTable->N.ShortName, 8);
          } else {
            symbol = stringTable + pSymbolTable->N.Name.Long;
          }

          // clear out any leading spaces
          while (isspace(symbol[0]))
            symbol.erase(0, 1);
          // if it starts with _ and has an @ then it is a __cdecl
          // so remove the @ stuff for the export
          if (symbol[0] == '_') {
            std::string::size_type posAt = symbol.find('@');
            if (posAt != std::string::npos) {
              symbol.erase(posAt);
            }
          }
          // For i386 builds we need to remove _
          if (this->IsI386 && symbol[0] == '_') {
            symbol.erase(0, 1);
          }

          // Check whether it is "Scalar deleting destructor" and "Vector
          // deleting destructor"
          // if scalarPrefix and vectorPrefix are not found then print
          // the symbol
          KLOG(INF);
          const char* scalarPrefix = "??_G";
          const char* vectorPrefix = "??_E";
          // The original code had a check for
          //     symbol.find("real@") == std::string::npos)
          // but this disallows member functions with the name "real".
          if (symbol.compare(0, 4, scalarPrefix) &&
              symbol.compare(0, 4, vectorPrefix)) {
            if(!this->SectionHeaders || !pSymbolTable || !&this->SectionHeaders[pSymbolTable->SectionNumber - 1])
              KERR << "Most likely compiled with -GL - this is not allowed";
            KLOG(INF);
            KLOG(INF) << &this->SectionHeaders[pSymbolTable->SectionNumber - 1]
                         .Characteristics;

            SectChar = this->SectionHeaders[pSymbolTable->SectionNumber - 1]
                         .Characteristics;
            // skip symbols containing a dot
            if (symbol.find('.') == std::string::npos) {
              if (!pSymbolTable->Type && (SectChar & IMAGE_SCN_MEM_WRITE)) {
                // Read only (i.e. constants) must be excluded
                this->DataSymbols.insert(symbol);
              } else {
                if (pSymbolTable->Type || !(SectChar & IMAGE_SCN_MEM_READ) ||
                    (SectChar & IMAGE_SCN_MEM_EXECUTE)) {
                  this->Symbols.insert(symbol);
                }
              }
            }
          }
        }
      }

      /*
       * Take into account any aux symbols
       */
      i += pSymbolTable->NumberOfAuxSymbols;
      pSymbolTable += pSymbolTable->NumberOfAuxSymbols;
      pSymbolTable++;
    }
  }

private:
  std::set<std::string>& Symbols;
  std::set<std::string>& DataSymbols;
  DWORD_PTR SymbolCount;
  PIMAGE_SECTION_HEADER SectionHeaders;
  ObjectHeaderType* ObjectImageHeader;
  SymbolTableType* SymbolTable;
  bool IsI386;
};


bool DumpFile(const char* filename, std::set<std::string>& symbols,
              std::set<std::string>& dataSymbols)
{
  HANDLE hFile;
  HANDLE hFileMapping;
  LPVOID lpFileBase;
  std::string  Fname(filename);
  std::wstring Wname(Fname.begin(), Fname.end());
  hFile = CreateFileW(Wname.c_str(), GENERIC_READ,
                      FILE_SHARE_READ, NULL, OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL, 0);

  if (hFile == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Couldn't open file '%s' with CreateFile()\n", filename);
    return false;
  }

  hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
  if (hFileMapping == 0) {
    CloseHandle(hFile);
    fprintf(stderr, "Couldn't open file mapping with CreateFileMapping()\n");
    return false;
  }

  lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
  if (lpFileBase == 0) {
    CloseHandle(hFileMapping);
    CloseHandle(hFile);
    fprintf(stderr, "Couldn't map view of file with MapViewOfFile()\n");
    return false;
  }

  const PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
  if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
    fprintf(stderr, "File is an executable.  I don't dump those.\n");
    return false;
  } else {
    const PIMAGE_FILE_HEADER imageHeader = (PIMAGE_FILE_HEADER)lpFileBase;
    /* Does it look like a COFF OBJ file??? */
    if (((imageHeader->Machine == IMAGE_FILE_MACHINE_I386) ||
         (imageHeader->Machine == IMAGE_FILE_MACHINE_AMD64) ||
         (imageHeader->Machine == IMAGE_FILE_MACHINE_ARM) ||
         (imageHeader->Machine == IMAGE_FILE_MACHINE_ARMNT) ||
         (imageHeader->Machine == IMAGE_FILE_MACHINE_ARM64)) &&
        (imageHeader->Characteristics == 0)) {
      /*
       * The tests above are checking for IMAGE_FILE_HEADER.Machine
       * if it contains supported machine formats (currently ARM and x86)
       * and IMAGE_FILE_HEADER.Characteristics == 0 indicating that
       * this is not linked COFF OBJ file;
       */
      DumpSymbols<IMAGE_FILE_HEADER, IMAGE_SYMBOL> symbolDumper(
        (PIMAGE_FILE_HEADER)lpFileBase, symbols, dataSymbols,
        (imageHeader->Machine == IMAGE_FILE_MACHINE_I386));
      symbolDumper.DumpObjFile();
    } else {
      // check for /bigobj format
      cmANON_OBJECT_HEADER_BIGOBJ* h =
        (cmANON_OBJECT_HEADER_BIGOBJ*)lpFileBase;
      if (h->Sig1 == 0x0 && h->Sig2 == 0xffff) {
        DumpSymbols<cmANON_OBJECT_HEADER_BIGOBJ, cmIMAGE_SYMBOL_EX>
          symbolDumper((cmANON_OBJECT_HEADER_BIGOBJ*)lpFileBase, symbols,
                       dataSymbols, (h->Machine == IMAGE_FILE_MACHINE_I386));
        symbolDumper.DumpObjFile();
      } else {
        printf("unrecognized file format in '%s'\n", filename);
        return false;
      }
    }
  }
  UnmapViewOfFile(lpFileBase);
  CloseHandle(hFileMapping);
  CloseHandle(hFile);
  return true;
}

void maiken::cpp::CL_DEF::make_def(const std::vector<std::string> &files){
  for(auto &f: files) DumpFile(f.c_str(), this->Symbols, this->DataSymbols);
}

#endif