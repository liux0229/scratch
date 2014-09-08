#! /usr/sbin/dtrace -s

#pragma D option quiet

dtrace:::BEGIN
{
}

io:::start
{
  this->size = args[0]->b_count;
  @Size[pid, curpsinfo->pr_psargs] = quantize(this->size);
}

dtrace:::END
{
  printa("%8d %S\n%@d\n", @Size);
}
