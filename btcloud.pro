SOURCES +=	source/main.cpp \
			source/BlueTecFileManager.cpp \
			source/FileMod.cpp \
			source/SwapFile.cpp \
			source/File.cpp \
			source/Protocolo.cpp \
			source/BTCloudApp.cpp \
			source/entities/pacote_posicao.pb.cc \
			source/entities/bluetec400.pb.cc \
			source/clock.cpp

HEADERS +=	\
			include/IFile.h \
			include/Defines.hpp \
			include/BlueTecFileManager.h \
			include/FileMod.h \
			include/SwapFile.h \
			include/File.h \
			include/Protocolo.h \
			include/BTCloudApp.hpp \
			include/entities/pacote_posicao.pb.h \
			include/entities/bluetec400.pb.h \
			include/clock.h

OTHER_FILES += resources/app.config \

include("app.pri")
include("compiler.pri")
