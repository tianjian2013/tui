#include "global.h"

class ConfirmBox
{
private:
  static const int cols = 50;
  static const int lines = 14; 
  bool okCancle;
  void switchOkCancle();
public:
  WINDOW *win;
  PANEL *panel;
  ConfirmBox();
  bool Init();
  bool Run(std::string &message);
};
