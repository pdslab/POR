// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "common.h"
#include <cstdlib>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <Shlwapi.h>
#include <sys/stat.h>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <experimental/filesystem>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <iterator>
//compiler hints 
#pragma comment (lib, "Shlwapi.lib")

//namespaces 
using namespace std;
using namespace cv;
namespace fs = experimental::filesystem;
// TODO: reference additional headers your program requires heri
