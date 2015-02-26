#include "Line_rasterizer.h"
#include <iostream>
#include "DotMaker.h"

void Line_rasterizer::init(int x1, int y1, int x2, int y2)
{
	this->x_start = x1;
	this->y_start = y1;

	this->x_stop = x2;
	this->y_stop = y2;

	this->x_current = this->x_start;
	this->y_current = this->y_start;

	this->dx = this->x_stop - this->x_start;
	this->dy = this->y_stop - this->y_start;

	this->abs_2dx = std::abs(this->dx) << 1; // 2 * |dx|
	this->abs_2dy = std::abs(this->dy) << 1; // 2 * |dy|

	this->x_step = (this->dx < 0) ? -1 : 1;
	this->y_step = (this->dy < 0) ? -1 : 1;

	// Check to see if the line is x-dominant
	if (this->abs_2dx > this->abs_2dy)
	{
		// Initialize x-dominant line
		this->left_right = (this->x_step > 0);
		this->d = this->abs_2dy - (this->abs_2dx >> 1);
		this->valid = (this->x_start != this->x_stop);
		this->innerloop =
			&Line_rasterizer::x_dominant_innerloop;
	}
	// If not, then it's y-dominant
	else
	{
		// Initialize y-dominant line
		this->left_right = (this->y_step > 0);
		this->d = this->abs_2dx - (this->abs_2dy >> 1);
		this->valid = (this->y_start != this->y_stop);
		this->innerloop =
			&Line_rasterizer::y_dominant_innerloop;
	}
}

bool Line_rasterizer::more_fragments() const
{
	return this->valid;
}

void Line_rasterizer::next_fragment()
{
	// Run the innerloop once
	DotMaker::instance()->drawDot(x_current, y_current);
	(this->*innerloop)();

}

int Line_rasterizer::x() const
{
	if (!this->valid)
	{
		throw std::runtime_error("line_rasterizer::x(): Invalid State");
	}
	return this->x_current;
}

int Line_rasterizer::y() const
{
	if (!this->valid)
	{
		throw std::runtime_error("line_rasterizer::y(): Invalid State");
	}
	return this->y_current;
}

void Line_rasterizer::x_dominant_innerloop()
{
	if (this->x_current == this->x_stop)
	{
		this->valid = false;
	}
	else 
	{
		if ((this->d > 0) || 
			((this->d == 0) && this->left_right)) {
			this->y_current += this->y_step;
			this->d			-= this->abs_2dx;
		}
		this->x_current += this->x_step;
		this->d			+= this->abs_2dy;
	}

}

void Line_rasterizer::y_dominant_innerloop()
{
	if (this->y_current == this->y_stop)
	{
		this->valid = false;
	}
	else 
	{
		if ((this->d > 0) || 
			((this->d == 0) && this->left_right)) {
			this->x_current += this->x_step;
			this->d			-= this->abs_2dy;
		}
		this->y_current += this->y_step;
		this->d			+= this->abs_2dx;
	}

}

Line_rasterizer::Line_rasterizer() : valid(false)
{
}


Line_rasterizer::~Line_rasterizer()
{
}