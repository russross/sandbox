#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>

#define MB 1024*1024

char *name;
void usage(void) {
    fprintf(stderr, "Usage: %s [opts] -- <command> <arg1> ...\n"
            "    -i <stdin file>\n"
            "    -o <stdout file>\n"
            "    -e <stderr file>\n"
            "    -m <max memory in MB, default 16, max 1024>\n"
            "    -c <max CPU time in seconds, default 10, max 300>\n"
            "    -t <max time in seconds, default 10, max 300>\n"
            "    -s <syscall to allow>\n", name);
    exit(1);
}

void limit(int resource, rlim_t value) {
    struct rlimit elt;
    elt.rlim_cur = value;
    elt.rlim_max = value;

    if (setrlimit(resource, &elt) < 0) {
        perror("Failure setting resource limits");
        exit(1);
    }
}

int main(int argc, char **argv, char **envp) {
    name = argv[0];

    char *stdin_file = NULL, *stdout_file = NULL, *stderr_file = NULL;
    int max_mem = 16;
    int max_cpu = 10;
    int max_time = 10;

    /* parse command-line args */
    int c;
    while ((c = getopt(argc, argv, "i:o:e:m:c:t:s:")) >= 0) {
        switch (c) {
        case 'i':
            stdin_file = optarg;
            break;
        case 'o':
            stdout_file = optarg;
            break;
        case 'e':
            stderr_file = optarg;
            break;
        case 'm':
            max_mem = atoi(optarg);
            if (max_mem < 1 || max_mem > 1024)
                usage();
            break;
        case 'c':
            max_cpu = atoi(optarg);
            if (max_cpu < 1 || max_cpu > 300)
                usage();
            break;
        case 't':
            max_time = atoi(optarg);
            if (max_time < 1 || max_time > 300)
                usage();
            break;
        case 's':
            break;
        default:
            usage();
        }
    }
    argc -= optind;
    argv += optind;
    if (argc <= 0)
        usage();

    /* set resource limits */
    limit(RLIMIT_AS, max_mem*MB);
    limit(RLIMIT_CORE, 0);
    limit(RLIMIT_CPU, max_cpu);
    limit(RLIMIT_DATA, max_mem*MB);
    limit(RLIMIT_FSIZE, 1*MB);
    limit(RLIMIT_MEMLOCK, 0);
    limit(RLIMIT_MSGQUEUE, 0);
    limit(RLIMIT_NOFILE, 10);
    limit(RLIMIT_NPROC, 1);
    limit(RLIMIT_STACK, max_mem*MB);

    int uid = geteuid();
    if (setreuid(uid, uid) < 0) {
        perror("Failed setting euid");
        exit(1);
    }

    /* launch the program */
    if (execve(argv[0], argv, envp) < 0) {
        perror("Failed to execute program");
        exit(1);
    }

    return 0;
}
