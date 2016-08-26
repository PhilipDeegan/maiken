

@ECHO off
SETLOCAL enabledelayedexpansion

SET KUL_VER=master
SET YAML_VER=master
SET HASH_VER=master
SET PWD=%CD%

SET WIN_KIT=
SET VIS_STU=
SET VSCC=
SET VSCAR=
SET CXXFLAGS=/EHsc /nologo
SET INCLUDES=
SET OBJECTS=
SET LIBS=yaml.lib
SET LIBPATHS=

ECHO "Making Maiken for Windows"

IF NOT EXIST ext\kul\%KUL_VER% (
	git clone https://github.com/mkn/mkn.kul.git --branch %KUL_VER% ext/kul/%KUL_VER%
)
IF NOT EXIST ext\yaml\%YAML_VER% (
	git clone https://github.com/mkn/parse.yaml.git --branch %YAML_VER% ext/yaml/%YAML_VER%
)
IF NOT EXIST ext\sparsehash\%HASH_VER% (
	git clone https://github.com/mkn/google.sparsehash.git --branch %HASH_VER% ext/sparsehash/%HASH_VER%
)

SET /P WIN_KIT="Enter Windows Kit 10 root dir (one above 'include'): "
SET /P VIS_STU="Enter Visual Studio 14/15 root dir (one above 'VC'): "

IF NOT EXIST %VIS_STU%\VC\bin\cl.exe (
	ECHO %VIS_STU%\VC\bin\cl.exe does not exist
	GOTO end
)
SET VSCC=%VIS_STU%\VC\bin\cl
SET VSCAR=%VIS_STU%\VC\bin\lib

SET INCLUDES=%INCLUDES% /I%PWD%\ext\kul\%KUL_VER%\inc
SET INCLUDES=%INCLUDES% /I%PWD%\ext\kul\%KUL_VER%\os\win\inc
SET INCLUDES=%INCLUDES% /I%PWD%\ext\sparsehash\%HASH_VER%
SET INCLUDES=%INCLUDES% /I%PWD%\ext\yaml\%YAML_VER%\include
SET INCLUDES=%INCLUDES% /I%WIN_KIT%\Include\10.0.10240.0\um
SET INCLUDES=%INCLUDES% /I%WIN_KIT%\Include\10.0.10240.0\ucrt
SET INCLUDES=%INCLUDES% /I%WIN_KIT%\Include\10.0.10240.0\shared
SET INCLUDES=%INCLUDES% /I%VIS_STU%\VC\include

SET LIBPATHS=%LIBPATHS% /LIBPATH:%PWD%\ext\kul\%KUL_VER%\bin
SET LIBPATHS=%LIBPATHS% /LIBPATH:%PWD%\ext\yaml\%YAML_VER%\bin
SET LIBPATHS=%LIBPATHS% /LIBPATH:%WIN_KIT%\Lib\10.0.10240.0\um\x86
SET LIBPATHS=%LIBPATHS% /LIBPATH:%WIN_KIT%\Lib\10.0.10240.0\ucrt\x86
SET LIBPATHS=%LIBPATHS% /LIBPATH:%VIS_STU%\VC\lib

SET OBJECTS=

RD /S/Q %PWD%\ext\yaml\%YAML_VER%\bin
MKDIR %PWD%\ext\yaml\%YAML_VER%\bin
FOR /f %%f in ('dir /b %PWD%\ext\yaml\%YAML_VER%\src\*.cpp') do %VSCC% %CXXFLAGS% %INCLUDES% /c /Fo"%PWD%\ext\yaml\%YAML_VER%\bin\%%f.o" "%PWD%\ext\yaml\%YAML_VER%\src\%%f" 
FOR /f %%f in ('dir /b %PWD%\ext\yaml\%YAML_VER%\src\contrib\*cpp') do %VSCC% %CXXFLAGS% %INCLUDES% /c /Fo"%PWD%\ext\yaml\%YAML_VER%\bin\%%f.o" "%PWD%\ext\yaml\%YAML_VER%\src\contrib\%%f" 
FOR /f %%f in ('dir /b %PWD%\ext\yaml\%YAML_VER%\bin') do SET OBJECTS=!OBJECTS! %PWD%\ext\yaml\%YAML_VER%\bin\%%f
%VSCAR% /OUT:"%PWD%\ext\yaml\%YAML_VER%\bin\yaml.lib" /NOLOGO /LTCG %OBJECTS%

SET OBJECTS=

RD /S/Q bin
MKDIR %PWD%\bin
FOR /f %%f in ('dir /A:-D /b %PWD%\src') do %VSCC% %CXXFLAGS% /I%PWD%\inc %INCLUDES% /c /Fo"%PWD%\bin\%%f.o" "%PWD%\src\%%f"
FOR /f %%f in ('dir /A:-D /b %PWD%\src\maiken') do %VSCC% %CXXFLAGS% /I%PWD%\inc %INCLUDES% /c /Fo"%PWD%\bin\%%f.o" "%PWD%\src\maiken\%%f" 
FOR /f %%f in ('dir /A:-D /b %PWD%\src\maiken\scm') do %VSCC% %CXXFLAGS% /I%PWD%\inc %INCLUDES% /c /Fo"%PWD%\bin\%%f.o" "%PWD%\src\maiken\scm\%%f" 
FOR /f %%f in ('dir /A:-D /b %PWD%\src\maiken\threading') do %VSCC% %CXXFLAGS% /I%PWD%\inc %INCLUDES% /c /Fo"%PWD%\bin\%%f.o" "%PWD%\src\maiken\threading\%%f" 
%VSCC% %CXXFLAGS% /I%PWD%\inc %INCLUDES% /c /Fo"%PWD%\bin\cpp.o" "%PWD%\cpp.cpp" 

FOR /f %%f in ('dir /b %PWD%\bin') do SET OBJECTS=!OBJECTS! %PWD%\bin\%%f
%VIS_STU%\VC\bin\link %LIBPATHS% /OUT:%PWD%\mkn.exe /NOLOGO %OBJECTS% %LIBS%

RD /S/Q ext
RD /S/Q bin

GOTO end
:end
ECHO END
