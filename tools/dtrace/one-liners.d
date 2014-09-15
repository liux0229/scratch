sudo dtrace -n 'proc:::exec-success { trace(func(pc)); }'

sudo dtrace -n 'syscall::read:return { @["size"] = quantize(arg0); }'

sudo dtrace -n 'syscall::open:entry { printf("%s %s", execname, copyinstr(arg0); }'

sudo dtrace -n 'profile:::profile-99 { @[stack()] = count(); }'

sudo dtrace -qn 'syscall::kill:entry { printf("%Y: %s (pid: %d) sends signal %d to pid %d\n", walltimestamp, execname, pid, arg1, arg0); }'
