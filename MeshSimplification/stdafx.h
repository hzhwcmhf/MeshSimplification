// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO:  在此处引用程序需要的其他头文件
#include <vector>
#include <algorithm>
#include <memory>
//#include <cassert>
#include <cmath>
#include <set>
#include <tuple>
#include <functional>
#include <iostream>

const double eps = 1e-10;

inline void assert(bool x)
{
	if(!x)
		throw 1;
}