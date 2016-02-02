SOURCES +=	source/main.cpp \
			source/BlueTecFileManager.cpp \
			source/FileMod.cpp \
			source/SwapFile.cpp \
			source/File.cpp \
			source/NotifyDirectory.cpp \
    source/BTCloudApp.cpp

HEADERS +=	\
			include/IFile.h \
			include/Defines.hpp \
			include/BlueTecFileManager.h \
			include/FileMod.h \
			include/SwapFile.h \
			include/File.h \
			include/NotifyDirectory.h \
    include/BTCloudApp.hpp

OTHER_FILES += resources/app.config \

include("app.pri")
include("compiler.pri")

