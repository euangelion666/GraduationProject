#ifndef OPENMESH_HELPER_H
#define OPENMESH_HELPER_H
#include "opencv_helper.h"
#include <opencv2/opencv.hpp>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Decimater/CollapseInfoT.hh>
using namespace std;
typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;

class OpenmeshHelper
{
private:
	enum PointType
	{
		None, Crease, Tear, Corner, Smooth
	};
	enum Flag
	{
		Middle, Remain, Remove
	};
	struct Pair
	{
		OpenMesh::HalfedgeHandle edge;
		double value;
		friend bool operator==(const Pair &l, const Pair &r)
		{
			return l.edge.idx() == r.edge.idx();
		}
	};
	
	struct Position
	{
		double x, y, z;
		friend bool operator==(const Position &l, const Position &r)
		{
			return l.x == r.x && l.y == r.y;
		}
		friend bool operator<(const Position &l, const Position &r)
		{
			if (l.x != r.x)
			{
				return l.x < r.x;
			}
			return l.y < r.y;
		}
	};
	static bool Compare(Pair a, Pair b);
	vector<Pair> point_pair_list;
	vector<Pair> all_pair_list;
	vector<Pair> second_pair_list;
	vector<MyMesh::VertexHandle> control_list;
	vector<MyMesh::HalfedgeHandle> tear_list;
	vector<MyMesh::HalfedgeHandle> crease_list;
	map<Position, MyMesh::VertexHandle> vertex_map;
	multimap<Position, Position> tear_map;
	multimap<Position, Position> crease_map;
	MyMesh mesh;
	IplImage *image;
	MyMesh::VertexHandle **point_data;
	double degree = 110;
	double zoom = 2;
	double lambda = 0.5;
	double num_vertices;
	double num_all_vertices;
	double max_dis = 0;
	int reduce_num = 0;

	void InitPointData();
	void ReSampleColor();
	void ConnectMesh(bool isContour);
	void CountVertices();
	void InitPairList();
	void InsertCreaseEdge();
	MyMesh::VertexHandle InsertVertex(MyMesh&, MyMesh::HalfedgeHandle);
	void OptimizeColor();
	void SortVertices();
	void SubDivision();
	void LoopReduce(double rate, bool visual);
	bool IsCollapseOK(MyMesh::HalfedgeHandle half);
	void RegulatePosition(OpenMesh::Decimater::CollapseInfoT<MyMesh> info, double *x, double *y, Flag *flag);
	void CollapseEdge(MyMesh::HalfedgeHandle half);
	void OptimizePosition();
	void RebuildCreaseList(MyMesh);
	void OutputImage();
	void getColor(double, double, MyMesh::Color &);
	double getTriArea(double, double, double, double, double, double);
	bool IsBoundary(MyMesh::VertexHandle);
	void getPlane(Position, Position, Position, double&, double&, double&, double&);
	double getDistance(Position pt, double a, double b, double c, double d);
	void OutputColor(string);
public:
	void Output(MyMesh, string);
	void ReduceVertices(double rate, bool visual);

	OpenmeshHelper(string input_location);
	~OpenmeshHelper();
};

#endif // OPENMESH_HELPER_H
