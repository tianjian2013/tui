#include "widgets.h"

int main()
{
  Widget w("tuiJson.conf");
  if (!w.Init()) return 1;
  w.Run();
  w.Release();
  return 0;
}
