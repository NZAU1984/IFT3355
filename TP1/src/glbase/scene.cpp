#include "scene.h"

#include <string>
#include <vector>

#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#pragma region NODE

GLint Node::uniform_model = -1, Node::uniform_color = -1;
GLint Node::attribute_position = 1, Node::attribute_normal = 2;

void Node::InitializePreLink(GLuint program)
{
	glBindAttribLocation(program, attribute_position, "in_position");
	glBindAttribLocation(program, attribute_normal, "in_normal");
}

void Node::InitializePostLink(GLuint program)
{
	uniform_model = glGetUniformLocation(program, "model");
	uniform_color = glGetUniformLocation(program, "color");
}

Node::Node()
	: _transform(), _children(), _parent(nullptr)
{ }

void Node::SetTransform(const mat4 &transform)
{
	_transform = transform;
}

void Node::AddChild(Node *child)
{
	_children.push_back(child);
	child->_parent = this;
}

glm::mat4 Node::fullTransform() const
{
	if (_parent == nullptr)
		return _transform;
	else
		return _parent->fullTransform() * _transform;
}

#pragma endregion

#pragma region SHAPE

void Shape::Render()
{
	glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(fullTransform()));
	glUniform4fv(uniform_color, 1, glm::value_ptr(_color));

	if (_color.a < 1)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
	}
	else
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
}

void Shape::applyTransformatioMatrix(VertexPositionNormal* vertices, uint nVertices, const glm::mat4& transformationMatrix)
{
	if (transformationMatrix != glm::mat4())
	{	
		float minX = vertices[0].position.x, 
			maxX = vertices[0].position.x,
			minY = vertices[0].position.y,
			maxY = vertices[0].position.y,
			minZ = vertices[0].position.z,
			maxZ = vertices[0].position.z;

		for (uint i = 0; i < nVertices; ++i)
		{
			/* We have to adapt normals, but only some cases, for example, a rotation. If we shear a shape, we must
			NOT modify the normals. The following line was taken from 
			http://www.arcsynthesis.org/gltut/Illumination/Tut09%20Normal%20Transformation.html */
			glm::mat4 normalTransformationMatrix = glm::transpose(glm::inverse(transformationMatrix));

			VertexPositionNormal* currentVertex = &vertices[i];
			vec4 currentVertexPosition = vec4((*currentVertex).position, 1);
			vec4 currentVertexNormal = vec4((*currentVertex).normal, 1);			
			(*currentVertex).position = vec3(transformationMatrix * currentVertexPosition);
			(*currentVertex).normal = glm::normalize(vec3(normalTransformationMatrix * currentVertexNormal));

			//_LOG_INFO() << "From " << currentVertexPosition.x << ", " << currentVertexPosition.y << ", " << currentVertexPosition.z
			//	<< "\n......................................to " << (*currentVertex).position.x << ", " << (*currentVertex).position.y << ", " << (*currentVertex).position.z;
		
			minX = min(minX, (*currentVertex).position.x);
			maxX = max(maxX, (*currentVertex).position.x);

			minY = min(minY, (*currentVertex).position.y);
			maxY = max(maxY, (*currentVertex).position.y);

			minZ = min(minZ, (*currentVertex).position.z);
			maxZ = max(maxZ, (*currentVertex).position.z);
		}

		_LOG_INFO() << "minX=" << minX << ", maxX=" << maxX << ", minY=" << minY << ", maxY=" << maxY << ", minZ=" << minZ << ", maxZ=" << maxZ;
	}
}

glm::vec3 Shape::getCenterVector() const
{
	glm::vec4 transformedCenter = _transform * glm::vec4(0, 0, 0, 1);

	return glm::vec3(transformedCenter.x, transformedCenter.y, transformedCenter.z);
}

Shape::~Shape()
{
	if (_vertexBuffer != BAD_BUFFER)
		glDeleteBuffers(1, &_vertexBuffer);

	if (_indexBuffer != BAD_BUFFER)
		glDeleteBuffers(1, &_indexBuffer);

	glDeleteVertexArrays(1, &_vao);
}
#pragma endregion

#pragma region BOX

/*Box::Box(vec4 color)
{
	glm::mat4 dummyMatrix;
	Box(color, dummyMatrix);
}*/
/* Constructs a cube with a color corresponding to 'color' and applies to it an initial transformation matrix (optional
parameter). */
Box::Box(vec4 color, const mat4& initialTransformationMatrix)
{
	_LOG_INFO() << "blablabla ; " << color[0] << ", " << color[1] << ", " << color[2];
	_vertexBuffer = _indexBuffer = BAD_BUFFER;

	_color = color;

	VertexPositionNormal vertices[36] = {
		{ vec3(0, 0, 0), vec3(0, -1, 0) },
		{ vec3(1, 0, 0), vec3(0, -1, 0) },
		{ vec3(0, 0, 1), vec3(0, -1, 0) },

		{ vec3(0, 0, 1), vec3(0, -1, 0) },
		{ vec3(1, 0, 0), vec3(0, -1, 0) },
		{ vec3(1, 0, 1), vec3(0, -1, 0) },


		{ vec3(1, 0, 0), vec3(1, 0, 0) },
		{ vec3(1, 1, 0), vec3(1, 0, 0) },
		{ vec3(1, 0, 1), vec3(1, 0, 0) },

		{ vec3(1, 0, 1), vec3(1, 0, 0) },
		{ vec3(1, 1, 0), vec3(1, 0, 0) },
		{ vec3(1, 1, 1), vec3(1, 0, 0) },


		{ vec3(1, 1, 0), vec3(0, 1, 0) },
		{ vec3(0, 1, 1), vec3(0, 1, 0) },
		{ vec3(1, 1, 1), vec3(0, 1, 0) },

		{ vec3(0, 1, 0), vec3(0, 1, 0) },
		{ vec3(0, 1, 1), vec3(0, 1, 0) },
		{ vec3(1, 1, 0), vec3(0, 1, 0) },


		{ vec3(0, 1, 1), vec3(-1, 0, 0) },
		{ vec3(0, 1, 0), vec3(-1, 0, 0) },
		{ vec3(0, 0, 1), vec3(-1, 0, 0) },

		{ vec3(0, 1, 0), vec3(-1, 0, 0) },
		{ vec3(0, 0, 0), vec3(-1, 0, 0) },
		{ vec3(0, 0, 1), vec3(-1, 0, 0) },


		{ vec3(0, 0, 1), vec3(0, 0, 1) },
		{ vec3(1, 0, 1), vec3(0, 0, 1) },
		{ vec3(0, 1, 1), vec3(0, 0, 1) },

		{ vec3(1, 0, 1), vec3(0, 0, 1) },
		{ vec3(1, 1, 1), vec3(0, 0, 1) },
		{ vec3(0, 1, 1), vec3(0, 0, 1) },


		{ vec3(0, 0, 0), vec3(0, 0, -1) },
		{ vec3(0, 1, 0), vec3(0, 0, -1) },
		{ vec3(1, 0, 0), vec3(0, 0, -1) },

		{ vec3(1, 0, 0), vec3(0, 0, -1) },
		{ vec3(0, 1, 0), vec3(0, 0, -1) },
		{ vec3(1, 1, 0), vec3(0, 0, -1) }
	};

	

	for (uint x = 0; x < 36; x++)
	{
		vertices[x].position -= 0.5;
	}

	/* Modification from original code. Applies an initial transformation matrix. */
	applyTransformatioMatrix(vertices, 36, initialTransformationMatrix);

	// Create Vertex Array Object
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// Generate Vertex Buffer
	glGenBuffers(1, &_vertexBuffer);

	// Fill Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Set Vertex Attributes
	glEnableVertexAttribArray(attribute_position);
	glVertexAttribPointer(attribute_position, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)0);
	glEnableVertexAttribArray(attribute_normal);
	glVertexAttribPointer(attribute_normal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)(0 + sizeof(vec3)));

	glBindVertexArray(0);

	debugGLError();
}

void Box::Render()
{
	Shape::Render();

	glBindVertexArray(_vao);
	
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
}

#pragma endregion

#pragma region SPHERE

/*Sphere::Sphere(vec4 color, uint nUpperStacks, uint nSlices, float radius) :
	_nStacks(2 * nUpperStacks + 1),

	_nSlices(nSlices),

	_vertexOrderVectorSize((2 * (nSlices * 3)) + (((2 * nUpperStacks) * (2 * nSlices)) * 3)),

	_radius(radius)
{
	glm::mat4 dummyMatrix;
	Sphere(color, nUpperStacks, nSlices, radius, glm::mat4());
}*/

/* Sphere constructor. Creates a sphere with a radius of 'radius'. Between the top vertex and the middle of the sphere
are 'nUpperStacks' stacks, which are a circle of 'nSlices' vertices used to approximate a circle. There is the same
amount of stacks below the middle. The middle is also a circle and also had 'nSlices' vertices. Finally, there is a
bottom vertex. The sphere will have a color defined by 'color' and may use an initial transformation matrix to
initially transform every vertex to reduce calculations when a frame is rendered. */
Sphere::Sphere(vec4 color, uint nUpperStacks, uint nSlices, float radius, const mat4& initialTransformationMatrix) :

	/* Total number of elements in vertexOrderVector. For every triangle, there are 3 indices corresponding each to a
	specific vertex.
	- First part (nSlices * 3 * 2): the top and bottom parts are like a pie, there's a center vertex (the very top of
	    the sphere / the very bottom of the sphere) and vertices around it. For each part, there are 'nSlices'
		triangles, each triangles having 3 coordinates. Since we have two parts, we multiply by '2'.
	- Second part (2 * nSlices * nUpperStacks * 3 * 2): there are '2 * nUpperStacks' "in between two stacks" parts. For
	    each, there are '2 * nSlices' triangles because each vertex of a stack 'A' (which is further away from the
	    center stack than stack 'B') is linked to the corresponding vertex on stack 'B' (same theta angle) and one
	    triangle is formed with the vertex "to the right", and another with the vertex "to the left" on stack 'B'.
		Finally, we multiply by '3' because there are 3 vertices per triangle.
	*/
	_vertexOrderVectorSize((2 * (nSlices * 3)) + (((2 * nUpperStacks) * (2 * nSlices)) * 3)),
	
	/* Storing radius for collision detection. */
	_radius(radius)
{
	_vertexBuffer = _indexBuffer = BAD_BUFFER;

	_color = color;

	/* Total number of stacks, '+ 1' is for the middle stack. */
	uint nStacks = 2 * nUpperStacks + 1;

	/* Total number of vertices. '+ 2' for top and bottom vertices. */
	uint nVertices = (nSlices * nStacks) + 2;

	/* Array of vertices
	=> 'NEW' OPERATOR :: CREATED ON HEAP :: POSSIBLE MEMORY LEAK => solved by using 'delete[]' further below. 
	*/
	VertexPositionNormal* vertices = new VertexPositionNormal[nVertices];

	/* Indices of top and bottom vertices. Indices will go from the highest vertex (topIndex) to the lower vertex
	(bottomIndex). The order of vertices on every stack will be the same (going in the same direction). */
	uint topIndex = 0;
	uint bottomIndex = nVertices - 1;

	/* Top and bottom vertices. */
	vertices[topIndex] = { vec3(0, _radius, 0), vec3(0, 1, 0) };
	vertices[bottomIndex] = { vec3(0, -_radius, 0), vec3(0, -1, 0) };

	/* This will tell OpenGL in which order to use the vertices to create all required triangles. Indices are not
	important (that's why we use a vector instead of an array like for vertices), the only important thing is that
	vertex indices for the same triangle be consecutive elements. */
	std::vector<GLuint> vertexOrderVector;

	/* Let's build the triangles for the top and bottom parts which are simply like a pie. */
	for (uint i = 1, iMax = nSlices; i <= iMax; ++i)
	{
		/* Top part. Each triangle always starts from the center vertex. */
		vertexOrderVector.push_back(topIndex);
		vertexOrderVector.push_back(i);
		vertexOrderVector.push_back((i == nSlices) ? 1 : (i + 1));

		/* Bottom part. Each triangle always starts from the center vertex. */
		vertexOrderVector.push_back(bottomIndex);
		vertexOrderVector.push_back(bottomIndex - i);
		vertexOrderVector.push_back((i == nSlices) ? (bottomIndex - 1) : (bottomIndex - i - 1));
	}

	/* Let's build the sphere with two loops. The outter loop is for stacks, the inner loop is for vertices all around
	a specific stack. We only loop through the upper stacks and reflect the upper vertices on the lower part by
	inverting the Y coordinates. */
	for (uint currentStack = 0; currentStack <= nUpperStacks; ++currentStack)
	{
		/* Phi is the angle used on the Y axis. In a sphere, the top is pi/2, and the bottom is -pi/2. Here we
		calculate the phi angle for the current stack. */
		float phi = (glm::pi<float>() / nStacks) * (nUpperStacks - currentStack);
			//(glm::pi<float>() / 2) - (((glm::pi<float>() / 2) / (nUpperStacks + 1)) * (currentStack + 1));

		float currentY = _radius * glm::sin(phi);

		/* The radius of every stack is dependent on phi. It's smaller at the top, larger at the center. */
		float currentRadius = _radius * glm::cos(phi);

		/* Indices used in the vertex array. */
		uint upperStackOffset	= ((currentStack) * nSlices) + 1;
		uint lowerStackOffset = ((nStacks - currentStack - 1) * nSlices) + 1;

		/* For every stack, let's go around the circle and determine the X and Z coordinates. */
		for (uint currentSlice = 0; currentSlice < nSlices; ++currentSlice)
		{
			/* Indices of current vertices.  */
			topIndex = upperStackOffset + currentSlice;
			bottomIndex = lowerStackOffset + currentSlice;

			/* Theta is used to go around a stack to defined X and Z. */
			float theta = ((glm::pi<float>() * 2) / nSlices) * currentSlice;

			float currentX = currentRadius * glm::cos(theta);
			float currentZ = currentRadius * glm::sin(theta);

			/* Upper and lower vertices for current X and Z (and Y) coordinates. We flip the sign of Y to reflect the
			upper vertex on the bottom. */
			vertices[topIndex] = {vec3(currentX, currentY, currentZ), glm::normalize(vec3(currentX, currentY, currentZ))};
			vertices[bottomIndex] = { vec3(currentX, -currentY, currentZ), glm::normalize(vec3(currentX, -currentY, currentZ)) };

			uint nSlicesMinusOne = nSlices - 1;

			/* Let's build triangles with indices. The center stack won't enter this block because it does not have to
			create any triangles. The stack right above it and the one right below it will build triangles with its
			vertices. */
			if (upperStackOffset != lowerStackOffset)
			{
				/* Upper triangles. */
				vertexOrderVector.push_back(topIndex);
				vertexOrderVector.push_back(topIndex + nSlices); // stack right below, vertex has same theta angle
				vertexOrderVector.push_back(topIndex + 1);

				vertexOrderVector.push_back(topIndex);
				vertexOrderVector.push_back(topIndex + nSlices); // stack right below, vertex has same theta angle
				vertexOrderVector.push_back(topIndex + nSlicesMinusOne); // TODO extract expression cause repeated+++

				/* Lower triangles */
				vertexOrderVector.push_back(bottomIndex);
				vertexOrderVector.push_back(bottomIndex - nSlices); // stack right above, vertex has same theta angle
				vertexOrderVector.push_back(bottomIndex - 1);

				vertexOrderVector.push_back(bottomIndex);
				vertexOrderVector.push_back(bottomIndex - nSlices); // stack right above, vertex has same theta angle
				vertexOrderVector.push_back(bottomIndex - nSlicesMinusOne);
			}
		}
	}

	/* Applies an initial transformation matrix on every vertex. Won't do anything if it is an identity matrix (which
	is what we get when the parameter is not specified since it's optional). */
	applyTransformatioMatrix(vertices, nVertices, initialTransformationMatrix);

	// Create Vertex Array Object
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// Generate Vertex Buffer
	glGenBuffers(1, &_vertexBuffer);

	// Fill Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(vec3) * 2) * nVertices, vertices, GL_STATIC_DRAW);

	/* Create an element buffer. */
	GLuint ebo;
	glGenBuffers(1, &ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * _vertexOrderVectorSize, &vertexOrderVector[0], GL_STATIC_DRAW);

	// Set Vertex Attributes
	glEnableVertexAttribArray(attribute_position);
	glVertexAttribPointer(attribute_position, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)0);
	glEnableVertexAttribArray(attribute_normal);
	glVertexAttribPointer(attribute_normal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)(0 + sizeof(vec3)));

	glBindVertexArray(0);

	debugGLError();

	/* Prevents potential memory leak caused by 'new'. Since OpenGL copies vertices, we can delete the array. */
	delete[] vertices;
}

float Sphere::getRadius() const
{
	return _radius;
}

void Sphere::Render()
{
	Shape::Render();

	glBindVertexArray(_vao);

	/* This draws the sphere by using the list of vertices to tell OpenGL in which order to use them to create each
	triangle. */
	glDrawElements(GL_TRIANGLES, _vertexOrderVectorSize, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

bool Sphere::collisionDetected(const Sphere& sphereShape) const
{
		glm::vec3 myCenter = getCenterVector();
		glm::vec3 itsCenter = sphereShape.getCenterVector();

		glm::vec3 distanceVector = glm::vec3(0, 2, 0); //glm::vec3(itsCenter.x - myCenter.x, itsCenter.y - myCenter.y, itsCenter.z - myCenter.z);

		float distance = sqrt(pow(distanceVector.x, 2) + pow(distanceVector.y, 2) + pow(distanceVector.z, 2)); //distanceVector.length();

		_LOG_INFO() << "collisionDetected my x,y,z =" << myCenter.x << ", " << myCenter.y << ", " << myCenter.z;
		_LOG_INFO() << "collisionDetected its x,y,z =" << itsCenter.x << ", " << itsCenter.y << ", " << itsCenter.z;
		_LOG_INFO() << "collisionDetected distance x,y,z =" << distanceVector.x << ", " << distanceVector.y << ", " << distanceVector.z;
		_LOG_INFO() << "distance=" << distance;

		std::this_thread::sleep_for(std::chrono::seconds(1));

	return false;
}

#pragma endregion

#pragma region CYLINDER

/*Cylinder::Cylinder(vec4 color, uint nSlices, float height, float radius) :
_nSlices(nSlices),
_nTrianglesOnSide(nSlices * 2 + 2),
_offsetTop(0),
_offsetSide(nSlices + 1),
_offsetBottom(nSlices * 3 + 3)
{
	glm::mat4 dummyMatrix;
	Cylinder(color, nSlices, height, radius, dummyMatrix);
}*/
/* Cylinder constructor. Creates a cylinder with a height of 'height' and a radius of 'radius'. There are 'nSlices'
vertices used to create a circle. In total, there are four circles, twice the same circle on top, twice the same circle
on bottom. The reason we duplicate vertices is that for the top, the normals must be pointing upward, for the bottom,
the normals must be pointing downward, and on the side, they must be pointing to the side. The cylinder will have a
color defined by 'color' and may use an initial transformation matrix to initially transform every vertex to reduce
calculations when a frame is rendered. */
Cylinder::Cylinder(vec4 color, uint nSlices, float height, float radius, const mat4& initialTransformationMatrix) :
_nSlices(nSlices),
_nTrianglesOnSide(nSlices * 2 + 2),
_offsetTop(0),
_offsetSide(nSlices + 1),
_offsetBottom(nSlices * 3 + 3)
{
	_vertexBuffer = _indexBuffer = BAD_BUFFER;

	_color = color;

	/* There are '_nSlices' vertices on top with an upward normal, same amount on the bottom with downward normal,
	same amount on top with normal pointing to the side, and same amount on the bottom with normal pointing to the side
	as well. '+4' is for one center vertex on top, one center vertex on bottom (those two are used to make a
	triangle fan (GL_TRIANGLE_FAN), and on the side, we have to define the vertex at theta=0 twice to have a triangle
	strip (GL_TRIANGLE_STRIP) that goes all the way around. If we don't specify them twice, one of the sides is an
	open hole! */
	uint nVertices = _nSlices * 4 + 4;

	/* Vertex array. Operator 'new' => created on heap, possible memory leak solved below by calling 'delete[]'. */
	VertexPositionNormal* vertices = new VertexPositionNormal[nVertices];

	float upperHeight = height / 2.0f;
	float lowerHeight = -upperHeight;

	/* First vertex if the top center vertex, last vertex is the bottom center vertex. */
	vertices[_offsetTop] = { vec3(0, upperHeight, 0), vec3(0, 1, 0) };
	vertices[nVertices - 1] = { vec3(0, lowerHeight, 0), vec3(0, -1, 0) };

	/* Let's create two circles, one at the top, one at the bottom, and for each, define twice the same vertex position
	with different normals (one poiting upward/downward, the other pointing on the side). Notice the '<=' which we use
	to define the vertices at theta=0 twice as explained above. */
	for (uint i = 0; i <= _nSlices; ++i)
	{
		float theta = ((glm::pi<float>() * 2) / _nSlices) * i;
		float x = radius * glm::cos(theta);
		float z = radius * glm::sin(theta);

		uint offset = _offsetSide + (i * 2);

		/* Top circle, upward normal */
		vertices[_offsetTop + i] = { vec3(x, upperHeight, z), vec3(0, 1, 0) };

		/* Top circle, side-pointing normal */
		vertices[offset] = { vec3(x, upperHeight, z), glm::normalize(vec3(x, 0, z)) };

		/* Bottom circle, side-pointing normal */
		vertices[offset + 1] = { vec3(x, lowerHeight, z), glm::normalize(vec3(x, 0, z)) };

		/* Bottom circle, downward normal */
		vertices[_offsetBottom + i] = { vec3(x, lowerHeight, z), vec3(0, -1, 0) };
	}

	/* Applies an initial transformation matrix on every vertex. Won't do anything if it is an identity matrix (which
	is what we get when the parameter is not specified since it's optional). */
	applyTransformatioMatrix(vertices, nVertices, initialTransformationMatrix);

	// Create Vertex Array Object
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// Generate Vertex Buffer
	glGenBuffers(1, &_vertexBuffer);

	// Fill Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(vec3) * 2) * nVertices, vertices, GL_STATIC_DRAW);

	// Set Vertex Attributes
	glEnableVertexAttribArray(attribute_position);
	glVertexAttribPointer(attribute_position, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)0);
	glEnableVertexAttribArray(attribute_normal);
	glVertexAttribPointer(attribute_normal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)(0 + sizeof(vec3)));

	glBindVertexArray(0);

	debugGLError();

	/* vertices was created on the heap, possible memory leak solved by calling delete[]. Also, OpenGL does a copy
	of the vertices, so no risk. */
	delete[] vertices;
}

void Cylinder::Render()
{
	Shape::Render();

	glBindVertexArray(_vao);

	/* Top circle is displayed using a "triangle fan", first vertex is the center, then all other vertices connect with
	that vertex. */
	glDrawArrays(GL_TRIANGLE_FAN, _offsetTop, _nSlices);

	/* The side is displayed using a triangle fan using triangles alternating between top and bottom. */
	glDrawArrays(GL_TRIANGLE_STRIP, _offsetSide, _nTrianglesOnSide);

	/* Bottom circle is displayed like top circle. */
	glDrawArrays(GL_TRIANGLE_FAN, _offsetBottom, _nSlices);

	glBindVertexArray(0);
}

#pragma endregion
