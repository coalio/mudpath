#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <filesystem>
#include <regex>
#include <map>
namespace fs = std::filesystem;

void read_files_curr(
    std::vector<std::string> &files,
    const std::string &path,
    std::vector<std::string> &exclude
) {
    std::string file_extensions = ".cpp .h";

    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry.path())) {
            std::string file_name = entry.path().filename().string();
            std::string file_extension = entry.path().extension().string();
            if (
                file_extensions.find(file_extension) != std::string::npos
                && !file_extension.empty()
                && std::find(exclude.begin(), exclude.end(), file_name) == exclude.end()
            ) {
                std::string file_path = entry.path().string();
                // Read files and store their contents
                std::ifstream file(file_path);
                std::string file_contents(
                    (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()
                );
                file.close();
                files.push_back(file_path);
            }
        }
    }
}

int main() {
    // Read .sourceignore file by lines
    std::vector<std::string> exclude;
    std::ifstream sourceignore(".sourcerignore");
    std::string line;
    while (std::getline(sourceignore, line)) {
        // Remove newline
        exclude.push_back(line);
    }
    sourceignore.close();

    // Read all .cpp and .h files in the current directory, recursively
    std::vector<std::string> files;
    std::string current_dir = ".";

    // Read all files in the current directory
    read_files_curr(files, current_dir, exclude);
    // Read all files in subdirectories
    for (const auto& entry : fs::directory_iterator(current_dir)) {
        if (fs::is_directory(entry.path())) {
            read_files_curr(files, entry.path().string(), exclude);
        }
    }

    std::cout << "[sourcer] Merging " << files.size() << " files:" << std::endl;
    // Print all files
    for (const auto& file : files) {
        std::cout << file << std::endl;
    }

    // Read all files
    std::map<std::string, std::string> file_map;
    for (const auto& file : files) {
        std::ifstream file_stream(file);
        std::string file_content((std::istreambuf_iterator<char>(file_stream)),
                                 std::istreambuf_iterator<char>());
        file_map[file] = file_content;
    }

    // Look for all includes in main.cpp
    // Then replace all includes with the content of the included file
    std::string main_file_content = file_map["./main.cpp"];
    std::regex include_regex("#include \"(.*)\"");
    std::smatch include_match;

    std::string main_file_content_new = main_file_content;
    while (std::regex_search(main_file_content_new, include_match, include_regex)) {
        std::string include_file_name = "./" + include_match[1].str();
        std::string include_file_content = file_map[include_file_name];
        std::cout << "[sourcer] Replacing include " << include_file_name << std::endl;
        main_file_content_new = std::regex_replace(
            main_file_content_new,
            include_regex,
            include_file_content,
            std::regex_constants::format_first_only
        );
    }

    // Find all #includes and remove them
    std::map<std::string, bool> includes;
    std::regex include_regex_2("(#include <.*>)");
    while (std::regex_search(main_file_content_new, include_match, include_regex_2)) {
        std::string include_file_name = include_match[1].str();
        std::cout << "[sourcer] Moving " << include_file_name << std::endl;
        // Add include to includes map
        includes[include_file_name] = true;
        main_file_content_new = std::regex_replace(
            main_file_content_new,
            include_regex_2,
            "",
            std::regex_constants::format_first_only
        );
    }

    // Write includes at the top of the main contents
    for (const auto& include : includes) {
        main_file_content_new = include.first + "\n" + main_file_content_new;
    }

    // Remove multiple 3+ newlines until there is no 3+ newlines
    std::regex empty_newline_regex("\n{3,}");
    while (std::regex_search(main_file_content_new, include_match, empty_newline_regex)) {
        main_file_content_new = std::regex_replace(
            main_file_content_new,
            empty_newline_regex,
            "\n\n",
            std::regex_constants::format_first_only
        );
    }

    // Write main-copy.cpp
    std::ofstream main_copy_file("mudpath.cpp");
    main_copy_file << main_file_content_new;
    main_copy_file.close();
    std::cout << "[sourcer] Wrote mudpath.cpp" << std::endl;
}