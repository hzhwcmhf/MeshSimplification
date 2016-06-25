// MeshSimplification.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Object.h"

int main(int argc, const char* argv[])
{
	std::string inobj, outobj;
	double rate = 0.01;
	std::string option;

	//解析输入参数
	//形式：MeshSimplification in.obj out.obj rate (r)(k)
	//r代表缩点后重新计算边权
	//k代表严格保持边界
	if (argc >= 4) {
		std::stringstream srate(argv[3]);
		inobj = std::string(argv[1]);
		outobj = std::string(argv[2]);
		srate >> rate;

		if (argc >= 5) {
			option = std::string(argv[4]);
			if (option.find('r')!= -1) Object::recalculate = true;
			if (option.find('k')!=-1) Object::keepBorder = true;
		}
	} else {
		inobj = "dragon.obj";
		outobj = "out.obj";
	}

	Object obj;
	int sc = clock();
	obj.Load(inobj.c_str());
	//obj.Load("data/block.obj");
	int rc = clock();
	std::cerr << "read time:" << rc - sc << std::endl;

	obj.simpify(rate);
	obj.Save(outobj.c_str());
	std::cerr << "process time:" << clock() - rc << std::endl;
    return 0;
}

