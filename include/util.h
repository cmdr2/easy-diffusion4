#ifndef __ED_UTIL_H__
#define __ED_UTIL_H__

#include <iostream>
#include <chrono>

std::string get_local_time();
std::string generate_uuid();
void openInDefaultBrowser(const std::string& url);
long toSecondsSinceEpochLong(const std::chrono::system_clock::time_point& tp);
double toSecondsSinceEpochDouble(const std::chrono::system_clock::time_point& tp);
unsigned char* convert_to_png_buffer(const unsigned char* image_data, int width, int height, int channels, size_t& out_size);

#endif
