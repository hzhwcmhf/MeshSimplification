#pragma once

#include "Point.h"

class Object
{
public:
	struct Vertex;
	struct Edge;
	struct Face;

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
	struct Vertex
	{
		Point p;
		int id;
		//Edge* e;
		ArgMatrix k;
	};

	struct Edge
	{
		Edge *next, *prev, *pair;
		Face* f;
		Vertex *p;

		Point pos;
		double cost;
	};

	struct Face
	{
		ArgMatrix k;
		//Edge* e;

		void makeArg(const Vertex &ap, const Vertex &bp, const Vertex &cp);
	};
private:
	std::vector<std::unique_ptr<Vertex>> vertexPool;
	std::vector<std::unique_ptr<Edge>> edgePool;
	std::vector<std::unique_ptr<Face>> facePool;

	bool Parse(FILE * fp);
	void buildStruct();

	std::tuple<Point, double> calEdgeCost(Vertex *a, Vertex *b, Edge* e);
public:
	bool Load(const char *filename);

	void simpify(double factor);
	void debugCheck();

	void Save(const char* filename);
};