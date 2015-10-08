#include "widgets.h"
#include "inputForm.h"
#include <algorithm>
#include <sstream>

//std::ofstream out("out.txt");


Widget::Widget(const std::string & jfilePath):currentItemIndex(0),
                                                                             currentItemsNumber(MAXITEMNUMBER-1),
                                                                             outputTextpos(1),
                                                                             exitAfterRunCommand(false),
                                                                             currentCharColor(0),
                                                                             jsonFilePath(jfilePath)

{

}

bool Widget::Init()
{
  if (!jLoader.Open(jsonFilePath)) 
  {
    return false;
  }
  if (initscr() == NULL)
  {
    printf("Error: Couldn't entry the curses mode .\n");
    return false;  
  }
  if (!has_colors())
	{	
		printf("Your terminal does not support color\n");
		return false;
	}
  else
  { 
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK); 
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK); 
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
  }
  cbreak();  //stop line buffering 
  noecho();  
  keypad(stdscr, TRUE);    //enables the reading of function keys like F1, F2, arrow keys etc
  srceenLines = LINES;     // size of the main window
  srceenCols = COLS/2;
  outputTextCols = COLS-8; //size of the command output window 
  outputTextLines = srceenLines-10;
  int i = 0;
  for (; i < MAXITEMNUMBER; ++i)
  {
    indexNames[i] = new char [MAXINDEXNAMESIZE+1];
    if (!indexNames[i]) 
    {
      break;
    }
    itemNames[i] = new char [MAXITEMNAMESIZE+1];
    if (!itemNames[i]) 
    {
      break;
    }
    memset(indexNames[i], ' ', MAXINDEXNAMESIZE);
    memset(itemNames[i], ' ', MAXITEMNAMESIZE);
  } 
  if (i < MAXITEMNUMBER)
  {
    for (int j = 0; j <= i; j++)
    {
      if (indexNames[i])   
      {
        delete indexNames[j];
      }
      if (itemNames[i])  
      {
        delete itemNames[j];
      }
    }
    return false;
  }
  items = (ITEM **)calloc(MAXITEMNUMBER+1, sizeof(ITEM *));
  for (int i = 0; i < MAXITEMNUMBER; ++i)
  {
    items[i] = new_item(indexNames[i], itemNames[i]);
  }
  items[MAXITEMNUMBER] = new_item((char *)NULL, (char *)NULL);
  menu = new_menu((ITEM **)items);  
  win = newwin(srceenLines, srceenCols, 0, 0);
  keypad(win, TRUE);
  set_menu_win(menu, win);
  set_menu_sub(menu, derwin(win, 0, 0, 2, 1));
  set_menu_mark(menu, " * "); 
  box(win, 0, 0);
  post_menu(menu);
  wrefresh(win);
  panel = new_panel(win);
  
  wattron(win, A_UNDERLINE);
  mvwprintw(win, srceenLines-2, 5, "%s", " press Esc to go back ");
  wattroff(win, A_UNDERLINE);
  
  outputtextWindow = newwin(outputTextLines, outputTextCols, srceenLines/2-outputTextLines/2, srceenCols-outputTextCols/2-1);
  box(outputtextWindow, 0, 0);
  outputtextPanel = new_panel(outputtextWindow);
  keypad(outputtextWindow, TRUE);
  hide_panel(outputtextPanel);

  infoLines = srceenLines;
  infoCols = COLS/2;
  infoStarty = 0;
  infoStartx = COLS/2-2;
  infoWindow = newwin(infoLines, infoCols, infoStarty, infoStartx);
  infoPanel = new_panel(infoWindow);
  
  confirmBox.Init();
  set_escdelay(0);
  curs_set(0);
  return true;
}
  
void Widget::Run()
{
  std::string message;
  jLoader.GetCommand("info", infoCommand, message);
  setItem("mainMenu");
  update_panels();
  doupdate();

  int c = 0;
  while (!exitAfterRunCommand && ( c = wgetch(win)) != KEY_F(1))
  {
    switch (c)
    {	
      case KEY_DOWN:
        currentItemIndex++;
        if (currentItemIndex >= currentItemsNumber) 
        {
          currentItemIndex = currentItemsNumber-1; 
        }
        else 
        {
          menu_driver(menu, REQ_DOWN_ITEM);
        }
        break;
      case KEY_UP:
        currentItemIndex--;
        if (currentItemIndex < 0)
        {
          currentItemIndex = 0;
        }  
        else
        {
          menu_driver(menu, REQ_UP_ITEM);
        }
        break;   
      case KEY_ESC:
        path.pop_back();
        if (path.empty()) return;
        else
        {
          changeMenuItem(path[path.size()-1]);
        }
        break;
      case KEY_NL:
        setItem(nextItems[currentItemIndex]);
        break;  
      default:
        if (c > '0' && c <= '0'+currentItemsNumber)
        {
          int objectIndex = c-'0'-1;
          while (currentItemIndex < objectIndex)
          {
            currentItemIndex++;
            menu_driver(menu, REQ_DOWN_ITEM);
          }
          while (currentItemIndex > objectIndex)
          {
            currentItemIndex--;
            menu_driver(menu, REQ_UP_ITEM);
          }  
          update_panels();
          doupdate();
          setItem(nextItems[currentItemIndex]); 
        }
        break;           
    } 
    update_panels();
    doupdate();
  }
}
  
void Widget::setItem(std::string name)
{
  std::string type = jLoader.GetNodeType(name);
  if (type.compare("menu") == 0)
  {
    changeMenuItem(name);
  }
  else if (type.compare("input") == 0)
  { 
    InputForm *p;
    if (forms.count(name) == 0) 
    { 
      p = new InputForm(name);
      if (!jLoader.Read(p))
      {
        return ;
      }
      p->Init();
      forms[name] = p;
    }
    else
    {
      p = forms[name];
    }
    wattron(win, A_UNDERLINE);
    mvwprintw(win, srceenLines-2, 5, "%s", "use  arrow keys           ");
    wattroff(win, A_UNDERLINE);
    wrefresh(win);
    curs_set(1);
    std::string command = p->Run();
    curs_set(0);
    wattron(win, A_UNDERLINE);
    mvwprintw(win, srceenLines-2, 5, "%s", " press Esc to go back ");
    wattroff(win, A_UNDERLINE);
    if (command.compare("cancle") != 0)
    {
      runCommand(command, outputtextWindow, outputtextPanel, true);
    }
  }
  else if (type.compare("command") == 0)
  {
    std::string command;
    std::string message;
    exitAfterRunCommand = jLoader.GetCommand(name, command, message);
    clearOutputWindow(confirmBox.win);
    if (message.empty() || confirmBox.Run(message))
    {
      runCommand(command, outputtextWindow, outputtextPanel, true);
    }
    else
    {
      exitAfterRunCommand = false;
    }  
  }
  else if (type.compare("system") == 0)
  {
    std::string command, message;
    jLoader.GetCommand(name, command, message);
    def_prog_mode();
    endwin();
    system(command.c_str());
    reset_prog_mode();
    refresh();
  }
  //runCommand(infoCommand);
}

void Widget::clearOutputWindow(WINDOW *win)
{
  int rows = 0;
  int cols = 0;
  getmaxyx(win, rows, cols);

  std::string str(cols-2, ' ');
  for (int i = 1; i< rows-1; i++)
  {
    mvwprintw(win, i, 1, "%s", str.c_str());
  } 
  wrefresh(win);
}


void Widget::showKeyValuesInInfoWindow(std::vector<std::string> &itemsKeys, std::vector<int> &keysSizes, std::vector<std::string> &itemsValues, int &lines)
{
  if (keysSizes.empty())
  {
    return;
  }
  int colonPos = *std::max_element(keysSizes.begin(), keysSizes.end());
  for (unsigned int j = 0; j < keysSizes.size(); j++)
  {
    mvwprintw(infoWindow, lines, 1, "%s", itemsKeys[j].c_str());
    mvwaddch(infoWindow, lines, colonPos+1, itemsKeys[j].empty() || itemsValues[j].empty()?' ':':');
    waddch(infoWindow, ' '); 
    for (unsigned int k = 0; k < itemsValues[j].size(); k++)
    {
      int y, x;
      getyx(infoWindow, y, x);
      if (x >= infoCols-1)
      {
        mvwaddch(infoWindow, ++lines, colonPos+2, ' ');
      }
      waddch(infoWindow, itemsValues[j][k]); 
    }
    lines++;
   }
   keysSizes.clear();
   itemsKeys.clear();
   itemsValues.clear();
}

void Widget::runCommand(std::string name)
{
  update_panels();
  doupdate();
  show_panel(infoPanel);
  clearOutputWindow(infoWindow);
  
  FILE *stream = popen(name.c_str(), "r"); 
  std::vector<std::string > outputStrings;
  std::string stringLine;
  while (!feof(stream))
  {
    char buf[infoCols-OUTPUTWINDOWGAP];
    memset(buf, '\0', infoCols-OUTPUTWINDOWGAP);
    int cnt = fread(buf, sizeof(char), infoCols-OUTPUTWINDOWGAP-1, stream);
    for (int i = 0; i < cnt; i++) 
    {
      if (buf[i] == '\n')
      {
        outputStrings.push_back(stringLine);
        stringLine.clear();
      } 
      else
      {
        stringLine.push_back(buf[i]);
      }
    }
  }
  pclose(stream);
  
  int lines = 1;
  char item[infoCols+1];
  item[infoCols] = '\0';
  
  std::vector<std::string> itemsKeys;
  std::vector<int> keysSizes;
  std::vector<std::string> itemsValues;

  for (unsigned int i = 0;i < outputStrings.size(); i++)
  {
    std::string::size_type pos = outputStrings[i].find(":");
    if (pos == std::string::npos)
    {
      showKeyValuesInInfoWindow(itemsKeys, keysSizes, itemsValues, lines);
      memset(item, ' ', infoCols);
      outputStrings[i].copy(item, outputStrings[i].size());
      wattron(infoWindow, COLOR_PAIR(6)|A_STANDOUT);
      mvwprintw(infoWindow, lines++, 1, "%s", item);
      wattroff(infoWindow, COLOR_PAIR(6)|A_STANDOUT);
    }
    else
    {
      keysSizes.push_back(pos);
      itemsKeys.push_back(outputStrings[i].substr(0, pos));
      itemsValues.push_back(outputStrings[i].substr(pos+1)); 
    }
  }
  showKeyValuesInInfoWindow(itemsKeys, keysSizes, itemsValues, lines);
  box(infoWindow, 0, 0);
  wrefresh(infoWindow);
}


void Widget::translateAline(std::string str)
{
  std::string stringLine(outputTextCols-OUTPUTWINDOWGAP, ' ');
  std::vector<int> colorChangePos;
  std::vector<int> colorChangeInfo;
  int index = 0;
  int n = str.size();
  int status = 0;
  std::string formatStr;
  int size = 0;
  for (int i = 0;i < n;i++)
  {
    if (status == 0)
    {
      size = index+1;
      if (str[i] == KEY_ESC)
      {
        status = 1;
        continue;
      }
      else if (str[i] == KEY_CR)
      {
        index = 0;
      }
      else
      {  
        if (index >= outputTextCols-OUTPUTWINDOWGAP)
        {
          outputStrings.push_back(stringLine);
          outputStringsColorPos.push_back(colorChangePos);
          outputStringsColors.push_back(colorChangeInfo);
          updateOutputWindow();
          stringLine = std::string(outputTextCols-OUTPUTWINDOWGAP, ' ');
          colorChangePos.clear();
          colorChangeInfo.clear();
          index = 0;
        }
        //if (str[i] >= 32 && str[i] <= 126)
        //{
        stringLine[index++] = str[i];
        //}   
      }   
    }
    else
    {
      if (str[i] == 'G')
      {
        std::string shift = formatStr.substr(1);
        std::stringstream ss;
        ss<<shift;
        int k;
        ss>>k;
        index = k;
        formatStr.clear();
        status = 0;
      }
      else if (str[i] == 'm')
      {
        std::vector<int> colorInfo;
        int num = 0;
        for (unsigned int i = 1; i < formatStr.size(); i++)
        {
          if (formatStr[i] == ';')
          {
            colorInfo.push_back(num);
            num = 0;
          }
          else
          {
            num = num*10+(int)(formatStr[i]-'0');
          }
        }
        colorInfo.push_back(num);
        colorChangePos.push_back(index);
        if (colorInfo.size() < 2)
        {
          colorChangeInfo.push_back(0);
        }
        else
        { 
          colorChangeInfo.push_back(colorInfo[1]%10); 
        }
        formatStr.clear();
        status = 0;
      }
      else
      {
        formatStr.push_back(str[i]);
      }
    }     
  }
  outputStrings.push_back(stringLine);
  outputStringsColorPos.push_back(colorChangePos);
  outputStringsColors.push_back(colorChangeInfo);
  updateOutputWindow();        
}

void Widget::printAlineInOutputWindow(int outputPos, int outputStringsIndex)
{
  int pos = 0;
  if (outputStringsIndex > outputStrings.size())
  {
    return;
  }
  int n = outputStrings[outputStringsIndex].size();
  int segment = outputStringsColorPos[outputStringsIndex].size();
  mvwaddch(outputtextWindow, outputPos, 1, ' ');
  for (int i = 0; i < segment; i++)
  {
    while (pos < outputStringsColorPos[outputStringsIndex][i])
    {
        waddch(outputtextWindow, outputStrings[outputStringsIndex][pos++]);
    }
    if (currentCharColor)
    {
      wattroff(outputtextWindow, COLOR_PAIR(currentCharColor));
      currentCharColor = 0;
    }
    else
    {
      currentCharColor = outputStringsColors[outputStringsIndex][i];
      wattron(outputtextWindow, COLOR_PAIR(currentCharColor));  
    }
  }
  while (pos < n)
  {
    waddch(outputtextWindow, outputStrings[outputStringsIndex][pos++]);
  }
}

void Widget::updateOutputWindow()
{
  if (outputTextpos <= outputTextLines-OUTPUTWINDOWGAP)
  {
    printAlineInOutputWindow(outputTextpos++, outputStrings.size()-1);
  }
  else 
  {
    for (int j = 1; j < outputTextpos; j++)
    {
      printAlineInOutputWindow(j, outputStrings.size()-outputTextpos+j);
    }
  }
  box(outputtextWindow, 0, 0);
  wrefresh(outputtextWindow);
}

void Widget::updateOutputWindow(int top)
{
  clearOutputWindow(outputtextWindow); 
  for (int j = 1; j <= outputTextLines-OUTPUTWINDOWGAP; j++)
  {
    printAlineInOutputWindow(j, top+j-outputTextLines+OUTPUTWINDOWGAP);
  }
  if (currentCharColor)
  {
    wattroff(outputtextWindow, COLOR_PAIR(currentCharColor));
    currentCharColor = 0;
  } 
  box(outputtextWindow, 0, 0);
  wrefresh(outputtextWindow);
}

void Widget::runCommand(std::string name, WINDOW *outputWindow, PANEL *outputPanel, bool wait)
{
  
  name.insert(0, "unbuffer ");
  outputTextpos = 1;
  int rows, cols;
  getmaxyx(outputWindow, rows, cols);
	
  wattron(win, A_UNDERLINE);
  mvwprintw(win, srceenLines-2, 5, "%s", "press enter to go back     ");
  wattroff(win, A_UNDERLINE); 

  update_panels();
  doupdate();
  show_panel(outputPanel);

  clearOutputWindow(outputWindow);
  outputStrings.clear();
  outputStringsColorPos.clear();
  outputStringsColors.clear();

  def_prog_mode();
  endwin();
  FILE *stream = popen(name.c_str(), "r"); 
  reset_prog_mode();
  refresh();

  std::string stringLine;

  while (!feof(stream))
  {
    char buf[cols-OUTPUTWINDOWGAP]; 
    memset(buf, '\0', cols-OUTPUTWINDOWGAP);
    int cnt = fread(buf, sizeof(char), cols-OUTPUTWINDOWGAP-1, stream); 
    for (int i = 0; i < cnt; i++)
    {
      if (buf[i] == '\n')
      {
        translateAline(stringLine);
        stringLine.clear();
      }
      else
      {
        stringLine.push_back(buf[i]);
      }
    }  
  }
  pclose(stream);
  int c = 0;
  int top = outputStrings.size()-1;
  while (wait && !exitAfterRunCommand && (c = wgetch(win)) != KEY_NL&& c != KEY_ESC)
  {
    if (outputStrings.size() <= outputTextLines-OUTPUTWINDOWGAP)
    {
       continue;
    }
    switch (c)
    {	
      case KEY_NPAGE:
        top += outputTextLines-OUTPUTWINDOWGAP;
        if (top > outputStrings.size()-1) 
        {
          top = outputStrings.size()-1;
        }
        updateOutputWindow(top);
        break;
      case KEY_PPAGE:
        top -= (outputTextLines-OUTPUTWINDOWGAP);
        if (top < outputTextLines-OUTPUTWINDOWGAP-1) 
        {
          top = outputTextLines-OUTPUTWINDOWGAP-1;
        }
        updateOutputWindow(top);
        break;
      case KEY_UP:
        top--;
        if (top < outputTextLines-OUTPUTWINDOWGAP-1) 
        {
          top = outputTextLines-OUTPUTWINDOWGAP-1;
        }
        else
        {
          updateOutputWindow(top);
        }   
        break;    
      case KEY_DOWN:
        top++;
        if (top > outputStrings.size()-1) 
        {
          top = outputStrings.size()-1;
        }
        else
        {
          updateOutputWindow(top);
        }
        break;    
    } 
  }
  hide_panel(outputPanel);
  wattron(win, A_UNDERLINE);
  mvwprintw(win, srceenLines-2, 5, "%s", "press Esc to go back    ");
  wattroff(win, A_UNDERLINE);
  update_panels();
  doupdate();
}

void Widget::changeMenuItem(std::string name)
{    
  if (path.empty() || name.compare(path[path.size()-1]) != 0)
  {
    path.push_back(name);
  }
  std::vector <std::string> menuIndexNames;
  std::vector <std::string> menuItemNames;
  if (!jLoader.GetMemuItems(name, menuIndexNames, menuItemNames, nextItems))
  {
    return;
  }
  int menuItemNumber = menuIndexNames.size();
  for (int i = 0; i < menuItemNumber; i++)
  {  
    memset(indexNames[i], ' ', MAXINDEXNAMESIZE);
    menuIndexNames[i].copy(indexNames[i], std::max((int)menuIndexNames[i].size(), MAXINDEXNAMESIZE));
    memset(itemNames[i], ' ', MAXITEMNAMESIZE);
    menuItemNames[i].copy(itemNames[i], std::max((int)menuItemNames[i].size(), MAXITEMNAMESIZE));
  }
  for (int i = menuItemNumber; i < MAXITEMNUMBER; i++)
  {
    memset(indexNames[i], ' ', MAXINDEXNAMESIZE);
    memset(itemNames[i], ' ', MAXITEMNAMESIZE);
  }
  box(win, 0, 0);
  wattron(win, COLOR_PAIR(1));
  std::string title = jLoader.GetMenuTitle(name);
  mvwprintw(win, 0, srceenCols/2-title.size()/2, "%s", title.c_str());
  wattroff(win, COLOR_PAIR(1));
  
  while (currentItemIndex < MAXITEMNUMBER-1)
  {
    currentItemIndex++;
    menu_driver(menu, REQ_DOWN_ITEM);
  }
  while (currentItemIndex>0)
  { 
    currentItemIndex--; 
    menu_driver(menu, REQ_UP_ITEM);
  }
  currentItemsNumber = menuItemNumber;
}

void Widget::Release()
{
  unpost_menu(menu);
  free_menu(menu);
  for (int i = 0; i < MAXITEMNUMBER; ++i)
  {
    free_item(items[i]);
    delete  indexNames[i];
    delete  itemNames[i];   
  }

  std::map<std::string, InputForm*> ::iterator  iterForms = forms.begin();
  while (iterForms != forms.end())
  {
    delete iterForms->second;
    iterForms++;
  }
  endwin();
  system("clear");
}

