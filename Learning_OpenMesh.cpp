#include <iostream>
#include <opencv2/opencv.hpp>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Decimater/CollapseInfoT.hh>
#include "opencv_helper.h"
using namespace std;
typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;

struct Pair
{
	OpenMesh::HalfedgeHandle edge;
	int value;
	friend bool operator==(const Pair &l, const Pair &r)
	{
		return l.edge == r.edge;
	}
};

struct EdgeLink
{
	Pair pair;
	EdgeLink *next;
};

struct ListLink
{
	EdgeLink *head, *last;
};

struct MyPoint
{
	MyMesh::VertexHandle point;
	CvScalar color;	
	bool isEdge;
};
void output(MyMesh mesh)
{
	try
	{
		if ( !OpenMesh::IO::write_mesh(mesh, "D:/output.off") )
		{
			std::cerr << "Cannot write mesh to file 'output.off'" << std::endl;
			return;
		}
	}
	catch( std::exception& x )
	{
		std::cerr << x.what() << std::endl;
		return;
	}
}

bool compare(Pair a, Pair b)
{
	return a.value > b.value;
}

void get_net(MyMesh &mesh, IplImage *result, MyPoint **point_data)
{
	vector<MyMesh::VertexHandle> list;

	uchar *data = (uchar*) result->imageData;	
	int step = result->widthStep / sizeof(uchar);
	
	for (int i = 0; i != result->height; ++ i)
	{
		for (int j = 0; j != result->width; ++ j)
		{
			point_data[i][j].point = mesh.add_vertex(MyMesh::Point(i, j, 0));
			if (data[i * step + j] == 255)
				point_data[i][j].isEdge = true;
			else point_data[i][j].isEdge = false;

			//if (i == 0 || j == 0 || i == result->height - 1 || j == result->width - 1)
			//	point_data[i][j].isEdge = true;
		}
	}

	for (int i = 0; i != result->height - 1; ++ i)
	{
		for (int j = 0; j != result->width - 1; ++ j)
		{
			bool point[4];
			point[0] = point_data[i][j].isEdge;
			point[1] = point_data[i + 1][j].isEdge;
			point[2] = point_data[i + 1][j + 1].isEdge;
			point[3] = point_data[i][j + 1].isEdge;
			int num = 0;
			for each(int i in point)
			{
				if (i)
					++ num;
			}

			switch (num)
			{
			case 0:
				list.clear();
				list.push_back(point_data[i][j].point);
				list.push_back(point_data[i + 1][j].point);
				list.push_back(point_data[i + 1][j + 1].point);
				list.push_back(point_data[i][j + 1].point);
				mesh.add_face(list);
				break;
			case 1:
				list.clear();
				list.push_back(point_data[i][j].point);
				list.push_back(point_data[i + 1][j].point);
				list.push_back(point_data[i + 1][j + 1].point);
				list.push_back(point_data[i][j + 1].point);
				mesh.add_face(list);
				break;
			case 2:
				if (point[0] && point[2])
				{
					list.clear();
					list.push_back(point_data[i][j].point);
					list.push_back(point_data[i + 1][j].point);
					list.push_back(point_data[i + 1][j + 1].point);
					mesh.add_face(list);	

					list.clear();
					list.push_back(point_data[i + 1][j + 1].point);
					list.push_back(point_data[i][j + 1].point);
					list.push_back(point_data[i][j].point);
					mesh.add_face(list);	
				}
				else
				{
					if (point[1] && point[3])
					{

						list.clear();
						list.push_back(point_data[i][j].point);
						list.push_back(point_data[i + 1][j].point);
						list.push_back(point_data[i][j + 1].point);
						mesh.add_face(list);	

						list.clear();
						list.push_back(point_data[i + 1][j].point);
						list.push_back(point_data[i + 1][j + 1].point);
						list.push_back(point_data[i][j + 1].point);
						mesh.add_face(list);	
					}
					else
					{
						list.clear();
						list.push_back(point_data[i][j].point);
						list.push_back(point_data[i + 1][j].point);
						list.push_back(point_data[i + 1][j + 1].point);
						list.push_back(point_data[i][j + 1].point);
						mesh.add_face(list);
					}
				}

				break;
			case 3:
				list.clear();
				list.push_back(point_data[i][j].point);
				list.push_back(point_data[i + 1][j].point);
				list.push_back(point_data[i + 1][j + 1].point);
				list.push_back(point_data[i][j + 1].point);
				mesh.add_face(list);
				break;
			case 4:
				list.clear();
				list.push_back(point_data[i][j].point);
				list.push_back(point_data[i + 1][j].point);
				list.push_back(point_data[i + 1][j + 1].point);
				list.push_back(point_data[i][j + 1].point);
				mesh.add_face(list);
				break;
			default:
				break;
			}
		}
	}
	mesh.triangulate();
}

int main()
{
	IplImage *image;
	image = cvLoadImage("D:/lena_s.jpg", -1);
	OpencvHelper helper;
	IplImage *result = cvCreateImage(cvGetSize(image), image->depth, 1);
	helper.pic_edge(image, result);

	MyMesh mesh;
	MyPoint **point_data = new MyPoint*[result->height];
	for (int i = 0; i != result->height; ++ i)
	{
		point_data[i] = new MyPoint[result->width];
	}
	for (int i = 0; i < image->height; ++ i)
	{
		for (int j = 0; j < image->width; ++ j)
		{
			point_data[i][j].color = cvGet2D(image, i, j);
		}
	}
	get_net(mesh, result, point_data);

	cout << "net_complete" << endl;
	
	//获取顶点对，加入vector
	vector<Pair> point_pair_list;
	for (auto i = mesh.halfedges_begin(); i != mesh.halfedges_end(); ++ i, ++ i)
	{
		int from_x = mesh.point(mesh.from_vertex_handle(i.handle())).data()[0];
		int from_y = mesh.point(mesh.from_vertex_handle(i.handle())).data()[1];
		int to_x = mesh.point(mesh.to_vertex_handle(i.handle())).data()[0];
		int to_y = mesh.point(mesh.to_vertex_handle(i.handle())).data()[1];
		int difference = 0;
		Pair newPair;
		newPair.edge = i.handle();
		if ((point_data[from_x][from_y].isEdge && point_data[to_x][to_y].isEdge) ||
			(!(point_data[from_x][from_y].isEdge || point_data[to_x][to_y].isEdge)))
		{
			CvScalar from_color = point_data[from_x][from_y].color;
			CvScalar to_color = point_data[to_x][to_y].color;
			for (int j = 0; j < 3; ++ j)
			{
				int temp = abs(from_color.val[j] - to_color.val[j]);
				difference += temp;
			}	
		}
		newPair.value = difference;
		point_pair_list.push_back(newPair);
		
	}
	cout << "list_complete" << endl;



	//计算顶点个数，用作停止条件，当剩余顶点数为原本的30%的时候停止
	double num_vertices = 0;
	for (auto i = mesh.vertices_begin(); i != mesh.vertices_end(); ++ i)
	{
		++ num_vertices;
	}
	cout << num_vertices << endl;
	double num_all_vertices = num_vertices;
	//统计与n顶点相连的边，记为链表
	ListLink temp_list_link;
	temp_list_link.head = nullptr;
	temp_list_link.last = nullptr;
	vector<ListLink> vertex_list(num_all_vertices, temp_list_link);
	EdgeLink *temp_link;
	for (auto i = mesh.halfedges_begin(); i != mesh.halfedges_end(); ++ i, ++ i)
	{
		int vertex_idx = mesh.to_vertex_handle(i.handle()).idx();
		if (vertex_list[vertex_idx].head == nullptr) 
		{
			vertex_list[vertex_idx].head = new EdgeLink;
			vertex_list[vertex_idx].head->pair = point_pair_list[i.handle().idx() / 2];
			vertex_list[vertex_idx].head->next = nullptr;

			vertex_list[vertex_idx].last = vertex_list[vertex_idx].head;
		}
		else
		{
			temp_link = new EdgeLink;
			temp_link->pair = point_pair_list[i.handle().idx() / 2];
			temp_link->next = nullptr;
			vertex_list[vertex_idx].last->next = temp_link;
			vertex_list[vertex_idx].last = vertex_list[vertex_idx].last->next;
		}
		


		vertex_idx = mesh.from_vertex_handle(i.handle()).idx();
		if (vertex_list[vertex_idx].head == nullptr) 
		{
			vertex_list[vertex_idx].head = new EdgeLink;
			vertex_list[vertex_idx].head->pair = point_pair_list[i.handle().idx() / 2];
			vertex_list[vertex_idx].head->next = nullptr;

			vertex_list[vertex_idx].last = vertex_list[vertex_idx].head;
		}
		else
		{
			temp_link = new EdgeLink;
			temp_link->pair = point_pair_list[i.handle().idx() / 2];
			temp_link->next = nullptr;
			vertex_list[vertex_idx].last->next = temp_link;
			vertex_list[vertex_idx].last = vertex_list[vertex_idx].last->next;
		}

	}
	cout << "link_complete" << endl;
	vector<Pair> temp_list;
	for (auto i = point_pair_list.begin(); i != point_pair_list.end(); ++i)
	{
		int from_x = mesh.point(mesh.from_vertex_handle(i->edge)).data()[0];
		int from_y = mesh.point(mesh.from_vertex_handle(i->edge)).data()[1];
		int to_x = mesh.point(mesh.to_vertex_handle(i->edge)).data()[0];
		int to_y = mesh.point(mesh.to_vertex_handle(i->edge)).data()[1];
		if ((point_data[from_x][from_y].isEdge && point_data[to_x][to_y].isEdge) ||
			(!(point_data[from_x][from_y].isEdge || point_data[to_x][to_y].isEdge)))
		{
			temp_list.push_back(*i);
		}
	}
	point_pair_list = temp_list;
	//建造最小堆
	make_heap(point_pair_list.begin(), point_pair_list.end(), compare);
	mesh.request_edge_status();
	mesh.request_vertex_status();
	mesh.request_halfedge_status();
	mesh.request_face_status();
	mesh.request_face_normals();
	mesh.request_vertex_normals();
	while (num_vertices / num_all_vertices > 0.25)
	//while (point_pair_list.size() > 0)
	{
		pop_heap(point_pair_list.begin(), point_pair_list.end(), compare);
		Pair temp = point_pair_list[point_pair_list.size() - 1];
		point_pair_list.pop_back();
		auto half = temp.edge;
		
		if (mesh.is_collapse_ok(half))
		{
			auto info = OpenMesh::Decimater::CollapseInfoT<MyMesh>::CollapseInfoT(mesh, half);
			EdgeLink *remove_head = vertex_list[info.v0.idx()].head;
			EdgeLink *remain_head = vertex_list[info.v1.idx()].head;
			
			mesh.collapse(half);
			-- num_vertices;
			//调整合并后点的位置
			int remove_x = info.p0.data()[0];
			int remove_y = info.p0.data()[1];
			int remain_x = info.p1.data()[0];
			int remain_y = info.p1.data()[1];
			CvScalar remove_color = point_data[remove_x][remove_y].color;
			CvScalar remain_color = point_data[remain_x][remain_y].color;
			//cout << remove_x << ' ' << remove_y << endl;
			//cout << remain_x << ' ' << remain_y << endl;
			if ((remove_x == 0 && remove_y == 0) ||
				(remove_x == 0 && remove_y == image->width - 1) ||
				(remove_x == image->height - 1 && remove_y == 0) ||
				(remove_x == image->height - 1 && remove_y == image->width - 1))
			{
				mesh.point(info.v1).data()[0] = remove_x;
				mesh.point(info.v1).data()[1] = remove_y;
				continue;
			}

			if ((remain_x == 0 && remain_y == 0) ||
				(remain_x == 0 && remain_y == image->width - 1) ||
				(remain_x == image->height - 1 && remain_y == 0) ||
				(remain_x == image->height - 1 && remain_y == image->width - 1))
			{
				continue;
			}
			
			if (remove_x == 0 || remove_y == 0 || remove_x == image->height - 1 || remove_y == image->width - 1)
			{
				if (remain_x == 0 || remain_y == 0 || remain_x == image->height - 1 || remain_y == image->width - 1)
				{
					mesh.point(info.v1).data()[0] = (int) ((info.p1.data()[0] + info.p0.data()[0]) / 2);
					mesh.point(info.v1).data()[1] = (int) ((info.p1.data()[1] + info.p0.data()[1]) / 2);
				}
				else 
				{
					mesh.point(info.v1).data()[0] = remove_x;
					mesh.point(info.v1).data()[1] = remove_y;
				}
				
			}
			else
			{
				
				if (!(remain_x == 0 || remain_y == 0 || remain_x == image->height - 1 || remain_y == image->width - 1))
				{
					mesh.point(info.v1).data()[0] = (int) ((info.p1.data()[0] + info.p0.data()[0]) / 2);
					mesh.point(info.v1).data()[1] = (int) ((info.p1.data()[1] + info.p0.data()[1]) / 2);
				}
			}
			//调整点的颜色
			int x = mesh.point(info.v1).data()[0];
			int y = mesh.point(info.v1).data()[1];
			for (int k = 0; k < 3; ++ k)
			{
				point_data[x][y].color.val[k] = (remove_color.val[k] + remain_color.val[k]) / 2;
			}

			//删除相邻的边
			do 
			{
				Pair temp_pair = remove_head->pair;
				remove_head = remove_head->next;
				remove(point_pair_list.begin(), point_pair_list.end(), temp_pair);
			/*	int from_x = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[0];
				int from_y = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[1]; 
				int to_x = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[0];
				int to_y = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[1];
				if (mesh.from_vertex_handle(temp_pair.edge).idx() != mesh.to_vertex_handle(temp_pair.edge).idx())
				{
					int difference = 0;
					CvScalar from_color = point_data[from_x][from_y].color;
					CvScalar to_color = point_data[to_x][to_y].color;
					for (int j = 0; j < 3; ++ j)
					{
						int temp = abs(from_color.val[j] - to_color.val[j]);
						difference += temp;
					}	
					temp_pair.value = difference;
				}*/
			} while (remove_head->next);
			
			do 
			{
				Pair temp_pair = remain_head->pair;
				remain_head = remain_head->next;
				remove(point_pair_list.begin(), point_pair_list.end(), temp_pair);
		/*		int from_x = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[0];
				int from_y = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[1]; 
				int to_x = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[0];
				int to_y = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[1];
				if (mesh.from_vertex_handle(temp_pair.edge).idx() != mesh.to_vertex_handle(temp_pair.edge).idx())
				{
					int difference = 0;
					CvScalar from_color = point_data[from_x][from_y].color;
					CvScalar to_color = point_data[to_x][to_y].color;
					for (int j = 0; j < 3; ++ j)
					{
						int temp = abs(from_color.val[j] - to_color.val[j]);
						difference += temp;
					}	
					temp_pair.value = difference;
				}*/
			} while (remain_head->next);
			//重新计算相连的边的代价，并加入vector
			for (auto it = mesh.voh_iter(info.v1); it; ++it)
			{
				Pair temp_pair;
				temp_pair.edge = it.handle();
				int from_x = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[0];
				int from_y = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[1];
				int to_x = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[0];
				int to_y = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[1];
				int difference = 0;
				CvScalar from_color = point_data[from_x][from_y].color;
				CvScalar to_color = point_data[to_x][to_y].color;
				for (int j = 0; j < 3; ++j)
				{
					int temp = abs(from_color.val[j] - to_color.val[j]);
					difference += temp;
				}
				temp_pair.value = difference;
				point_pair_list.push_back(temp_pair);
			}
			make_heap(point_pair_list.begin(), point_pair_list.end(), compare);
			//cout << mesh.point(info.v1).data()[0] << ' ' << mesh.point(info.v1).data()[1] << endl;
		}
		
		else
		{

			half = mesh.opposite_halfedge_handle(half);
			if (mesh.is_collapse_ok(half))
			{
				cout << "opposite" << endl;
				auto info = OpenMesh::Decimater::CollapseInfoT<MyMesh>::CollapseInfoT(mesh, half);
				EdgeLink *remove_head = vertex_list[info.v0.idx()].head;
				EdgeLink *remain_head = vertex_list[info.v1.idx()].head;

				mesh.collapse(half);
				--num_vertices;
				//调整合并后点的位置
				int remove_x = info.p0.data()[0];
				int remove_y = info.p0.data()[1];
				int remain_x = info.p1.data()[0];
				int remain_y = info.p1.data()[1];
				CvScalar remove_color = point_data[remove_x][remove_y].color;
				CvScalar remain_color = point_data[remain_x][remain_y].color;
				//cout << remove_x << ' ' << remove_y << endl;
				//cout << remain_x << ' ' << remain_y << endl;
				if ((remove_x == 0 && remove_y == 0) ||
					(remove_x == 0 && remove_y == image->width - 1) ||
					(remove_x == image->height - 1 && remove_y == 0) ||
					(remove_x == image->height - 1 && remove_y == image->width - 1))
				{
					mesh.point(info.v1).data()[0] = remove_x;
					mesh.point(info.v1).data()[1] = remove_y;
					continue;
				}

				if ((remain_x == 0 && remain_y == 0) ||
					(remain_x == 0 && remain_y == image->width - 1) ||
					(remain_x == image->height - 1 && remain_y == 0) ||
					(remain_x == image->height - 1 && remain_y == image->width - 1))
				{
					continue;
				}

				if (remove_x == 0 || remove_y == 0 || remove_x == image->height - 1 || remove_y == image->width - 1)
				{
					if (remain_x == 0 || remain_y == 0 || remain_x == image->height - 1 || remain_y == image->width - 1)
					{
						mesh.point(info.v1).data()[0] = (int)((info.p1.data()[0] + info.p0.data()[0]) / 2);
						mesh.point(info.v1).data()[1] = (int)((info.p1.data()[1] + info.p0.data()[1]) / 2);
					}
					else
					{
						mesh.point(info.v1).data()[0] = remove_x;
						mesh.point(info.v1).data()[1] = remove_y;
					}

				}
				else
				{

					if (!(remain_x == 0 || remain_y == 0 || remain_x == image->height - 1 || remain_y == image->width - 1))
					{
						mesh.point(info.v1).data()[0] = (int)((info.p1.data()[0] + info.p0.data()[0]) / 2);
						mesh.point(info.v1).data()[1] = (int)((info.p1.data()[1] + info.p0.data()[1]) / 2);
					}
				}
				//调整点的颜色
				int x = mesh.point(info.v1).data()[0];
				int y = mesh.point(info.v1).data()[1];
				for (int k = 0; k < 3; ++k)
				{
					point_data[x][y].color.val[k] = (remove_color.val[k] + remain_color.val[k]) / 2;
				}

				//重新计算代价
				do
				{
					Pair temp_pair = remove_head->pair;
					remove_head = remove_head->next;
					remove(point_pair_list.begin(), point_pair_list.end(), temp_pair);
					/*	int from_x = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[0];
					int from_y = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[1];
					int to_x = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[0];
					int to_y = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[1];
					if (mesh.from_vertex_handle(temp_pair.edge).idx() != mesh.to_vertex_handle(temp_pair.edge).idx())
					{
					int difference = 0;
					CvScalar from_color = point_data[from_x][from_y].color;
					CvScalar to_color = point_data[to_x][to_y].color;
					for (int j = 0; j < 3; ++ j)
					{
					int temp = abs(from_color.val[j] - to_color.val[j]);
					difference += temp;
					}
					temp_pair.value = difference;
					}*/
				} while (remove_head->next);

				do
				{
					Pair temp_pair = remain_head->pair;
					remain_head = remain_head->next;
					remove(point_pair_list.begin(), point_pair_list.end(), temp_pair);
					/*		int from_x = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[0];
					int from_y = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[1];
					int to_x = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[0];
					int to_y = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[1];
					if (mesh.from_vertex_handle(temp_pair.edge).idx() != mesh.to_vertex_handle(temp_pair.edge).idx())
					{
					int difference = 0;
					CvScalar from_color = point_data[from_x][from_y].color;
					CvScalar to_color = point_data[to_x][to_y].color;
					for (int j = 0; j < 3; ++ j)
					{
					int temp = abs(from_color.val[j] - to_color.val[j]);
					difference += temp;
					}
					temp_pair.value = difference;
					}*/
				} while (remain_head->next);
				for (auto it = mesh.voh_iter(info.v1); it; ++it)
				{
					Pair temp_pair;
					temp_pair.edge = it.handle();
					int from_x = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[0];
					int from_y = mesh.point(mesh.from_vertex_handle(temp_pair.edge)).data()[1];
					int to_x = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[0];
					int to_y = mesh.point(mesh.to_vertex_handle(temp_pair.edge)).data()[1];
					int difference = 0;
					CvScalar from_color = point_data[from_x][from_y].color;
					CvScalar to_color = point_data[to_x][to_y].color;
					for (int j = 0; j < 3; ++j)
					{
						int temp = abs(from_color.val[j] - to_color.val[j]);
						difference += temp;
					}
					temp_pair.value = difference;
					point_pair_list.push_back(temp_pair);
				}
				make_heap(point_pair_list.begin(), point_pair_list.end(), compare);
			}
		}
		//cout << num_vertices / num_all_vertices << endl;
	}
	
	cout << "size" << point_pair_list.size() << endl;
	cout << num_vertices << endl;
	mesh.garbage_collection();

	mesh.update_vertex_normals();
	mesh.update_face_normals();

	mesh.release_face_normals();
	mesh.release_vertex_normals();
	mesh.release_edge_status();
	mesh.release_vertex_status();
	mesh.release_halfedge_status();
	mesh.release_face_status();
	int num_face = 0;
	for (auto i = mesh.faces_begin(); i != mesh.faces_end(); ++ i)
		++ num_face;
	cout << "faces:" << num_face << endl;
	num_vertices = 0;
	for (auto i = mesh.vertices_begin(); i != mesh.vertices_end(); ++ i)
	{
		++ num_vertices;
	}
	cout << num_vertices << endl;
	
	output(mesh);
	cvReleaseImage(&image);
	cvReleaseImage(&result);
	system("pause");
	return 0;
}