#include "net_config.h"
#include "os/os_compat.h"

#ifdef IPERF_ENABLE
static int iperf_test_pid;
static int iperf_test_pid1;

extern int iperf_main(int argc, char **argv);

static void iperf_test_thread(void *priv)
{
    int argc = 0;
#ifndef LONG_POWER_IPC
    char *argv[32] = {"iperf3", 0}; //p:f:i:D1VJvsc:ub:t:n:k:l:P:Rw:B:M:N46S:L:ZO:F:A:T:C:dI:hX:

    argv[++argc] = "-i";
    argv[++argc] = "60";
    argv[++argc] = "-s";
    argv[++argc] = "-p";
    argv[++argc] = "5001";
//    argv[++argc] = "-V";
//    argv[++argc] = "-d";
#else

    char *argv[32] = {"iperf3", 0}; //p:f:i:D1VJvsc:ub:t:n:k:l:P:Rw:B:M:N46S:L:ZO:F:A:T:C:dI:hX:

    argv[++argc] = "-c";
    argv[++argc] = "43.139.166.136";
    argv[++argc] = "-i";
    argv[++argc] = "1";
    argv[++argc] = "-p";
    argv[++argc] = "8080";
    argv[++argc] = "-t";
    argv[++argc] = "180";

#endif // 0

    iperf_main(++argc, argv);

}

static void iperf_test_thread1(void *priv)
{
    int argc = 0;
    char *argv[32] = {"iperf3", 0}; //p:f:i:D1VJvsc:ub:t:n:k:l:P:Rw:B:M:N46S:L:ZO:F:A:T:C:dI:hX:

    argv[++argc] = "-c";
    argv[++argc] = "43.139.166.136";
    argv[++argc] = "-i";
    argv[++argc] = "1";
    argv[++argc] = "-p";
    argv[++argc] = "8000";
    argv[++argc] = "-t";
    argv[++argc] = "180";
    argv[++argc] = "-R";
    iperf_main(++argc, argv);

}

#endif

void iperf_test(void)
{
#ifdef IPERF_ENABLE

    if (iperf_test_pid == 0) {
        thread_fork("iperf_test", 15, 1 * 1024, 0, &iperf_test_pid, iperf_test_thread, 0);
#ifdef LONG_POWER_IPC
        msleep(2000);
       thread_fork("iperf_test1", 15, 1 * 1024, 0, &iperf_test_pid1, iperf_test_thread1, 0);
#endif	   
    }
#endif
}

