SOURCES +=	source/main.cpp \
			source/BlueTecFileManager.cpp \
			source/FileMod.cpp \
			source/SwapFile.cpp \
			source/File.cpp \
			source/Protocolo.cpp \
			source/BTCloudApp.cpp \
			source/entities/pacote_posicao.pb.cc

HEADERS +=	\
			include/IFile.h \
			include/Defines.hpp \
			include/BlueTecFileManager.h \
			include/FileMod.h \
			include/SwapFile.h \
			include/File.h \
			include/Protocolo.h \
			include/BTCloudApp.hpp \
			include/entities/pacote_posicao.pb.h

OTHER_FILES += resources/app.config \

include("app.pri")
include("compiler.pri")
