SOURCES +=	source/main.cpp \
			source/BlueTecFileManager.cpp \
			source/FileMod.cpp \
			source/SwapFile.cpp \
			source/File.cpp \
			source/Protocol.cpp \
			source/BTCloudApp.cpp \
			source/entities/pacote_posicao.pb.cc \
			source/entities/bluetec400.pb.cc \
			source/clock.cpp \
    source/util/ProtocolUtil.cpp

HEADERS +=	\
			include/IFile.h \
			include/Defines.hpp \
			include/BlueTecFileManager.h \
			include/FileMod.h \
			include/SwapFile.h \
			include/File.h \
			include/Protocol.hpp \
			include/BTCloudApp.hpp \
			include/entities/pacote_posicao.pb.h \
			include/entities/bluetec400.pb.h \
			include/clock.h \
    include/util/ProtocolUtil.hpp

OTHER_FILES += resources/app.config \

include("app.pri")
include("compiler.pri")

DISTFILES += \
    resources/Model.json
