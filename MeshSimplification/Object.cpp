#include "stdafx.h"
#include "Object.h"

#pragma warning(disable:4996)
bool Object::Parse(FILE * fp)
{
	char buf[256];

	int nVertices = 0;
	int nTriangles = 0;
	int lineNumber = 0;

	while (fscanf(fp, "%s", buf) != EOF)
	{
		lineNumber++;
		switch (buf[0])
		{
		case '#':				/* comment */
								/* eat up rest of line */
			fgets(buf, sizeof(buf), fp);
			break;
		case 'v':				/* v, vn, vt */
			switch (buf[1])
			{
			case '\0':			    /* vertex */
			{
				vertexPool.emplace_back(new Vertex);
				auto& vP = *vertexPool.back();
				vP.id = (int)vertexPool.size();
				vP.k.clear();
				vP.pa = nullptr;
				if (fscanf(fp, "%lf %lf %lf",
					&vP.p.x,
					&vP.p.y,
					&vP.p.z) == 3)
				{
					nVertices++;
				}
				else
				{
					fprintf(stderr, "Error: Wrong Number of Values(Should be 3). at Line %d\n", lineNumber);
					return false;
				}
			}
			break;
			}
			break;
		case 'f':				/* face */
		{
			int ai, bi, ci;
			if (fscanf(fp, "%s", buf) != 1)
			{
				fprintf(stderr, "Error: Wrong Face at Line %d\n", lineNumber);
				return false;
			}

			int n, t;
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			if (strstr(buf, "//"))
			{
				/* v//n */
				if (sscanf(buf, "%d//%d", &ai, &n) == 2 &&
					fscanf(fp, "%d//%d", &bi, &n) == 2 &&
					fscanf(fp, "%d//%d", &ci, &n) == 2)
				{
					nTriangles++;
				}
				else
				{
					fprintf(stderr, "Error: Wrong Face at Line %d\n", lineNumber);
					return false;
				}

			}
			else if (sscanf(buf, "%d/%d/%d", &ai, &t, &n) == 3)
			{
				/* v/t/n */
				if (fscanf(fp, "%d/%d/%d", &bi, &t, &n) == 3 &&
					fscanf(fp, "%d/%d/%d", &ci, &t, &n) == 3)
				{
					nTriangles++;
				}
				else
				{
					printf("Error: Wrong Face at Line %d\n", lineNumber);
					return false;
				}
			}
			else if (sscanf(buf, "%d/%d", &ai, &t) == 2)
			{
				/* v/t */
				if (fscanf(fp, "%d/%d", &bi, &t) == 2 &&
					fscanf(fp, "%d/%d", &ci, &t) == 2)
				{
					nTriangles++;
				}
				else
				{
					printf("Error: Wrong Face at Line %d\n", lineNumber);
					return false;
				}
			}
			else
			{
				/* v */
				if (sscanf(buf, "%d", &ai) == 1 &&
					fscanf(fp, "%d", &bi) == 1 &&
					fscanf(fp, "%d", &ci) == 1)
				{
					nTriangles++;
				}
				else
				{
					printf("Error: Wrong Face at Line %d\n", lineNumber);
					return false;
				}
			}

			auto &a = *vertexPool[ai-1], &b = *vertexPool[bi-1], &c = *vertexPool[ci-1];
			facePool.emplace_back(new Face);
			auto& f = *facePool.back();

			Edge *ae, *be, *ce;
			ae = a.findNeighborEdge(&b);
			be = b.findNeighborEdge(&c);
			ce = c.findNeighborEdge(&a);
			if (!ae) {
				edgePool.emplace_back(new Edge);
				ae = edgePool.back().get();
				a.edgeList.push_back(ae);
				b.edgeList.push_back(ae);
			}
			if (!be) {
				edgePool.emplace_back(new Edge);
				be = edgePool.back().get();
				b.edgeList.push_back(be);
				c.edgeList.push_back(be);
			}
			if (!ce) {
				edgePool.emplace_back(new Edge);
				ce = edgePool.back().get();
				c.edgeList.push_back(ce);
				a.edgeList.push_back(ce);
			}

			f.ap = &a, f.bp = &b, f.cp = &c;
			ae->ap = &a, be->ap = &b, ce->ap = &c;
			ae->bp = &b, be->bp = &c, ce->bp = &a;
			a.faceList.push_back(&f);
			c.faceList.push_back(&f);
			b.faceList.push_back(&f);
			f.makeArg();
			f.increaseNeighborPointArg();
		}

		break;

		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), fp);
			break;
		}
	}
	
	std::cerr << "read ok!" << std::endl;
	return true;
}

void Object::Edge::makeArg()
{
	//如果两个点其中之一有边界点
	/*if (isnan(a->k.a[0][0]) || isnan(b->k.a[0][0])) {
		return std::make_tuple(Point{ NAN, NAN, NAN }, INFINITY);
	}*/

	//计算部分

	//两个点的代价矩阵相加
	ArgMatrix k = ap->k;
	k += bp->k;

	//行列式计算
	auto det = [](
		double a, double b, double c,
		double d, double e, double f,
		double g, double h, double i)
	{
		return a*e*i + d*h*c + b*f*g - a*f*h - b*d*i - c*e*g;
	};

	//计算矩阵乘法 pos * k * posT
	auto calCost = [this](const Point &pos, const ArgMatrix &k) {
		double cost = k.a[3][3];
		for (int i = 0;i < 3;i++) {
			for (int j = 0;j < 3;j++) {
				cost += pos.p[i] * pos.p[j] * k.a[i][j];
			}
			cost += pos.p[i] * k.a[3][i] * 2;
		}
		return cost;
	};

	//克莱姆解方程
	double D = det(
		k.a[0][0], k.a[0][1], k.a[0][2],
		k.a[1][0], k.a[1][1], k.a[1][2],
		k.a[2][0], k.a[2][1], k.a[2][2]
	);
	if (abs(D) < eps) {
		pos = (ap->p + bp->p) / 2;
		cost = calCost(pos, k);
		if (this->ifBorderEdge()) cost += norm(ap->p - pos) + norm(bp->p - pos);
		return;
	}

	double x = det(
		-k.a[0][3], k.a[0][1], k.a[0][2],
		-k.a[1][3], k.a[1][1], k.a[1][2],
		-k.a[2][3], k.a[2][1], k.a[2][2]
	) / D;
	double y = det(
		k.a[0][0], -k.a[0][3], k.a[0][2],
		k.a[1][0], -k.a[1][3], k.a[1][2],
		k.a[2][0], -k.a[2][3], k.a[2][2]
	) / D;
	double z = det(
		k.a[0][0], k.a[0][1], -k.a[0][3],
		k.a[1][0], k.a[1][1], -k.a[1][3],
		k.a[2][0], k.a[2][1], -k.a[2][3]
	) / D;
	pos = { x, y, z };
	cost = calCost(pos, k);
	if (this->ifBorderEdge()) cost += norm(ap->p - pos) + norm(bp->p - pos);
}

bool Object::Edge::ifBorderEdge() const
{
	//wrong
	return ap->edgeList.size() != ap->faceList.size() || bp->edgeList.size() != bp->faceList.size();
}

bool Object::Load(const char *filename)
{
	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Error: Loading %s failed.\n", filename);
		return false;
	}
	else
	{
		if (Parse(fp))
		{
			fprintf(stderr, "Loading from %s successfully.\n", filename);
			fprintf(stderr, "Vertex Number = %d\n", (int)vertexPool.size());
			fprintf(stderr, "Triangle Number = %d\n", (int)facePool.size());
			fclose(fp);
			return true;
		}
		else
		{
			fclose(fp);
			return false;
		}
	}
}

void Object::simpify(double factor)
{
	int nowFaceNum = facePool.size();
	int finalFaceNum = (int)(nowFaceNum * factor);

	//finalFaceNum = nowFaceNum - 10;

	//堆比较函数
	auto edgeComp = [](const Edge* a, const Edge *b)
	{
		assert(!isnan(a->cost) && !isnan(b->cost));
		if (a->cost == b->cost) {
			return a < b;
		} else {
			return a->cost < b->cost;
		}
	};
	std::set<Edge*, std::function<bool(const Edge*, const Edge*)>> ss(edgeComp);

	//建堆
	for (auto &x : edgePool){
		//if (x->ifBorderEdge()) continue; //禁止删边界边
		//std::tie(x->pos, x->cost) = calEdgeCost(x->ap, x->bp);
		x->makeArg();
		ss.insert(x.get());
	}

	while (nowFaceNum > finalFaceNum)
	{
		if (nowFaceNum % 1001 == 0) {
			std::cerr << nowFaceNum << std::endl;
		}
		//std::cerr << nowFaceNum << std::endl;
		//std::cerr << ss.size() << std::endl;

		if (nowFaceNum == 8)
			std::cerr << "";

		Edge* e = *ss.begin();
		ss.erase(e);

		assert(!isinf(e->cost) && !isnan(e->cost));

		auto* pa = e->ap;
		auto* pb = e->bp;
		if(pa != pa->getroot() || pb != pb->getroot() || pa == pb) continue;
		//if (e->ifBorderEdge()) continue;

		//std::cerr << e->cost << std::endl;


		/*//去除合并后重边
		for (auto &x : pa->edgeList) {
			auto uselessEdge = pb->findNeighborEdge(x->findOtherVertex(pa));
			if (uselessEdge) {
				pb->edgeList.remove(uselessEdge);

				//if (!uselessEdge->ifBorderEdge()) {
				//	ASSERT_DELETE(ss.erase(uselessEdge));
				//}
			}
		}
		pb->edgeList.remove(pb->findNeighborEdge(pa));*/

		pa->k += pb->k;

		//合并
		pb->join(pa);
		pa->p = e->pos;

		//去重边\无效边
		//注意所有update过的边都需要删堆
		for (auto &x = pa->edgeList.begin(); x != pa->edgeList.end(); ) {
			ss.erase(*x);
			if (!(*x)->update()) { //无效边
				x = pa->edgeList.erase(x);
				continue;
			}
			x++;
		}
		pa->edgeList.sort([&pa](const Edge* a, const Edge* b) {
			return a->findOtherVertex(pa) < b->findOtherVertex(pa);
		});
		pa->edgeList.unique([&pa](const Edge* a, const Edge* b) {
			return a->findOtherVertex(pa) == b->findOtherVertex(pa);
		});
		
		//还原权值,记录涉及到的边
		std::vector<Edge*> editEdgeList;
		pa->faceList.sort();
		pa->faceList.unique();
		for (auto &nFace = pa->faceList.begin(); nFace != pa->faceList.end(); nFace++) {
			if (!(*nFace)->ap) continue;
			(*nFace)->update();
			//(*nFace)->decreaseNeighborPointArg(); //让周围点减去之前的arg

			editEdgeList.insert(editEdgeList.end(), (*nFace)->ap->edgeList.begin(), (*nFace)->ap->edgeList.end());
			editEdgeList.insert(editEdgeList.end(), (*nFace)->bp->edgeList.begin(), (*nFace)->bp->edgeList.end());
			editEdgeList.insert(editEdgeList.end(), (*nFace)->cp->edgeList.begin(), (*nFace)->cp->edgeList.end());
		}

		//访问生成的点周围的面，去除无效面
		//更新面的权值
		//再更新面相邻点权值
		for (auto &nFace = pa->faceList.begin(); nFace != pa->faceList.end(); ) {
			if (!(*nFace)->ap || !(*nFace)->update()) { //无效面
				if ((*nFace)->ap) {
					(*nFace)->ap = nullptr;
					nowFaceNum--;
				}
				nFace = pa->faceList.erase(nFace);
				continue;
			}

			//(*nFace)->makeArg(); //更新权值
			//(*nFace)->increaseNeighborPointArg(); //让周围点加上现在的arg

			nFace++;
		}

		//再更新点相邻边的权值
		//为了减少对堆的操作量，对修改进行去重处理
		sort(editEdgeList.begin(), editEdgeList.end());
		editEdgeList.resize((unique(editEdgeList.begin(), editEdgeList.end()) - editEdgeList.begin()));
		for (auto &x : editEdgeList) if(x->update()){
			ss.erase(x);
			x->makeArg();
			ss.insert(x);
		}

	}

	//清除、重新编号并调整结构
	std::vector<std::unique_ptr<Vertex>> _vertexPool;
	std::vector<std::unique_ptr<Edge>> _edgePool;
	std::vector<std::unique_ptr<Face>> _facePool;
	for (auto &x : vertexPool) {
		if (x->getroot() == x.get()) {
			x->id = _vertexPool.size() + 1;
			_vertexPool.push_back(std::move(x));
		}
	}
	for (auto &x : edgePool) {
		if (x->update()) {
			_edgePool.push_back(std::move(x));
		}
	}
	for (auto &x : facePool) {
		if (x->ap && x->update()) {
			_facePool.push_back(std::move(x));
		}
	}
	vertexPool = std::move(_vertexPool);
	edgePool = std::move(_edgePool);
	facePool = std::move(_facePool);
}


void Object::Save(const char * filename)
{
	FILE * fp = fopen(filename, "w");

	for (auto &x : vertexPool) {
		fprintf(fp, "v %lf %lf %lf\n", x->p.x, x->p.y, x->p.z);
	}
	for (auto &x : facePool) {
		fprintf(fp, "f %d %d %d\n", x->ap->id, x->bp->id, x->cp->id);
	}
}

void Object::Face::makeArg()
{
	Point n = cross(ap->p - cp->p, bp->p - cp->p);
	n /= abs(n);

	double tmp[] = { n.x, n.y, n.z, -dot(n, ap->p) };
	for (int i = 0;i < 4;i++) {
		for (int j = 0;j < 4;j++) {
			k.a[i][j] = tmp[i] * tmp[j];
		}
	}
}

bool Object::Face::update()
{
	ap = ap->getroot();
	bp = bp->getroot();
	cp = cp->getroot();
	return !(ap == bp || bp == cp || ap == cp);
}

void Object::Face::increaseNeighborPointArg()
{
	ap->k += k;
	bp->k += k;
	cp->k += k;
}

void Object::Face::decreaseNeighborPointArg()
{
	ap->k -= k;
	bp->k -= k;
	cp->k -= k;
}

Object::Vertex* Object::Vertex::getroot()
{
	if (!pa) return this;
	return pa = pa->getroot();
}

void Object::Vertex::join(Vertex * b)
{
	pa = b;
	b->faceList.insert(b->faceList.end(), faceList.begin(), faceList.end());
	faceList.clear();
	b->edgeList.insert(b->edgeList.end(), edgeList.begin(), edgeList.end());
	edgeList.clear();
}

Object::Edge * Object::Vertex::findNeighborEdge(Vertex * b) const
{
	for (auto &x : edgeList) {
		if (x->findOtherVertex(this) == b) return x;
	}
	return nullptr;
}
	

Object::Vertex* Object::Edge::findOtherVertex(const Vertex * p) const
{
	assert(ap == p || bp == p);
	return (Vertex*)((int)ap ^ (int)bp ^ (int)p);
}

bool Object::Edge::update()
{
	ap = ap->getroot();
	bp = bp->getroot();
	return ap != bp;
}
