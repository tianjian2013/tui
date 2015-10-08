#include "global.h"

class InputForm;
class ConfigLoader
{
private:
  Json::Reader reader; 
  Json::Value root;
public:
  bool Open(const std::string &path);
  std::string GetNodeType(const std::string &name);
  std::string GetNextWidgetName(const std::string &name, int index);
  std::string GetMenuTitle(const std::string &name);
  bool GetMemuItems(const std::string &name, std::vector <std::string> &indexNames, std::vector <std::string> &itemNames, std::vector <std::string> &nextItems);
  bool Read(InputForm * pi);
  bool GetCommand(const std::string &name, std::string &command, std::string &message);
};
