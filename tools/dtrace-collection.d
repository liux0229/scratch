sudo dtrace -n 'proc:::exec-success { trace(func(pc)); }'

sudo dtrace -n 'syscall::read:return { @["size"] = quantize(arg0); }'

sudo dtrace -n 'syscall::open:entry { printf("%s %s", execname, copyinstr(arg0); }'

sudo dtrace -n 'profile:::profile-99 { @[stack()] = count(); }'
