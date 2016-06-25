#pragma once

#include "Point.h"

class Object
{
public:
	static bool recalculate;//是否重算边权
	static bool keepBorder;//是否严格保持边界

	struct Vertex;
	struct Edge;
	struct Face;

	//参数矩阵
	struct ArgMatrix
	{
		double a[4][4];

		ArgMatrix& operator+=(const ArgMatrix &b)
		{
			for (int i = 0;i < 4;i++) {
				for (int j = 0;j < 4;j++) {
					a[i][j] += b.a[i][j];
				}
			}
			return *this;
		}
		ArgMatrix& operator-=(const ArgMatrix &b)
		{
			for (int i = 0;i < 4;i++) {
				for (int j = 0;j < 4;j++) {
					a[i][j] -= b.a[i][j];
				}
			}
			return *this;
		}
		void clear()
		{
			for (int i = 0;i < 4;i++) {
				for (int j = 0;j < 4;j++) {
					a[i][j] = 0;
				}
			}
		}
	};

	//顶点结构体
	struct Vertex
	{
		Point p;	//顶点位置
		int id;	//顶点编号
		
		std::list<Face*> faceList;	//面表
		std::list<Edge*> edgeList;	//边表

		ArgMatrix k;//该点参数矩阵
		Vertex *pa;	//并查集父亲

		Vertex* getroot();	//并查集寻根
		void join(Vertex *b);	//并查集合并 将自己加入b 合并面边表
		Edge* findNeighborEdge(Vertex* b) const;	//根据邻点找边
		//void uniqueEdge(); //修正指向 去除重边/无效边
	};

	//边结构体
	struct Edge
	{
		Vertex *ap, *bp;//两端顶点

		Point pos;//缩点后位置
		double cost;//二次误差代价

		Vertex* findOtherVertex(const Vertex* p) const;//已知一端找另一端
		bool update();	//通过并查集更新两端的点，无效返回false
		void makeArg();	//通过两端的点计算缩点位置
		bool ifBorderEdge() const;	//考察该边是否位于边界
	};

	//面结构体
	struct Face
	{
		ArgMatrix k;	//系数矩阵
		Vertex *ap, *bp, *cp;	//三顶点位置

		void makeArg();	//计算系数矩阵
		bool update(); //通过并查集更新顶点，无效返回false
		void increaseNeighborPointArg();	//将自己系数加到顶点上
		void decreaseNeighborPointArg();	//将顶点上自己系数扣除
	};

private:
	std::vector<std::unique_ptr<Vertex>> vertexPool;//结点池
	std::vector<std::unique_ptr<Edge>> edgePool;//边池
	std::vector<std::unique_ptr<Face>> facePool;//面池

	bool Parse(FILE * fp);//解析obj文件
public:
	bool Load(const char *filename);//读入obj文件

	void simpify(double factor);//进行简化，简化比为factor

	void Save(const char* filename);//写入obj文件
};