#include "configLoader.h"
#include "inputForm.h"
#include <sstream>

bool ConfigLoader::Open(const std::string &path)
{
  std::ifstream ifs;
  ifs.open(path.c_str());
  if (!ifs.is_open())
  {
    std::cout<< "Failed to open : "<< path<<std::endl;
    return false;
  }
  if (!reader.parse(ifs, root, false))
  {
    std::cout<< "Failed to parse configuration : "<< path<<std::endl;
    return false;
  }
  else
  {
    return true;
  }
}

std::string ConfigLoader::GetNodeType(const std::string & name)
{
  const Json::Value Root = root[name]; 
  if (!Root.isNull() && Root["type"].isString())
  {
    return Root["type"].asString();
  }
  else 
  {
    return "unknown";
  }    
}

bool ConfigLoader::Read(InputForm * pi)
{
  const Json::Value Root = root[pi->id];
  const Json::Value array = Root["field"];
  if (Root.isNull() || array.isNull() || !Root["command"].isString() || !Root["function"].isString() || !Root["title"].isString()) 
  {
    return false;
  }
  pi->command = Root["command"].asString();
  pi->function = Root["function"].asString();
  pi->title = Root["title"].asString();
  pi->choices = array.size();
  for (int i = 0; i < pi->choices; ++i)
  {
    if (!array[i]["type"].isString() || !array[i]["name"].isString()) 
    {
      return false;
    }
    pi->itemTypes.push_back(array[i]["type"].asString());
    pi->itemNames.push_back(array[i]["name"].asString());
    if (array[i]["default"].isString()) 
    {
      pi->itemInputs.push_back(array[i]["default"].asString());
    }
    else
    {
       pi->itemInputs.push_back("");
    }
  } 
  return true;
}

bool ConfigLoader::GetCommand(const std::string &name, std::string &command, std::string &message)
{
  const Json::Value Root = root[name];
  if (Root.isNull() || !Root["name"].isString()  || !Root["function"].isString())
  {
    command = "echo unknown command";
    return false;
  }
  if (Root["message"].isString())
  {
    message = Root["message"].asString();
  }
  command += Root["name"].asString();
  command += " ";
  command += Root["function"].asString();
  return Root["exit"].isString() && Root["exit"].asString().compare("yes") == 0;
}

std::string ConfigLoader::GetNextWidgetName(const std::string &name, int index)
{
  std::string nextWidgetName;
  const Json::Value menuRoot = root[name]; 
  if (menuRoot.isNull()) 
  {
    return nextWidgetName;
  }
  const Json::Value array = menuRoot["item"];
  if (array.isNull()) 
  {
    return nextWidgetName;
  }
  int choices = array.size();
  if (index < choices && array[index]["next"].isString())
  {
    nextWidgetName = array[index]["next"].asString();
  }
  return nextWidgetName;
}

std::string ConfigLoader::GetMenuTitle(const std::string &name)
{
  const Json::Value menuRoot = root[name]; 
  if (menuRoot.isNull()) 
  {
    return "";
  }
  if (!menuRoot["title"].isString()) 
  {
    return "";
  }
  else
  {
    return menuRoot["title"].asString();
  }
}

bool ConfigLoader::GetMemuItems(const std::string &name, std::vector <std::string> &indexNames, std::vector <std::string> &itemNames, std::vector <std::string> &nextItems)
{
  nextItems.clear();
  const Json::Value menuRoot = root[name]; 
  if (menuRoot.isNull()) 
  {
    return false;
  }
  const Json::Value  array = menuRoot["item"];
  if (array.isNull()) 
  {
    return false;
  }
  int n = array.size();
  int validItemIndex = 1;
  for (int i = 0; i <n; ++i)
  {
    const Json::Value  conditionNode = array[i]["condition"];
    if (!conditionNode.isNull() && array[i]["condition"].isString())
    {
        continue;
    }
    std::stringstream ss;
    ss<<validItemIndex++;
    std::string str;
    ss>>str;
    indexNames.push_back(str);
    if (array[i]["itemName"].isString())
    {
      itemNames.push_back(array[i]["itemName"].asString());
    }
    else
    {
      return false;
    }
    if (array[i]["next"].isString())
    {
      nextItems.push_back(array[i]["next"].asString());
    }
    else
    {
      return false;
    }
  }
  return true;
}


