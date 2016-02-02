/*
 * NotifyDirectory.h
 *
 *  Created on: 01/10/2013
 *      Author: david.cam
 */

#ifndef NOTIFYDIRECTORY_H_
#define NOTIFYDIRECTORY_H_
#include <sys/inotify.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <thread>
#include <map>
#define EVENT_BUFFER_LENGTH 100000

class NotifyDirectory {
	public:
		enum listenerParam {LOCAL_PATH, RECURSIVE_PATH};
		NotifyDirectory(listenerParam param, std::string path);
		NotifyDirectory();
		virtual ~NotifyDirectory();
		virtual void setListenerParam(listenerParam param);
		virtual void setPath(std::string path);
		virtual std::string getPath();
		virtual void setListnerNotifyDirectory(void (*function)(std::string path));
		virtual void start();
		virtual void stop();

	private:
		listenerParam param;
		std::string path;
		int notify_fd;
		bool processing;
		void (*notifica)(std::string path);
		std::map<int,std::string> listPath;
		void init();
		void listDir(std::string directory);
		void listenPath(std::string path);
		void process();
		void processEvent(struct inotify_event *event);

};

#endif /* NOTIFYDIRECTORY_H_ */

