#include "global.h"
#include "confirmBox.h"

ConfirmBox::ConfirmBox():okCancle(false)
{
  
}

bool ConfirmBox::Init()
{
  win = newwin(lines, cols, LINES/2-lines/2, COLS/2-cols/2);
  keypad(win, TRUE);
  box(win, 0, 0);
  panel = new_panel(win);
  switchOkCancle();
  update_panels();
  doupdate();
  hide_panel(panel);
  return true;
}

void ConfirmBox::switchOkCancle()
{
  okCancle = !okCancle;
  if (okCancle)
  {
    wattron(win, COLOR_PAIR(2)|A_STANDOUT);
    mvwprintw(win, lines-2, cols-10, "%s", "  OK  ");
    wattroff(win, COLOR_PAIR(2)|A_STANDOUT);
    mvwprintw(win, lines-2, cols-25, "%s", "  CANCLE  ");
  }
  else
  {
    wattron(win, COLOR_PAIR(2)|A_STANDOUT);
    mvwprintw(win, lines-2, cols-25, "%s", "  CANCLE  ");
    wattroff(win, COLOR_PAIR(2)|A_STANDOUT);
    mvwprintw(win, lines-2, cols-10, "%s", "  OK  ");
  }
}

bool ConfirmBox::Run(std::string &message)
{
  message += "\nAre you sure you want to continue?";
  int x = 0;
  int y = 0;
  mvwaddch(win, 1, 0, ' ');
  for (unsigned int i = 0; i < message.size(); i++)
  {
    getyx(win, y, x);
    if (x >= cols-1)
    {
      waddch(win, ' ');
      waddch(win, ' ');
    }
    if (x == 0)
    {
      waddch(win, ' ');
    }
    if (y < lines-2)
    {
      waddch(win, message[i]);
    }
  }
  okCancle = false;
  switchOkCancle();
  box(win, 0, 0);
  wattron(win, COLOR_PAIR(1));
  mvwprintw(win, 0, cols/2-6, "%s", "Warning");
  wattroff(win, COLOR_PAIR(1));
  wrefresh(win);
  show_panel(panel);
  int ch;  
  while ((ch = wgetch(stdscr)) != KEY_NL && ch != KEY_ESC)
  {	
    switch(ch)
    {
      case KEY_LEFT:
      case KEY_RIGHT:
        switchOkCancle();
        break;
    }
    update_panels();
    doupdate();
  }
  hide_panel(panel);
  return (ch != KEY_ESC) && okCancle;
}
