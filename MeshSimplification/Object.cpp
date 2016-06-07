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
				vP.id = vertexPool.size();
				vP.e = nullptr;
				if (fscanf(fp, "%lf %lf %lf",
					&vP.x,
					&vP.y,
					&vP.z) == 3)
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

			auto &a = *vertexPool[ai], &b = *vertexPool[bi], &c = *vertexPool[ci];
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
			f.e = &ae;
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
	
	buildStruct();
	return true;
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
			fprintf(stderr, "Vertex Number = %d\n", vertexPool.size());
			fprintf(stderr, "Triangle Number = %d\n", facePool.size());
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

