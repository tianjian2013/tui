#include "global.h"
#include "configLoader.h"
#include "confirmBox.h"

class InputForm;

class Widget
{
private:
//  static const int MAXITEMNUMBER;
 // static const int MAXINDEXNAMESIZE;
 // static const int MAXITEMNAMESIZE;
 // static const int OUTPUTWINDOWGAP;
  
  int srceenLines;
  int srceenCols;
  char *indexNames[MAXITEMNUMBER];
  char *itemNames[MAXITEMNUMBER];

  WINDOW *win;
  PANEL *panel;
  ITEM **items;				
  MENU *menu;
  int currentItemIndex;
  int currentItemsNumber;
  std::vector<std::string> nextItems;
  
  std::map<std::string, InputForm*> forms;
  std::vector< std::string> path;

  int outputTextCols;
  int outputTextLines;
  int outputTextpos;
  WINDOW *outputtextWindow;
  PANEL *outputtextPanel;
  bool exitAfterRunCommand;
  std::vector<std::string > outputStrings;
  std::vector<std::vector<int > > outputStringsColorPos;
  std::vector<std::vector<int > > outputStringsColors;
  int currentCharColor;

  //info window
  WINDOW *infoWindow;
  PANEL *infoPanel;
  int infoLines, infoCols, infoStarty, infoStartx;
  //config file
  std::string jsonFilePath;
  ConfigLoader jLoader;
  std::string infoCommand;
  
  //
  ConfirmBox confirmBox;

private:
  void runCommand(std::string name, WINDOW *outputWindow, PANEL *outputPanel, bool wait);
  void runCommand(std::string name);
  void clearOutputWindow(WINDOW *outputWindow);
  void changeMenuItem(std::string name);
  void setItem(std::string name);
  void translateAline(std::string str);
  void updateOutputWindow();
  void updateOutputWindow(int top);
  void printAlineInOutputWindow(int outputPos, int outputStringsIndex);
  void showKeyValuesInInfoWindow(std::vector<std::string> &itemsKeys, std::vector<int> &keysSizes, std::vector<std::string> &itemsValues, int &lines);
public:
  Widget(const std::string &jfilePath);
  bool Init();
  void Run();
  void Release();
};
