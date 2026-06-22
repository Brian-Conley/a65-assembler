#ifndef CREATEBUFFER_HPP
#define CREATEBUFFER_HPP

#include <string>

/**
 * @brief Creates a string of a file's entire content.
 *
 * @param path Path to the file.
 * @return A string containing the file's content.
 */
std::string create_file_buffer(const std::string& path);

#endif
