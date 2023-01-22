int isHandler(int signo)
{
  if(block & signo)
  {
    // 阻塞了 根本不管有没有信号
  }
  else
  {
    // 没有被block
    if (signo & pending)
    {
      // 该信号被收到了
      hadnler_array[signo](signo);
      return 0;
    }
  }
  return 1;
}
