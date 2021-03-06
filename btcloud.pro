SOURCES +=	source/main.cpp \
			source/Protocol.cpp \
			source/BTCloudApp.cpp \
			source/managers/BlueTecFileManager.cpp \
			source/io/FileMod.cpp \
			source/io/SwapFile.cpp \
			source/io/File.cpp \
			source/util/ProtocolUtil.cpp \
			source/queue/BT4Consumer.cpp \
			source/queue/BT4Producer.cpp \
			source/entities/Message.cpp

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
			include/entities/ProtocolEntity.hpp \
			include/queue/BT4Consumer.hpp \
			include/queue/BT4Producer.hpp \
			include/entities/Message.hpp

OTHER_FILES += resources/app.config \

include("app.pri")
include("compiler.pri")
