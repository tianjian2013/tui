#include "inputForm.h"
#include "global.h"

InputForm::InputForm(std::string name):id(name),
                                       okCancle(false), 
                                       index(0)
{
}

bool InputForm::Init()
{
  field = (FIELD **)calloc(choices*2+1, sizeof(FIELD *));
  for (int i = 0; i < choices; ++i)
  {
    field[2*i] = new_field(1, 30, 4+i*2, 2, 02, 02);
    field[2*i+1] = new_field(1, 20, 4+i*2, 30, 02, 02);
    set_field_back(field[2*i+1], A_UNDERLINE);
    field_opts_off(field[2*i+1], O_AUTOSKIP);
    set_field_buffer(field[2*i], 0, itemNames[i].c_str());
    set_field_back(field[2*i], O_EDIT);
  }
  field[choices*2] = NULL;
  form = new_form(field);
  scale_form(form, &formRows, &formCols);
  win = newwin(formRows+8, formCols+20, LINES/2-formRows/2 -6, COLS/2-10-formCols/2);
  keypad(win, TRUE);
  set_form_win(form, win);
  set_form_sub(form, derwin(win, formRows, formCols, 2, 2));
  box(win, 0, 0);
  wattron(win, COLOR_PAIR(1));
  
  mvwprintw(win, 0, formCols/2+8-title.size()/2, "%s", title.c_str());
  wattroff(win, COLOR_PAIR(1));
  post_form(form);
  panel = new_panel(win);
  switchOkCancle();
  
  update_panels();
  doupdate();
  form_driver(form, REQ_NEXT_FIELD);
  return true;
}
  
InputForm::~InputForm()
{ 
  unpost_form(form);
  free_form(form);
  for (int i = 0;i < choices;i++)
  {
    free_field(field[i]);
  } 
}

void InputForm::switchOkCancle()
{
  okCancle = !okCancle;
  if (okCancle)
  {
    wattron(win, COLOR_PAIR(2)|A_STANDOUT);
    mvwprintw(win, formRows+5, formCols+10, "%s", "  OK  ");
    wattroff(win, COLOR_PAIR(2)|A_STANDOUT);
    mvwprintw(win, formRows+5, formCols-5, "%s", "  CANCLE  ");
  }
  else
  {
    wattron(win, COLOR_PAIR(2)|A_STANDOUT);
    mvwprintw(win, formRows+5, formCols-5, "%s", "  CANCLE  ");
    wattroff(win, COLOR_PAIR(2)|A_STANDOUT);
    mvwprintw(win, formRows+5, formCols+10, "%s", "  OK  ");
  }
}
  
std::string InputForm::makeCommand()
{
  std::string wholeCommand;
  wholeCommand += command;
  wholeCommand += " ";
  wholeCommand += function;
  wholeCommand += " ";
  for (int i = 0;i < choices;i++)
  {
    if (itemInputs[i].empty()) 
    {
      wholeCommand += "null";
    }
    else
    {
      wholeCommand += itemInputs[i];
    }
    wholeCommand += " ";
  }
  return wholeCommand;
}

void InputForm::fillField()
{
  int n = itemInputs[index].size();
  if (itemTypes[index].compare("checklist") == 0)
  {
    fillCheckList(itemInputs[index].compare("y") == 0);
  }
  else if (itemTypes[index].compare("password") == 0)
  {
    for (int i = 0; i < n; i++)
    {
      form_driver(form, '*');  
    }
  }
  else
  { 
    for (int i = 0; i < n; i++)
    {
      form_driver(form, itemInputs[index][i]);
    }
  }
}
  
void InputForm::delAchar()
{  
  if (itemTypes[index].compare("checklist") == 0)
  {
    return;
  }
  int n = itemInputs[index].size();
  if (n == 0) return ;  
  form_driver(form, REQ_DEL_PREV); 
  itemInputs[index].erase(itemInputs[index].end()-1);
}
  
std::string InputForm::Run()
{
  for (int i = 0;i < choices;i++)
  {
    fillField();
    form_driver(form, REQ_NEXT_FIELD);
    form_driver(form, REQ_NEXT_FIELD);
    index++;
    index %= choices;
  }
  fillField();
  wrefresh(win);
  show_panel(panel);
  int ch;  
  bool exit = false;
  while (!exit && (ch = wgetch(stdscr)) != KEY_ESC)
  {	
    switch(ch)
    {
      case KEY_LEFT:
      case KEY_RIGHT:
        switchOkCancle();
        form_driver(form, (int)'0');
        form_driver(form, REQ_DEL_PREV);  
        break;
      case KEY_UP:
      case KEY_BTAB:
        index--;
        index = (index+choices)%choices;
        form_driver(form, REQ_PREV_FIELD);
        form_driver(form, REQ_PREV_FIELD);
        fillField();
        break;
      case KEY_DOWN:
      case KEY_TAB:  
        index++;
        index %= choices;
        form_driver(form, REQ_NEXT_FIELD);
        form_driver(form, REQ_NEXT_FIELD);
        fillField();
        break;
      case KEY_BACKSPACE:
        delAchar(); 	
        break;
      case KEY_NL:
        if (index == choices-1 || !okCancle)
        {
          exit = true;
        }
        else
        {
          index++;
          index %= choices;
          form_driver(form, REQ_NEXT_FIELD);
          form_driver(form, REQ_NEXT_FIELD);
          fillField();
        }
        break;
      default:
        if (itemTypes[index].compare("password") == 0) 
        {
          itemInputs[index].push_back(ch);
          form_driver(form, '*');
        }
        else if (itemTypes[index].compare("checklist") == 0)
        {
          if (ch == ' ')
          {
            if (itemInputs[index].compare("y") == 0)
            {
              itemInputs[index] = "n"; 
              fillCheckList(false);
            }
            else
            {
              itemInputs[index] = "y";
              fillCheckList(true);
            }
          }
        }
        else
        {
          itemInputs[index].push_back(ch);
          form_driver(form, ch);
        }
        break;
    }
    update_panels();
    doupdate();
  }
  hide_panel(panel);
  if (ch == KEY_ESC || !okCancle)
    return "cancle";
  else
    return makeCommand();
}

void InputForm::fillCheckList(bool choose)
{
  form_driver(form, REQ_CLR_FIELD);
  form_driver(form, '[');
  if (choose)
  {
    form_driver(form, '*');
  }
  else
  {
    form_driver(form, ' ');
  }
  form_driver(form, ']');
  form_driver(form, REQ_PREV_CHAR);
  form_driver(form, REQ_PREV_CHAR);
}
  
