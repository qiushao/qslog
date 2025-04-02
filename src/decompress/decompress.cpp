#include "LogDecompressor.h"
#include <iostream>
#include <string>

void printUsage(const char *programName) {
    std::cout << "Usage: " << programName << " <input_file> <output_file>" << std::endl;
    std::cout << "  input_file:  Path to the compressed log file" << std::endl;
    std::cout << "  output_file: Path to save the decompressed log file" << std::endl;
}

int main(int argc, char *argv[]) {
    // 检查参数数量
    if (argc != 3) {
        std::cerr << "Error: Invalid number of arguments." << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    std::cout << "Decompressing log file..." << std::endl;
    std::cout << "Input file: " << inputFile << std::endl;
    std::cout << "Output file: " << outputFile << std::endl;

    // 创建并使用LogDecompressor
    qslog::LogDecompressor decompressor(inputFile, outputFile);

    bool success = decompressor.decompress();

    if (success) {
        std::cout << "Decompression completed successfully." << std::endl;
        return 0;
    } else {
        std::cerr << "Error: Failed to decompress log file." << std::endl;
        return 1;
    }
}
