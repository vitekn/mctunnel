#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "udptcpstreamer.h"
#include "udprecvpool.h"
#include "tcpmcoutput.h"
#include "libzet/logger.h"
#include "libzet/lbconfig.h"

using namespace Configuration;

static bool volatile run = true;

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
    //l -> init(APP_NAME, ZLOG_DEBUG, 10, ZLOG_OUTPUT_FILE, (char*)"log_mc2tcpd.txt");
    l -> init(APP_NAME, ZLOG_NOTICE, 10, ZLOG_OUTPUT_FILE, (char*)"log_mc2tcpd.txt");
    //l -> init(APP_NAME, ZLOG_DEBUG, 10);
    //l -> init(APP_NAME, ZLOG_NOTICE, 10);

	l -> log(ZLOG_NOTICE, 2, "main(argc = %d, argv = %p)\n", argc, argv);

	LibConf *conf(LibConf::Instance());
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
	if(sid < 0)
		exit(EXIT_FAILURE);

	if((chdir("/")) < 0)
		exit(EXIT_FAILURE);

	signal(SIGTERM, signal_handler);

	l -> log(ZLOG_DEBUG, 5, "main():    streamer\n");
	UDPTCPStreamer st;

	l -> log(ZLOG_DEBUG, 5, "main():    recvpool\n");
	UDPRecvPool mc_recvs(&st);

	l -> log(ZLOG_DEBUG, 5, "main():    tcpout\n");
	TcpMCOutput mco(&st);

	l -> log(ZLOG_DEBUG, 5, "main():    starting recvs\n");
	mc_recvs.startAll();

	l -> log(ZLOG_DEBUG, 5, "main():    starting tcpout\n");
    mco.start();

	l -> log(ZLOG_DEBUG, 5, "main():    start ok\n");

	while(run)
		sleep(1);

	l -> log(ZLOG_DEBUG, 5, "main():    stopping tcpout\n");
	mco.stop();

	l -> log(ZLOG_DEBUG, 5, "main():    stopping recvs\n");
	mc_recvs.stopAll();

	l -> log(ZLOG_DEBUG, 5, "main():    stopping ok\n");

	st.resetBuf();
	/*close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);*/

	l -> log(ZLOG_NOTICE, 2, "main():    end of the function main()\n");
	exit(EXIT_SUCCESS);
}
