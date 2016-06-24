// MeshSimplification.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Object.h"

int main()
{
	Object obj;
	obj.Load("data/fixed.perfect.dragon.100K.0.07.obj");
	obj.simpify(0.01);
	obj.Save("output.obj");
    return 0;
}

