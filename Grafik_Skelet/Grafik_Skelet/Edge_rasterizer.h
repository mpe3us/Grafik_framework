#pragma once
class Edge_rasterizer
{
public:
	Edge_rasterizer();
	virtual ~Edge_rasterizer();

	void init(int x1, int y1, int x2, int y2);
	void init(int x1, int y1, int x2, int y2, int x3, int y3);

	bool more_fragments() const;
	void next_fragment();

	int x() const;
	int y() const;
private:
	int x1; int y1;
	int x2; int y2;
	int x3; int y3;

	int x_start; int y_start;
	int x_stop; int y_stop;
	int x_current; int y_current;

	bool valid;
	bool two_edges;

	void init_edge(int x1, int y1, int x2, int y2);
	void update_edge();

	int dx;
	int dy;
	int x_step;
	int y_step;
	int Numerator;
	int Denominator;
	int Accumulator;
};

