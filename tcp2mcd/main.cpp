#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "libzet/logger.h"
#include "tcprecvserver.h"
#include "depacker.h"
#include "udpsender.h"
#include "libzet/lbconfig.h"

using namespace Configuration;

static volatile bool run = true;

/// Oбpaбomчuk coбыmuй
static void signal_handler(int sig)
{
	switch(sig)
	{
	case SIGTERM:
		run = false;
	break;
	default: break;
	}
}

int main(int argc, char** argv)
{
	ZLogger* l(ZLogger::Instance());
    //l -> init(APP_NAME, ZLOG_DEBUG, 10, ZLOG_OUTPUT_FILE | ZLOG_OUTPUT_TTY, (char*)"log.txt");
    //l -> init(APP_NAME, ZLOG_DEBUG, 10, ZLOG_OUTPUT_FILE, (char*)"log_tcp2mcd.txt");
    l -> init(APP_NAME, ZLOG_NOTICE, 10, ZLOG_OUTPUT_FILE, (char*)"log_tcp2mcd.txt");
    //l -> init(APP_NAME, ZLOG_DEBUG, 10);
    //l -> init(APP_NAME, ZLOG_NOTICE, 10);

	l -> log(ZLOG_NOTICE, 2, "main(argc = %d, argv = %p)\n", argc, argv);

	LibConf* conf(LibConf::Instance());
	string file_name("");
	if(argc <= 1)
		file_name = string(string(PATH_PREFIX) + string("etc/") + string(APP_NAME) + string("/") + string("conf.cfg"));
	else
		file_name = string(string(PATH_PREFIX) + string("etc/") + string(APP_NAME) + string("/") + string(argv[1]));
	bool is_open = conf -> open(file_name.c_str());
	if(!is_open)
	{
		l -> log(ZLOG_ERR, 2, "main():    error in the file %s: %s!\n", file_name.c_str(), conf -> errorText().c_str());
		exit(EXIT_FAILURE);
	}
	bool is_parse = conf -> parse();
	if(!is_parse)
	{
		l -> log(ZLOG_ERR, 2, "main():    the file %s: doesn't parse: %s!\n", file_name.c_str(), conf -> errorText().c_str());
		exit(EXIT_FAILURE);
	}

	pid_t pid = fork();

	if(pid < 0) exit(EXIT_FAILURE);
	if(pid > 0) exit(EXIT_SUCCESS);

	umask(0);
	pid_t sid = setsid();
	if(sid < 0) exit(EXIT_FAILURE);

	if((chdir("/")) < 0)
		exit(EXIT_FAILURE);

	signal(SIGTERM, signal_handler);

	Depacker dep;
	TCPRecvServer tcp_r(&dep);
	UDPSender udp_s(&dep);

	tcp_r.start();
	udp_s.start();

	while(run)
		usleep(10 * 1000);

	udp_s.stop();
	tcp_r.stop();

    //tcp_r.deleteThreads();

    l -> log(ZLOG_NOTICE, 2, "main():    end of the function main()\n");

	/*close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);*/

	exit(EXIT_SUCCESS);
}
