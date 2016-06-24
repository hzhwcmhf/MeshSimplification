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
				//vP.e = nullptr;
				vP.k.clear();
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
			edgePool.emplace_back(new Edge);
			auto& ae = *edgePool.back();
			edgePool.emplace_back(new Edge);
			auto& be = *edgePool.back();
			edgePool.emplace_back(new Edge);
			auto& ce = *edgePool.back();

			ae.next = &be, be.next = &ce, ce.next = &ae;
			ae.prev = &ce, ce.prev = &be, be.prev = &ae;
			ae.pair = be.pair = ce.pair = nullptr;
			ae.f = be.f = ce.f = &f;
			//f.e = &ae;
			ae.p = &a, be.p = &b, ce.p = &c;
			f.makeArg(a, b, c);
		}

		break;

		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), fp);
			break;
		}
	}
	
	std::cerr << "read ok!" << std::endl;
	buildStruct();
	std::cerr << "build ok!" << std::endl;
	return true;
}

void Object::buildStruct()
{
	std::vector<std::vector<Edge*>> adj(vertexPool.size());

	//构造邻接表
	for (auto &x : edgePool) {
		adj[x->p->id-1].push_back(x.get());
	}

	std::vector<std::vector<Edge*>> outNilEdge(vertexPool.size()), inNilEdge(vertexPool.size());
	//寻找边的pair
	for(int xi = 0; xi < (int)edgePool.size(); xi++){
	//for (auto &x : edgePool) {
		Edge* x = edgePool[xi].get();
		/*if (!x->p->e) {
			x->p->e = x.get();
		}*/

		if (!x->pair) {
			auto pairEdge_it = find_if(adj[x->next->p->id-1].begin(), adj[x->next->p->id-1].end(),
				[&x](Edge *y) {
				return y->next->p == x->p;
			});
			if (pairEdge_it == adj[x->next->p->id-1].end()) {
				//未找到，加入哨兵边
				edgePool.emplace_back(new Edge);
				auto &e = *edgePool.back();
				e.f = nullptr;
				e.p = x->next->p;
				e.pair = x;
				x->pair = &e;

				outNilEdge[x->next->p->id-1].push_back(&e);
				inNilEdge[x->p->id-1].push_back(&e);
			}
			else {
				//找到
				(*pairEdge_it)->pair = x;
				x->pair = (*pairEdge_it);
			}
		}
	}

	//连接哨兵边
	for (int i = 0;i < (int)vertexPool.size(); i++) {
		assert(outNilEdge[i].size() == inNilEdge[i].size());

		assert(outNilEdge[i].size() <= 1);//现假设只有一边有洞
		for (int j = 0;j < (int)outNilEdge[i].size(); j++) {
			auto &in = inNilEdge[i][j], &out = outNilEdge[i][j];
			in->next = out;
			out->prev = in;
		}
	}

	//计算每个点的参数
	for(auto &e : edgePool){
		if (e->f) {
			if (!isnan(e->p->k.a[0][0])) {
				e->p->k += e->f->k;
			}
		} else {
			e->p->k.a[0][0] = NAN;
		}
	}
	/*for (int i = 0;i < vertexPool.size(); i++) {
		vertexPool[i]->k.clear();
		Edge* s = vertexPool[i]->e;
		for (Edge* e = s; ; e = e->prev->pair) {
			if (e->f) {
				vertexPool[i]->k += e->f->k;
			} else {
				vertexPool[i]->k.a[0][0] = NAN;
				break;
			}
			if(e->prev->pair == s)
				break;
		}
	}*/
}

std::tuple<Point, double> Object::calEdgeCost(Vertex *a, Vertex *b, Edge *e)
{
	if (isnan(a->k.a[0][0]) || isnan(b->k.a[0][0])) {
		return std::make_tuple(Point{ NAN, NAN, NAN }, INFINITY);
	}
	std::vector<int> vt;
	for (Edge* s = e->prev->pair; s!=e; s = s->prev->pair) {
		vt.push_back(s->pair->p->id);
	}
	for (Edge* s = e->pair->prev->pair->prev->pair; s != e->pair->pair->next; s = s->prev->pair) {
		vt.push_back(s->pair->p->id);
	}
	sort(vt.begin(), vt.end());
	int osize = vt.end() - vt.begin();
	if (unique(vt.begin(), vt.end()) - vt.begin() != osize) {
		return std::make_tuple(Point{ NAN, NAN, NAN }, INFINITY);
	}


	ArgMatrix k = a->k;
	k += b->k;

	auto det = [](
		double a, double b, double c,
		double d, double e, double f,
		double g, double h, double i)
	{
		return a*e*i + d*h*c + b*f*g - a*f*h - b*d*i - c*e*g;
	};

	auto calCost = [](const Point &pos, const ArgMatrix &k) {
		double cost = k.a[3][3];
		for (int i = 0;i < 3;i++) {
			for (int j = 0;j < 3;j++) {
				cost += pos.p[i] * pos.p[j] * k.a[i][j];
			}
			cost += pos.p[i] * k.a[3][i] * 2;
		}
		return cost;
	};


	double D = det(
		k.a[0][0], k.a[0][1], k.a[0][2],
		k.a[1][0], k.a[1][1], k.a[1][2],
		k.a[2][0], k.a[2][1], k.a[2][2]
	);
	if (abs(D) < eps) {
		Point pos = (a->p + b->p) / 2;
		return std::make_tuple(pos, calCost(pos, k));
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
	Point pos = { x, y, z };
	return std::make_tuple(pos, calCost(pos, k));
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
	for (auto &x : edgePool) if(x->p->id < x->pair->p->id){
		std::tie(x->pos, x->cost) = calEdgeCost(x->p, x->pair->p, x.get());
		ss.insert(x.get());
	}

	while (nowFaceNum > finalFaceNum)
	{
		if (nowFaceNum % 1001 == 0) {
			std::cerr << nowFaceNum << std::endl;
		}
		//std::cerr << nowFaceNum << std::endl;
		//std::cerr << ss.size() << std::endl;
		Edge* e = *ss.begin();

		ss.erase(e);

		if (!e->pair) continue;

		assert(!isinf(e->cost));
		Vertex *pa = e->p, *pb = e->pair->p;
		Vertex *pc = e->prev->p, *pd = e->pair->prev->p;
		
		vertexPool.emplace_back(new Vertex);
		Vertex *pnew = vertexPool.back().get();
		pnew->p = e->pos;
		pnew->k.clear();
		pnew->id = (int)vertexPool.size();
		
		//pnew->k = pa->k;
		//pnew->k += pb->k;
		/*for (Edge *s = e->prev->pair; s != e; s = s->prev->pair) {
			s->p = pnew;
			s->f->makeArg(*s->p, *s->next->p, *s->prev->p);
		}

		for (Edge *s = e->pair->prev->pair; s != e; s = s->prev->pair) {
			s->p = pnew;
			s->f->makeArg(*s->p, *s->next->p, *s->prev->p);
		}*/


		

		////pc部分删堆
		//if (e->prev->p->id < e->prev->pair->p->id) {
		//	size_t tmp = ss.erase(e->prev);
		//	assert(tmp == 1);
		//}	else {
		//	size_t tmp = ss.erase(e->prev->pair);
		//	assert(tmp == 1);
		//}
		//if (e->next->p->id < e->next->pair->p->id) {
		//	size_t tmp = ss.erase(e->next);
		//	assert(tmp == 1);
		//}else {
		//	size_t tmp = ss.erase(e->next->pair);
		//	assert(tmp == 1);
		//}

		//std::cerr << pa->id << ": ";
		//调整pa邻点部分权值,pa删堆
		for (Edge *s = e->prev->pair; s != e; s = s->prev->pair) {
			//std::cerr << s->pair->p->id << " ";

			s->pair->p->k -= s->pair->f->k;
			s->pair->p->k -= s->f->k;

			//if (s->pair->p != pc && s->pair->p != pd) {
				if (s->p->id < s->pair->p->id) {
					size_t tmp = ss.erase(s);
					assert(tmp == 1);
				}
				else {
					size_t tmp = ss.erase(s->pair);
					assert(tmp == 1);
				}
			//}

			//if (s->prev->pair == e->prev->pair) break;
		}
		//std::cerr << std::endl;



		//pd部分删堆
		//if (e->pair->prev->p->id < e->pair->prev->pair->p->id) {
		//	size_t tmp = ss.erase(e->pair->prev);
		//	assert(tmp == 1);
		//}else {
		//	size_t tmp = ss.erase(e->pair->prev->pair);
		//	assert(tmp == 1);
		//}
		//if (e->pair->next->p->id < e->pair->next->pair->p->id) {
		//	size_t tmp = ss.erase(e->pair->next);
		//	assert(tmp == 1);
		//}else {
		//	size_t tmp = ss.erase(e->pair->next->pair);
		//	assert(tmp == 1);
		//}

		//调整pb邻点部分权值，删堆
		//std::cerr << pb->id << ": ";
		for (Edge *s = e->pair->prev->pair; s != e->pair; s = s->prev->pair) {
			//std::cerr << s->pair->p->id << " ";

			s->pair->p->k -= s->pair->f->k;
			s->pair->p->k -= s->f->k;

			//if (s->pair->p != pc && s->pair->p != pd) {
				if (s->p->id < s->pair->p->id) {
					size_t tmp = ss.erase(s);
					assert(tmp == 1);
				}
				else {
					size_t tmp = ss.erase(s->pair);
					assert(tmp == 1);
				}
			//}

			//if (s->prev->pair == e->prev->pair) break;
		}
		//std::cerr << std::endl;

		//重建pc pnew
		e->prev->pair->pair = e->next->pair;
		e->next->pair->pair = e->prev->pair;

		//重建pd pnew
		e->pair->next->pair->pair = e->pair->prev->pair;
		e->pair->prev->pair->pair = e->pair->next->pair;

		//调整pc\pd权值,多减加回来
		pc->k += e->f->k;
		pd->k += e->pair->f->k;

		/*for (Edge *s = e->prev->pair; ; s = s->prev->pair) {
			s->pair->p->k -= s->pair->f->k;
			s->pair->p->k -= s->pair->next->f->k;
			if (s->prev->pair == e->prev->pair) break;
		}*/

		//std::cerr << pnew->id << ": ";
		for (Edge *s = e->prev->pair; ; s = s->prev->pair) {
			//std::cerr << s->pair->p->id << " ";
			s->p = pnew;
			s->f->makeArg(*s->p, *s->next->p, *s->prev->p);
			if (s->prev->pair == e->prev->pair) break;
		}
		//std::cerr << std::endl << std::endl;

		for (Edge *s = e->prev->pair; ; s = s->prev->pair) {
			s->pair->p->k += s->pair->f->k;
			s->pair->p->k += s->f->k;
			pnew->k += s->f->k;
			if (s->prev->pair == e->prev->pair) break;
		}

		for (Edge *s = e->prev->pair; ; s = s->prev->pair) {
			for (Edge *t = s->pair; ; t = t->prev->pair) {
				if (t->p->id < t->pair->p->id) {
					if (t->pair->p != pnew) {
						size_t tmp = ss.erase(t);
						assert(tmp == 1);
					}
					std::tie(t->pos, t->cost) = calEdgeCost(t->p, t->pair->p, t);
					ss.insert(t);
				}else {
					size_t tmp = ss.erase(t->pair);
					assert(tmp == 1);
					std::tie(t->pair->pos, t->pair->cost) = calEdgeCost(t->pair->p, t->p, t);
					ss.insert(t->pair);
				}
				if(t->prev->pair == s->pair) break;
			}

			//std::tie(s->pair->pos, s->pair->cost) = calEdgeCost(s->pair->p, s->p, s->pair);
			//ss.insert(s->pair);
			if (s->prev->pair == e->prev->pair) break;
		}

		
		//清除无用边、点、面
		pa->id = pb->id = -1;
		e->f->k.a[0][0] = NAN;
		e->pair->f->k.a[0][0] = NAN;
		e->prev->pair = nullptr;
		e->next->pair = nullptr;
		e->pair->next->pair = nullptr;
		e->pair->prev->pair = nullptr;
		e->pair->pair = nullptr;
		e->pair = nullptr;

		nowFaceNum -= 2;

		//std::cerr << ss.size() << std::endl;

		//debugCheck();
		//std::cerr << "" << std::endl;
	}

	std::vector<std::unique_ptr<Vertex>> _vertexPool;
	std::vector<std::unique_ptr<Edge>> _edgePool;
	std::vector<std::unique_ptr<Face>> _facePool;
	for (auto &x : vertexPool) {
		if (x->id >= 0) {
			x->id = _vertexPool.size() + 1;
			_vertexPool.push_back(std::move(x));
		}
	}
	for (auto &x : edgePool) {
		if (x->pair != nullptr) {
			_edgePool.push_back(std::move(x));
		}
	}
	for (auto &x : facePool) {
		if (!isnan(x->k.a[0][0])) {
			_facePool.push_back(std::move(x));
		}
	}
	vertexPool = std::move(_vertexPool);
	edgePool = std::move(_edgePool);
	facePool = std::move(_facePool);
}

void Object::debugCheck()
{
	for (auto &e : edgePool) {
		if (e->f) {
			if (!isnan(e->p->k.a[0][0])) {
				e->p->k -= e->f->k;
			}
		}
		else {
			assert(isnan(e->p->k.a[0][0]));
		}
	}
	for (auto &v : vertexPool) {
		assert(isnan(v->k.a[0][0]) || abs(v->k.a[0][0]) < eps);
	}
	for (auto &e : edgePool) {
		if (e->f) {
			if (!isnan(e->p->k.a[0][0])) {
				e->p->k += e->f->k;
			}
		}
	}
}

void Object::Save(const char * filename)
{
	FILE * fp = fopen(filename, "w");

	for (auto &x : vertexPool) {
		fprintf(fp, "v %lf %lf %lf\n", x->p.x, x->p.y, x->p.z);
	}
	for (auto &x : edgePool) {
		if (x->f && x->p->id < x->next->p->id && x->p->id < x->prev->p->id) {
			fprintf(fp, "f %d %d %d\n", x->p->id, x->next->p->id, x->next->next->p->id);
		}
	}
}

void Object::Face::makeArg(const Vertex & ap, const Vertex & bp, const Vertex & cp)
{
	Point n = cross(ap.p - cp.p, bp.p - cp.p);
	n /= abs(n);

	double tmp[] = { n.x, n.y, n.z, -dot(n, ap.p) };
	for (int i = 0;i < 4;i++) {
		for (int j = 0;j < 4;j++) {
			k.a[i][j] = tmp[i] * tmp[j];
		}
	}
}
