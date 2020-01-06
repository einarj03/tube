#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdlib>

using namespace std;

#include "tube.h"

/* You are pre-supplied with the functions below. Add your own 
   function definitions to the end of this file. */

/* internal helper function which allocates a dynamic 2D array */
char **allocate_2D_array(int rows, int columns) {
  char **m = new char *[rows];
  assert(m);
  for (int r=0; r<rows; r++) {
    m[r] = new char[columns];
    assert(m[r]);
  }
  return m;
}

/* internal helper function which deallocates a dynamic 2D array */
void deallocate_2D_array(char **m, int rows) {
  for (int r=0; r<rows; r++)
    delete [] m[r];
  delete [] m;
}

/* internal helper function which gets the dimensions of a map */
bool get_map_dimensions(const char *filename, int &height, int &width) {
  char line[512];
  
  ifstream input(filename);

  height = width = 0;

  input.getline(line,512);  
  while (input) {
    if ( (int) strlen(line) > width)
      width = strlen(line);
    height++;
    input.getline(line,512);  
  }

  if (height > 0)
    return true;
  return false;
}

/* pre-supplied function to load a tube map from a file*/
char **load_map(const char *filename, int &height, int &width) {

  bool success = get_map_dimensions(filename, height, width);
  
  if (!success)
    return NULL;

  char **m = allocate_2D_array(height, width);
  
  ifstream input(filename);

  char line[512];
  char space[] = " ";

  for (int r = 0; r<height; r++) {
    input.getline(line, 512);
    strcpy(m[r], line);
    while ( (int) strlen(m[r]) < width )
      strcat(m[r], space);
  }
  
  return m;
}

/* pre-supplied function to print the tube map */
void print_map(char **m, int height, int width) {
  cout << setw(2) << " " << " ";
  for (int c=0; c<width; c++)
    if (c && (c % 10) == 0) 
      cout << c/10;
    else
      cout << " ";
  cout << endl;

  cout << setw(2) << " " << " ";
  for (int c=0; c<width; c++)
    cout << (c % 10);
  cout << endl;

  for (int r=0; r<height; r++) {
    cout << setw(2) << r << " ";    
    for (int c=0; c<width; c++) 
      cout << m[r][c];
    cout << endl;
  }
}

/* pre-supplied helper function to report the errors encountered in Question 3 */
const char *error_description(int code) {
  switch(code) {
  case ERROR_START_STATION_INVALID: 
    return "Start station invalid";
  case ERROR_ROUTE_ENDPOINT_IS_NOT_STATION:
    return "Route endpoint is not a station";
  case ERROR_LINE_HOPPING_BETWEEN_STATIONS:
    return "Line hopping between stations not possible";
  case ERROR_BACKTRACKING_BETWEEN_STATIONS:
    return "Backtracking along line between stations not possible";
  case ERROR_INVALID_DIRECTION:
    return "Invalid direction";
  case ERROR_OFF_TRACK:
    return "Route goes off track";
  case ERROR_OUT_OF_BOUNDS:
    return "Route goes off map";
  }
  return "Unknown error";
}

/* presupplied helper function for converting string to direction enum */
Direction string_to_direction(const char *token) {
  const char *strings[] = {"N", "S", "W", "E", "NE", "NW", "SE", "SW"};
  for (int n=0; n<8; n++) {
    if (!strcmp(token, strings[n])) 
      return (Direction) n;
  }
  return INVALID_DIRECTION;
}

bool get_symbol_position(char **m, const int height, const int width, const char ch, int &r, int &c) {
  for (r = 0; r < height; ++r) {
    for (c = 0; c < width; ++c) {
      if (m[r][c] == ch)
        return true;
    }
  }

  r = -1;
  c = -1;
  return false;
}

char get_symbol_for_station_or_line(const char *object) {
  ifstream in;

  char sym;
  bool found = false;
  char list_object[MAX_LENGTH];

  in.open("lines.txt");
  while (!found && in >> sym && in.getline(list_object, MAX_LENGTH)) {
    if (strcmp(object, list_object+1) == 0)
      found = true;
  }
  in.close();

  in.open("stations.txt");
  while (!found && in >> sym && in.getline(list_object, MAX_LENGTH)) {
    if (strcmp(object, list_object+1) == 0)
      found = true;
  }
  in.close();

  if (!found)
    sym = ' ';
  
  return sym;
}

int validate_route(char **m, const int height, const int width, const char *start_station, char route[MAX_LENGTH], char destination[MAX_LENGTH]) {
  ifstream in;

  char ch;
  bool found = false;
  char list_object[MAX_LENGTH];

  in.open("stations.txt");
  while (!found && in >> ch && in.getline(list_object, MAX_LENGTH)) {
    if (strcmp(start_station, list_object+1) == 0)
      found = true;
  }
  in.close();

  if (!found)
    return ERROR_START_STATION_INVALID;

  int station_count = 0;
  int r_cur, c_cur, r_next, c_next, r_prev, c_prev;
  
  char prev, current, next;
  current = get_symbol_for_station_or_line(start_station);
  get_symbol_position(m, height, width, current, r_cur, c_cur);

  Direction directions[128];
  get_directions(directions, route);

  for (int i = 0; directions[i] != END; ++i) {
    if (!move_step(m, directions[i], r_cur, c_cur, r_next, c_next))
      return ERROR_INVALID_DIRECTION;

    if (!valid_coordinates(height, width, r_next, c_next))
      return ERROR_OUT_OF_BOUNDS;

    if (m[r_next][c_next] == ' ')
      return ERROR_OFF_TRACK;

    next = m[r_next][c_next];

    if (i > 0 && !isalnum(current) && r_next == r_prev && c_next == c_prev)
      return ERROR_BACKTRACKING_BETWEEN_STATIONS;

    if (i > 0 && current != next) {
      if (!isalnum(next) && !isalnum(current)) {
        return ERROR_LINE_HOPPING_BETWEEN_STATIONS;
      } else if (isalnum(current)){
        station_count++;
      }
    }

    r_prev = r_cur;
    c_prev = c_cur;
    r_cur = r_next;
    c_cur = c_next;
    prev = current;
    current = next;  
  }

  found = false;
  in.open("stations.txt");
  while (!found && in >> ch && in.getline(list_object, MAX_LENGTH)) {
    if (current == ch) {
      found = true;
      strcpy(destination, list_object+1);
    }
  }
  in.close();

  return station_count;
}

bool move_step(char **m, Direction dir, const int r_cur, const int c_cur, int &r_next, int &c_next) {
  r_next = r_cur;
  c_next = c_cur;
  
  switch (dir) {
    case N: --r_next; break;
    case E: ++c_next; break;
    case S: ++r_next; break;
    case W: --c_next; break;
    case NE: --r_next; ++c_next; break;
    case NW: --r_next; --c_next; break;
    case SE: ++r_next; ++c_next; break;
    case SW: ++r_next; --c_next; break;
    case INVALID_DIRECTION: return false;
  }

  return true;
}

bool valid_coordinates(const int height, const int width, const int r, const int c) {
  return r >= 0 && r < height && c >= 0 && c < width;
}

void get_directions(Direction *directions, const char *route) {
  int d_i = 0;
  for (int i = 0; route[i] != '\0'; i += 2) {
    char dir_str[2] = {route[i], '\0'};

    if (route[i+1] != ',' && route[i+1] != '\0') {
      dir_str[1] = route[i+1];
      ++i;
    }

    Direction dir = string_to_direction(dir_str);
    directions[d_i] = dir;
    ++d_i;
  }

  directions[d_i] = END;
}