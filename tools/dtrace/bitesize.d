#! /usr/sbin/dtrace -s

#pragma D option quiet

dtrace:::BEGIN
{
}

io:::start
{
    this->size = args[0]->b_bcount;
    @Size[pid, curpsinfo->pr_psargs] = quantize(this->size);
}

dtrace:::END
{
    printa("%d %s\n%@d\n", @Size);
}
