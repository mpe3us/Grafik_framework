#include "Edge_rasterizer.h"
#include <iostream>
#include "DotMaker.h"

void Edge_rasterizer::init(int x1, int y1, int x2, int y2)
{
	this->two_edges = false;

	this->x1 = x1;
	this->y1 = y1;

	this->x2 = x2;
	this->y2 = y2;

	this->init_edge(x1, y1, x2, y2);
}

void Edge_rasterizer::init(int x1, int y1, int x2, int y2, int x3, int y3)
{
	this->two_edges = true;

	this->x1 = x1;
	this->y1 = y1;

	this->x2 = x2;
	this->y2 = y2;

	this->x3 = x3;
	this->y3 = y3;

	this->init_edge(x1, y1, x2, y2);
}

void Edge_rasterizer::init_edge(int x1, int y1, int x2, int y2)
{
	this->x_start = x1;
	this->y_start = y1;

	this->x_stop = x2;
	this->y_stop = y2;

	this->x_current = this->x_start;
	this->y_current = this->y_start;

	int dx = this->x_stop - this->x_start;
	int dy = this->y_stop - this->y_start;

	this->x_step = (dx < 0) ? -1 : 1;
	this->y_step = 1;
	this->Numerator = std::abs(dx); // Numerator = |dx|
	this->Denominator = dy; // dy > 0 Assumption
	this->Accumulator = (x_step > 0) ? Denominator : 1;

	this->valid = (this->y_current < this->y_stop);
}

bool Edge_rasterizer::more_fragments() const
{
	return this->valid;
}

void Edge_rasterizer::next_fragment()
{
	this->y_current += this->y_step;
	if (this->y_current >= this->y_stop) {
		if (!(this->two_edges)) {
			this->valid = false;
		}
		else {
			this->init_edge(x2, y2, x3, y3);
			this->two_edges = false;
		}
	}
	else {
		this->update_edge();
	}
}

void Edge_rasterizer::update_edge()
{
	this->Accumulator += this->Numerator;
	while (this->Accumulator > this->Denominator) {
		this->x_current += this->x_step;
		this->Accumulator -= this->Denominator;
	}
}

int Edge_rasterizer::x() const
{
	if (!this->valid) {
		throw std::runtime_error(
			"edge_rasterizer::x(): Invalid State"
		);
	}
	return this->x_current;
}

int Edge_rasterizer::y() const
{
	if (!this->valid) {
		throw std::runtime_error(
			"edge_rasterizer::y(): Invalid State"
		);
	}
	return this->y_current;
}

Edge_rasterizer::Edge_rasterizer(void) : valid(false)
{
}


Edge_rasterizer::~Edge_rasterizer(void)
{
}
