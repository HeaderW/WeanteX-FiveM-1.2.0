#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <random>
#include <dwmapi.h>
#include <thread>
#include <unordered_map>
#include <XorStr/XorStr.hpp>

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4244)

namespace mjLib {
#include "String.hpp"
#include "Process.hpp"
#include "Console.hpp"
#include "Logger.hpp"
}