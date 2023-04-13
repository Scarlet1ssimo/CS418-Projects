#pragma once
#include <bits/stdc++.h>
// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
  rtrim(s);
  ltrim(s);
}
static inline void remove_extra_space(std::string &s) {
  bool prev_is_space = true;
  s.erase(std::remove_if(s.begin(), s.end(),
                         [&prev_is_space](unsigned char curr) {
                           bool r = std::isspace(curr) && prev_is_space;
                           prev_is_space = std::isspace(curr);
                           return r;
                         }),
          s.end());
}
static inline std::vector<std::string> split(std::string s,
                                             std::string delimiter) {
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}