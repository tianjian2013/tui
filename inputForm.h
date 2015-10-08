#include "global.h"

class InputForm
{
public:
  std::string id;
  std::string title;  
  bool okCancle;
  int index;
  int choices;

  WINDOW *win;
  PANEL *panel;
  FIELD **field; 
  FORM *form;

  int formRows, formCols;
  std::vector<std::string> itemNames;
  std::vector<std::string> itemTypes;
  std::vector<std::string> itemInputs;
  std::vector<std::string> defaultValues;
  std::string command;
  std::string function;

  void fillField();
  void delAchar();
  void switchOkCancle();
  std::string makeCommand();
  void fillCheckList(bool choose);
public:
  InputForm(std::string name);
  bool Init();
  ~InputForm();
  std::string Run();
};
