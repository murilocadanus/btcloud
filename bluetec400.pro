SOURCES +=	source/main.cpp \
			source/Bluetec400App.cpp

HEADERS +=	\
			include/Defines.hpp \
			include/Bluetec400App.hpp

OTHER_FILES += resources/app.config \

include("app.pri")
include("compiler.pri")

