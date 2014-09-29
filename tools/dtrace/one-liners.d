sudo dtrace -n 'proc:::exec-success { trace(func(pc)); }'

sudo dtrace -n 'syscall::read:return { @["size"] = quantize(arg0); }'

sudo dtrace -n 'syscall::open:entry { printf("%s %s", execname, copyinstr(arg0); }'

sudo dtrace -n 'profile:::profile-99 { @[stack()] = count(); }'

sudo dtrace -qn 'syscall::kill:entry { printf("%Y: %s (pid: %d) sends signal %d to pid %d\n", walltimestamp, execname, pid, arg1, arg0); }'

// no symbols available
sudo dtrace -qn 'syscall::read:entry /execname == "Google Chrome"/ { @[ustack()] = count(); }'

sudo dtrace -n 'profile-997 /arg0/ { @[stack()] = count(); } tick-10s {exit(0);} END { trunc(@, 10); }'

sudo dtrace -n 'profile-997 /arg0/ { @[func(arg0)] = count(); } tick-10s {exit(0);} END { trunc(@, 10); }'

sudo dtrace -n 'profile-997 /execname == "Google Chrome"/ { @[cpu] = count(); } tick-10s {exit(0);}'

sudo dtrace -n 'sched:::on-cpu /execname == "Google Chrome"/ { self->ts = vtimestamp; } sched:::off-cpu /self->ts/ { @["elapsed"] = quantize(vtimestamp - self->ts); self->ts = 0; } tick-60s {exit(0);}'

sudo dtrace -n 'fbt::kmem_cache_alloc:entry { @[stack()] = count(); }'

sudo dtrace -n 'pid$target::malloc:entry { @["size", ustack()] = quantize(arg0); }' -p 75518

sudo dtrace -n 'vminfo:::as_fault /pid == 75518/ { @[ustack()] = count(); }'
