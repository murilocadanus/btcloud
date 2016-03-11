SOURCES +=	source/main.cpp \
			source/Protocol.cpp \
			source/BTCloudApp.cpp \
			source/managers/BlueTecFileManager.cpp \
			source/io/FileMod.cpp \
			source/io/SwapFile.cpp \
			source/io/File.cpp \
			source/util/ProtocolUtil.cpp


HEADERS +=	\
			include/Defines.hpp \
			include/Protocol.hpp \
			include/BTCloudApp.hpp \
			include/managers/BlueTecFileManager.h \
			include/io/IFile.h \
			include/io/FileMod.h \
			include/io/SwapFile.h \
			include/io/File.h \
			include/util/ProtocolUtil.hpp \
    include/entities/Bluetec400.hpp

OTHER_FILES += resources/app.config \

include("app.pri")
include("compiler.pri")

DISTFILES += \
			resources/Model.json
