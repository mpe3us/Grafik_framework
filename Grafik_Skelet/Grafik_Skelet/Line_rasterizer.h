#pragma once
class Line_rasterizer
{
public:
	Line_rasterizer(void);
	virtual ~Line_rasterizer(void);

	void init(int x1, int y1, int x2, int y2);
	bool more_fragments() const;
	void next_fragment();

	int x() const;
	int y() const;

	void x_dominant_innerloop();
	void y_dominant_innerloop();
private:
	int x_start;
	int y_start;

	int x_stop;
	int y_stop;

	int x_current;
	int y_current;

	bool left_right;

	int dx;
	int dy;

	int abs_2dx;
	int abs_2dy;

	int d;

	int x_step;
	int y_step;

	bool valid;

	void (Line_rasterizer::*innerloop)();
};

