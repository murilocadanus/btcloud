/*
 * NotifyDirectory.cpp
 *
 *  Created on: 01/10/2013
 *      Author: david.cam
 */

#include "NotifyDirectory.h"
#include <chrono>
#include <unistd.h>

NotifyDirectory::NotifyDirectory()
{
	this->init();
}

NotifyDirectory::NotifyDirectory(listenerParam param, std::string path)
{
	this->init();
	this->setListenerParam(param);
	this->setPath(path);
}

void NotifyDirectory::init()
{
	this->param = LOCAL_PATH;
	this->notifica = NULL;
	this->notify_fd = 0;
	this->processing = false;
}

NotifyDirectory::~NotifyDirectory()
{
	this->processing = NULL;
	if( this->notify_fd )
		close(this->notify_fd);
}

void NotifyDirectory::setListnerNotifyDirectory(void (*function)(std::string path))
{
	this->notifica = function;
}

void NotifyDirectory::setListenerParam(listenerParam param)
{
	this->param = param;
}

void NotifyDirectory::setPath(std::string path)
{
	this->path = path;
}

std::string NotifyDirectory::getPath()
{
	return this->path;
}

void NotifyDirectory::listDir(std::string directory)
{
	struct dirent *dir;
	DIR  *d;
	std::string pasta;

	d = opendir(directory.c_str());

	if (d)
	{
		while ( (dir = readdir(d)) != NULL )
		{
			/* Ignora arquivos ocultos. */
			if( dir->d_name[0] != '.' )
			{
				if( dir->d_type == DT_REG )
				{

					if( this->notifica != NULL )
					{
						std::cout << "Processando Listagem de deretorio " << std::endl;
						(*this->notifica)(directory + dir->d_name);
					}

					//printf("Processar: %s%s\n", directory.c_str(), dir->d_name); // Preciso saber se o arquiv esta sendo editado ou nao.
				}
				else if ( dir->d_type == DT_DIR && this->param == RECURSIVE_PATH)
				{
					pasta = directory;
					pasta += dir->d_name;
					pasta += "/";

					/* Adiciona esta pasta ao monitoramento. */
					listenPath(pasta);
					/* Acessa as pastas recursivamente. */
					listDir(pasta);
				}
			}
		}
		closedir(d);
	}
}

void NotifyDirectory::listenPath(std::string path)
{
	int watch_fd;

	watch_fd = inotify_add_watch(notify_fd, path.c_str(), IN_CLOSE | IN_CREATE);

	if (watch_fd < 0)
	{
		perror("Nao foi possivel adicionar diretorio.");
		exit(EXIT_FAILURE);
	}
	else
	{
		printf("Monitorando diretorio: %s\n", path.c_str());
		listPath[watch_fd] = path;
	}
}

void NotifyDirectory::processEvent(struct inotify_event *event)
{
	char pasta[1024];
	pasta[0]=0;

	/* Nova Pasta criada. */
	if( (event->mask & IN_CREATE) && (event->mask & IN_ISDIR))
	{
		sprintf(pasta,"%s%s/",listPath[event->wd].c_str(),event->name);
		/* Monitora a nova pasta. */
		if( this->param == RECURSIVE_PATH )
			listenPath(pasta);
	}
	else if ((event->mask & IN_CLOSE_WRITE) && !(event->mask & IN_ISDIR))
	{   /* Novo arquivo para processar. */

		if( this->notifica != NULL )
		{

			std::cout << "processando evento" << std::endl;
			(*this->notifica)(listPath[event->wd] + event->name);
		}

		//printf("Processar: %s%s\n",listPath[event->wd].c_str(),event->name);
	}
}

void NotifyDirectory::process()
{
	long input_len;
	char *ptr;
	char buffer[EVENT_BUFFER_LENGTH];
	struct inotify_event *event;

	/* Se já estiver rodando da um close. */
	if( this->notify_fd )
	{
		close(this->notify_fd);
	}

	/* Inicia o notify. */
	this->notify_fd =  inotify_init();
	/* Adiciona a pasta no inotify. */
	listenPath(path);
	/* Lista por subpastas e arquivos enviando direto para o processamento. */
	listDir(path);


	while (this->processing)
	{
		input_len = read(this->notify_fd, buffer, EVENT_BUFFER_LENGTH);
		if (input_len <= 0)
		{
			perror("Erro ao ler do fd inotify.");
			exit(EXIT_FAILURE);
		}

		ptr = buffer;
		while (ptr < buffer + input_len)
		{
			event = (struct inotify_event *) ptr;
			if( event->name[0] != '.' && event->len > 0 )
				processEvent(event);
			ptr += sizeof (struct inotify_event) +event->len;
		}
	}
}


void NotifyDirectory::start()
{
	if( !this->path.length() ) throw "NotifyDirectory: Path não informado.";

	if(! this->processing)
	{
		this->processing = true;
		std::thread (&NotifyDirectory::process,this).detach();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void NotifyDirectory::stop()
{
	std::cout << "stop" << std::endl;
	this->processing = false;
}


