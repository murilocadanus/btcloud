TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH +=	include/ \
				../libi9/include
				/usr/include/apr-1.0

CONFIG(debug, debug|release) {
	TARGET = $${TARGET}-debug
}

CONFIG(release, debug|release) {
	TARGET = $${TARGET}
}

unix {

	LIBS	+= -L ../libi9/lib -lI9 -pthread -lmongoclient -lcurl -lyajl -lboost_system -lboost_thread -lactivemq-cpp -lmysqlclient

	#Configs
	APP_CONFIG_FILES.files = $$files($${PWD}/resources/*.*)
	APP_CONFIG_FILES.path = $${OUT_PWD}/$${DESTDIR}
	APP_CONFIG_FILES.commands += test -d $${APP_CONFIG_FILES.path} || mkdir -p $${APP_CONFIG_FILES.path} &&
	APP_CONFIG_FILES.commands += ${COPY_FILE} $$APP_CONFIG_FILES.files $${APP_CONFIG_FILES.path}

	QMAKE_EXTRA_TARGETS += APP_CONFIG_FILES
	POST_TARGETDEPS += APP_CONFIG_FILES
}
