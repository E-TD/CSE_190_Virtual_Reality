#include "window.h"
using namespace std;

const char* window_title = "GLFW Starter Project";
/*Skybox code taken from tutorial linked in project description. 
Skybox jpegs downloaded from link in project description*/
/*LoadPPM code taken from sample code in project description*/

Cam* myCam;
Cam* bearCam;
Cam* currCam;
Point* activePt;
Group* tracks;
int camCtr = 0;
Track* active_track;
float track_pt = 0.0f;
float grav = 300.0f;
float speed = 0.01f;
int dor = 1;
Group* world;
Group* trecks;

Lights* light;
GLuint skyboxVAO, skyboxVBO;
GLint shaderProgram;
GLint skyboxShader;
GLint trackShader;
int mode = 4;
int keystate = 0;

GLuint texture;

std::vector<const GLchar*> textures_faces;

// Default camera parameters
glm::vec3 cam_pos(0.0f, 0.0f, 20.0f);		// e  | Position of camera
glm::vec3 cam_look_at(0.0f, 0.0f, 0.0f);	// d  | This is where the camera looks at
glm::vec3 cam_up(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is

int Window::width = 500;
int Window::height = 500;
glm::vec2 center((float)Window::width / 2.0f, (float)Window::height / 2.0f);
glm::vec2 moPos(0.0f, 0.0f);
int pressToggle = 0;
Track* this_track;
float temp_height = -100.0f;
float max_height = temp_height;
float min_height = max_height;
float ht_addres = 0.0f;

glm::mat4 Window::P;
glm::mat4 Window::V;


unsigned char* loadPPM(const char* filename, int& width, int& height)
{
	const int BUFSIZE = 128;
	FILE* fp;
	unsigned int read;
	unsigned char* rawData;
	char buf[3][BUFSIZE];
	char* retval_fgets;
	size_t retval_sscanf;

	if ((fp = fopen(filename, "rb")) == NULL)
	{
		std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
		width = 0;
		height = 0;
		return NULL;
	}

	// Read magic number:
	retval_fgets = fgets(buf[0], BUFSIZE, fp);

	// Read width and height:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');
	retval_sscanf = sscanf(buf[0], "%s %s", buf[1], buf[2]);
	width = atoi(buf[1]);
	height = atoi(buf[2]);

	// Read maxval:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');

	// Read image data:
	rawData = new unsigned char[width * height * 3];
	read = (unsigned int)fread(rawData, width * height * 3, 1, fp);
	fclose(fp);
	if (read != 1)
	{
		std::cerr << "error parsing ppm file, incomplete data" << std::endl;
		delete[] rawData;
		width = 0;
		height = 0;
		return NULL;
	}

	return rawData;
}

GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char* image;
	int zeruh = 0;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = loadPPM(faces[i], width, height);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, zeruh, GL_RGB, GL_UNSIGNED_BYTE, image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}

Group* draw_control() {

	std::vector<glm::vec3> vertices;
	GLuint VAO, VBO;

	std::list<Node*>::iterator it = tracks->children.begin();//children[0]
	it++;
	glm::vec3 sav = ((Point*)(*it))->loc;//children[1]
	
	for (int i = 0; i < 7; i++) {
		it++;
		vertices.push_back(((Point*)(*it))->loc);
		it++;
		it++;
		vertices.push_back(((Point*)(*it))->loc);
	}

	it++;
	vertices.push_back(((Point*)(*it))->loc);
	vertices.push_back(sav);
	it++;

	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);

	glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive vertex attributes. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	glEnableVertexAttribArray(0); //enable the var

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO




	// We need to calculate this because as of GLSL version 1.40 (OpenGL 3.1, released March 2009), gl_ModelViewProjectionMatrix has been
	// removed from the language. The user is expected to supply this matrix to the shader when using modern OpenGL.
	GLuint MatrixID = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);

	MatrixID = glGetUniformLocation(shaderProgram, "mode");
	glUniform1i(MatrixID, 3);

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 16);
	glBindVertexArray(0);

	return ((Group*)(*it));
}

Point* manip_pt(double ex, double why) {
	glm::vec2 pt, mouse, diff;
	std::list<Node*>::iterator it = tracks->children.begin();//children[0]
	glm::vec4 cam_space;
	float x, y;

	float mouse_x = ((2.0f * (float)ex) - Window::width) / Window::width;
	float mouse_y = (Window::height - (2.0f * (float)why)) / Window::height;
	mouse = glm::vec2(mouse_x, mouse_y);
	Point *found = nullptr;
	for (int i = 0; i < 24; i++) {
		cam_space = Window::P * Window::V * ((Point*)(*it))->toWorld * glm::vec4(((Point*)(*it))->loc, 1.0);
		x = cam_space.x / cam_space.w;
		y = cam_space.y / cam_space.w;
		pt = glm::vec2(x, y);
		diff = pt - mouse;
		
		if (abs(length(diff)) < 0.035f) {
			found = ((Point*)(*it));
			std::cout << "HIT\n";
		}
		it++;
	}
	return found;

}

void update_tracks(Group* track_grp) {
	for (std::list<Node*>::iterator it = track_grp->children.begin(); it != track_grp->children.end(); ++it) {
		((Track*)(*it))->update_curves();
	}
}

Track* find_highest_pos() {
	temp_height = -100.0f;
	max_height = temp_height;
	min_height = 100.0f;
	ht_addres = 0.0f;
	Track* to_ret = nullptr;
	for (std::list<Node*>::iterator it = trecks->children.begin(); it != trecks->children.end(); ++it) {
		if (to_ret == nullptr) {
			to_ret = ((Track*)(*it));
		}
		this_track = ((Track*)(*it));
		temp_height = this_track->vertices[0].y;

		if (temp_height > max_height) {//initialize 
			max_height = temp_height;
			ht_addres = 0.0f;
			to_ret = this_track;
		}
		else if (temp_height < min_height) {//initialize
			min_height = temp_height;
		}

		for (int i = 1; i < 200; i = i + 2) {
			temp_height = this_track->vertices[i].y;
			if (temp_height > max_height) {
				max_height = temp_height;
				ht_addres = ((float)i)/200.0f;
				
				to_ret = this_track;
			}
			else if (temp_height < min_height) {
				min_height = temp_height;
			}

		}
	}
	return to_ret;
}

void place_highest_pos() {

	active_track = find_highest_pos();
	track_pt = ht_addres;
	float two[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		active_track->find_pt(track_pt).x, active_track->find_pt(track_pt).y + 0.5f, active_track->find_pt(track_pt).z, 1.0f };
	world->mat = glm::make_mat4(two);

}

void Window::initialize_objects()
{
	glLineWidth(5.0f);
	glPointSize(10.0f);
	GLfloat skyboxVertices[] = {
		// Positions          
		-500.0f,  500.0f, -500.0f,
		-500.0f, -500.0f, -500.0f,
		500.0f, -500.0f, -500.0f,
		500.0f, -500.0f, -500.0f,
		500.0f,  500.0f, -500.0f,
		-500.0f,  500.0f, -500.0f,

		-500.0f, -500.0f,  500.0f,
		-500.0f, -500.0f, -500.0f,
		-500.0f,  500.0f, -500.0f,
		-500.0f,  500.0f, -500.0f,
		-500.0f,  500.0f,  500.0f,
		-500.0f, -500.0f,  500.0f,

		500.0f, -500.0f, -500.0f,
		500.0f, -500.0f,  500.0f,
		500.0f,  500.0f,  500.0f,
		500.0f,  500.0f,  500.0f,
		500.0f,  500.0f, -500.0f,
		500.0f, -500.0f, -500.0f,

		-500.0f, -500.0f,  500.0f,
		-500.0f,  500.0f,  500.0f,
		500.0f,  500.0f,  500.0f,
		500.0f,  500.0f,  500.0f,
		500.0f, -500.0f,  500.0f,
		-500.0f, -500.0f,  500.0f,

		-500.0f,  500.0f, -500.0f,
		500.0f,  500.0f, -500.0f,
		500.0f,  500.0f,  500.0f,
		500.0f,  500.0f,  500.0f,
		-500.0f,  500.0f,  500.0f,
		-500.0f,  500.0f, -500.0f,

		-500.0f, -500.0f, -500.0f,
		-500.0f, -500.0f,  500.0f,
		500.0f, -500.0f, -500.0f,
		500.0f, -500.0f, -500.0f,
		-500.0f, -500.0f,  500.0f,
		500.0f, -500.0f,  500.0f
	};

	textures_faces.push_back("right.ppm");
	textures_faces.push_back("left.ppm");
	textures_faces.push_back("top.ppm");
	textures_faces.push_back("bottom.ppm");
	textures_faces.push_back("back.ppm");
	textures_faces.push_back("front.ppm");
	// Setup skybox VAO
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	texture = loadCubemap(textures_faces);

	myCam = new Cam();
	currCam = myCam;
	bearCam = new Cam();
	bearCam->cam_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	bearCam->cam_look_at = glm::vec3(0.0f, 0.0f, 1.0f);
	bearCam->direction = glm::vec3(0.0f, 0.0f, 1.0f);
	
	light = new Lights(1);
	// Load the shader program. Similar to the .obj objects, different platforms expect a different directory for files
#ifdef _WIN32 // Windows (both 32 and 64 bit versions)
	shaderProgram = LoadShaders("../shader.vert", "../shader.frag");
	skyboxShader = LoadShaders("../sbshader.vert", "../sbshader.frag");
	trackShader = LoadShaders("../trackshader.vert", "../trackshader.frag");
#else // Not windows
	shaderProgram = LoadShaders("shader.vert", "shader.frag");
#endif

	world = new Group(shaderProgram);
	world->update(glm::mat4(1.0f));

	tracks = init_tracks();

	

	bearCam->cam_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec4 bearMod(bearCam->cam_pos, 1.0f);
	float mov[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f, 1.0f };
	glm::mat4 movpls = glm::make_mat4(mov);
	bearMod = world->bear->toWorld * movpls *  bearMod;
	bearCam->cam_pos = glm::vec3(bearMod.x, bearMod.y, bearMod.z);
	bearCam->cam_look_at = bearCam->cam_pos + bearCam->direction;

}

Group* Window::init_tracks() {
	glm::vec3 ay1(-4.142135624f, 0.0f, 10.0f);
	glm::vec3 es1(-2.0f, 2.0f, 10.0f);
	glm::vec3 es2(2.0f, -2.0f, 10.0f);
	glm::vec3 ay2(4.142135624f, 0.0f, 10.0f);

	Point* a1 = new Point(trackShader, ay1, 0);
	Point* a2 = new Point(trackShader, ay2, 0);
	Point* s1 = new Point(trackShader, es1, 1);
	a1->pair = s1;
	s1->anchor = a1;
	Point* s2 = new Point(trackShader, es2, 1);
	s2->anchor = a2;
	a2->anchor = s2;
	Point* initial_pts[4] = { a1, s1, s2, a2 };

	Track* track1 = (new Track(trackShader, initial_pts));
	Track* orig = track1;
	Track* track2;
	Group* to_ret = new Group();
	Group* trackGrp = new Group();

	to_ret->addChild(initial_pts[0]);
	to_ret->addChild(initial_pts[1]);
	to_ret->addChild(initial_pts[2]);
	to_ret->addChild(initial_pts[3]);
	trackGrp->addChild(track1);
	
	glm::mat4 rotatey(1.0f);
	glLoadIdentity(); // load the identity matrix onto the matrix stack
	glRotatef(45.0f, 0.0f, 1.0f, 0.0f); // rotate this identity matrix by angleor degrees around axisor
	glMultMatrixf((GLfloat *)&rotatey[0][0]); // multiply this rotated matrix by the toWorld matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)&rotatey[0][0]); // overwrite the toWorld matrix with this new matrix

	Point *ab1 = a1;
	Point *sb1 = s1;
	Point *sb2 = s2;
	Point *ab2 = a2;
	glm::vec4 ay24, es14, es24;
	glm::vec3 e1_a1;

	for (int i = 0; i < 7; i++) {
		ay1 = glm::vec3(ay2);//change pt2 to be pt1 of a new line

		//rotate ay2 to be endpt of new line
		ay24 = glm::vec4(ay2, 1.0f);
		ay24 = rotatey * ay24;
		ay2 = glm::vec3(ay24.x, ay24.y, ay24.z);

		//make es1 the mirrored point of the previous es2
		e1_a1 = es2 - ay1;
		es1 = ay1 - e1_a1;
		ab1 = ab2;
		sb1 = new Point(trackShader, es1, 1);
		ab2->pair = sb1;
		sb1->anchor = ab1;
		sb1->pair = sb2;
		sb2->pair = sb1;
		to_ret->addChild(sb1);



		if (i == 6) {//last iteration
			e1_a1 = s1->loc - a1->loc;
			es1 = a1->loc - e1_a1;
			
			sb2 = new Point(trackShader, es1, 1);
			sb2->anchor = a1;
			sb2->pair = s1;
			s1->pair = sb2;
			to_ret->addChild(sb2);

			ab2 = a1;
			s1->pair = sb2;
			sb2->pair = s1;
			//a1->anchor = sb2;
		}
		else {
			//rotate es2 to be ctrlpt of new line
			es24 = glm::vec4(es2, 1.0f);
			es24 = rotatey * es24;
			es2 = glm::vec3(es24.x, es24.y, es24.z);
			sb2 = new Point(trackShader, es2, 1);
			to_ret->addChild(sb2);

			ab2 = new Point(trackShader, ay2, 0);
			to_ret->addChild(ab2);
		}
		sb2->anchor = ab2;
		ab2->anchor = sb2;
		initial_pts[0] = ab1;
		initial_pts[1] = sb1;
		initial_pts[2] = sb2; 
		initial_pts[3] = ab2;

		track2 = (new Track(trackShader, initial_pts));
		track1->next = track2;
		track2->prev = track1;
		track1 = track2;
		trackGrp->addChild(track2);
	}

	track2->next = orig;
	orig->prev = track2;

	to_ret->addChild(trackGrp);
	return to_ret;
}

float calcPE() {
	//find_highest_pos();
	float curr_height = active_track->find_pt(track_pt).y;

	float mgh_max = max_height + 0.2f;

	float old_speed = speed;
	if(curr_height <= min_height) {
		curr_height = min_height;
	}
	float delta = mgh_max - curr_height;
	if (delta <= 0.0f) {
		dor = -dor;
		speed = old_speed;
		//std::cout << speed << std::endl;
	}
	else {
		speed = sqrtf(10.0f* (mgh_max - curr_height)) / 1500.0f;
	}

	return speed;
}

void next_state() {
	float speed = calcPE();
	glm::mat4 old_wld = world->mat;
	Track* old_track = active_track;
	float old_trp = track_pt;

	track_pt = track_pt + (dor*speed);
	if (track_pt < 0.0f) {
		active_track = active_track->prev;
		track_pt = track_pt + 1.0f;
		float two[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			active_track->find_pt(track_pt).x, active_track->find_pt(track_pt).y + 0.5f, active_track->find_pt(track_pt).z, 1.0f };
		world->mat = glm::make_mat4(two);
	}
	else if (track_pt < 1.0f) {
		float two[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			active_track->find_pt(track_pt).x, active_track->find_pt(track_pt).y + 0.5f, active_track->find_pt(track_pt).z, 1.0f };
		world->mat = glm::make_mat4(two);
	}
	else {
		active_track = active_track->next;
		track_pt = track_pt - 1.0f;
		float two[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			active_track->find_pt(track_pt).x, active_track->find_pt(track_pt).y + 0.5f, active_track->find_pt(track_pt).z, 1.0f };
		world->mat = glm::make_mat4(two);
	}

	float twoo[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		old_track->find_pt(old_trp).x, old_track->find_pt(old_trp).y + 0.5f, old_track->find_pt(old_trp).z, 1.0f };
	old_wld = glm::make_mat4(twoo);

	world->calc_dir(old_wld, dor);
	
}

void Window::clean_up()
{
	delete(myCam);
	delete(bearCam);
	delete(world);//should recursively delete all other children
	delete(light);
	glDeleteProgram(shaderProgram);
	glDeleteProgram(skyboxShader);
	glDeleteProgram(trackShader);
}

GLFWwindow* Window::create_window(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Create the GLFW window
	GLFWwindow* window = glfwCreateWindow(width, height, window_title, NULL, NULL);

	// Check if the window could not be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window
	glfwMakeContextCurrent(window);

	// Set swap interval to 1
	glfwSwapInterval(1);

	// Get the width and height of the framebuffer to properly resize the window
	glfwGetFramebufferSize(window, &width, &height);
	// Call the resize callback to make sure things get drawn immediately
	Window::resize_callback(window, width, height);

	return window;
}

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
	Window::width = width;
	Window::height = height;

	center = glm::vec2((float)Window::width / 2.0f, (float)Window::height / 2.0f);

	// Set the viewport size
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);
	}
}

void Window::idle_callback()
{
	next_state();
	if (keystate != 0) {
		if (keystate == GLFW_KEY_W)
		{
			currCam->translate(0);
		}
		else if (keystate == GLFW_KEY_A)
		{

			currCam->translate(1);
		}
		else if (keystate == GLFW_KEY_S)
		{

			currCam->translate(2);
		}
		else if (keystate == GLFW_KEY_D)
		{

			currCam->translate(3);
		}

		V = glm::lookAt(currCam->cam_pos, currCam->cam_look_at, currCam->cam_up);
	}
}

void Window::display_callback(GLFWwindow* window)
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// skybox cube
	glDepthMask(GL_FALSE);// Remember to turn depth writing off
	// Update the scene graph
	world->update(glm::mat4(1.0f));
	bearCam->cam_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	bearCam->cam_look_at = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec4 bearPos(bearCam->cam_pos, 1.0f);
	glm::vec4 bearLookAt(bearCam->cam_look_at, 1.0f);
	float mov[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f, 1.0f };
	glm::mat4 movpls = glm::make_mat4(mov);
	bearPos = world->bear->toWorld * movpls *  bearPos;
	bearLookAt = world->bear->toWorld * movpls *  bearLookAt;
	bearCam->cam_pos = glm::vec3(bearPos.x, bearPos.y, bearPos.z);
	bearCam->cam_look_at = glm::vec3(bearLookAt.x, bearLookAt.y, bearLookAt.z);
	bearCam->direction = bearCam->cam_look_at - bearCam->cam_pos;
	
	// Draw skybox first
	glUseProgram(skyboxShader);
	V = glm::lookAt(currCam->cam_pos - currCam->cam_pos, currCam->cam_look_at - currCam->cam_pos, currCam->cam_up);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, &V[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, &P[0][0]);
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(skyboxShader, "skybox"), 0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);

	// Use the shader of programID
	glUseProgram(shaderProgram);
	V = glm::lookAt(currCam->cam_pos, currCam->cam_look_at, currCam->cam_up);
	GLuint MatrixID = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Window::V[0][0]);
	MatrixID = glGetUniformLocation(shaderProgram, "perspective");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Window::P[0][0]);
	glUniform1i(glGetUniformLocation(shaderProgram, "skybox"), 0);
	MatrixID = glGetUniformLocation(shaderProgram, "cameraPos");
	glUniform3f(MatrixID, (currCam->cam_pos.x), (currCam->cam_pos.y), (currCam->cam_pos.z));


	MatrixID = glGetUniformLocation(shaderProgram, "daLight.intens");
	glUniform3f(MatrixID, (light->intensity.x), (light->intensity.y), (light->intensity.z));
	MatrixID = glGetUniformLocation(shaderProgram, "daLight.dir");
	glUniform3f(MatrixID, (light->direction.x), (light->direction.y), (light->direction.z));
	MatrixID = glGetUniformLocation(shaderProgram, "daLight.amb");
	glUniform1f(MatrixID, light->ambient);
	MatrixID = glGetUniformLocation(shaderProgram, "daLight.pos");
	glUniform3f(MatrixID, light->position.x, light->position.y, light->position.z);
	MatrixID = glGetUniformLocation(shaderProgram, "daLight.spec");
	glUniform1f(MatrixID, light->specular);
	MatrixID = glGetUniformLocation(shaderProgram, "daLight.theta");
	glUniform1f(MatrixID, light->theta);
	MatrixID = glGetUniformLocation(shaderProgram, "daLight.mode");
	glUniform1i(MatrixID, light->mode);
	MatrixID = glGetUniformLocation(shaderProgram, "daLight.cosExp");
	glUniform1f(MatrixID, light->cosExp);
	
	world->draw();

	glUseProgram(trackShader);
	MatrixID = glGetUniformLocation(trackShader, "view");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Window::V[0][0]);
	MatrixID = glGetUniformLocation(trackShader, "perspective");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Window::P[0][0]);
	tracks->draw();
	glLineWidth(1.0f);
	trecks = draw_control();
	if (active_track == nullptr) {
		place_highest_pos();
	}
	glLineWidth(5.0f);

	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();
	// Swap buffers
	glfwSwapBuffers(window);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	keystate = 0;
	// Check for a key press
	if (action == GLFW_PRESS)
	{
		// Check if escape was pressed
		if (key == GLFW_KEY_ESCAPE)
		{
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		else if (key == GLFW_KEY_0)
		{
			grav = 200.0f;
		}
		else if (key == GLFW_KEY_1)
		{
			grav = 300.0f;
		}
		else if (key == GLFW_KEY_2)
		{
			grav = 400.0f;
		}
		else if (key == GLFW_KEY_3)
		{
			grav = 500.0f;
		}
		else if (key == GLFW_KEY_4)
		{
			grav = 600.0f;
		}
		else if (key == GLFW_KEY_5)
		{
			grav = 700.0f;
		}
		else if (key == GLFW_KEY_6)
		{
			grav = 800.0f;
		}
		else if (key == GLFW_KEY_7)
		{
			grav = 900.0f;
		}
		else if (key == GLFW_KEY_8)
		{
			grav = 1000.0f;
		}
		else if (key == GLFW_KEY_9)
		{
			grav = 1200.0f;
		}
		else if (key == GLFW_KEY_R) {
			moPos = glm::vec2(0.0f, 0.0f);
			mode = 4;
			place_highest_pos();
			light->reset();
			dor = 1;
			//myCam->reset();
		}
		else if (key == GLFW_KEY_E) {


		}
		else if (key == GLFW_KEY_W)
		{
			keystate = GLFW_KEY_W;

			currCam->translate(0);
		}
		else if (key == GLFW_KEY_A)
		{
			keystate = GLFW_KEY_A;
			currCam->translate(1);
		}
		else if (key == GLFW_KEY_S)
		{
			keystate = GLFW_KEY_S;
			currCam->translate(2);
		}
		else if (key == GLFW_KEY_D)
		{
			keystate = GLFW_KEY_D;
			currCam->translate(3);
		}
		else if (key == GLFW_KEY_C)
		{

			if (camCtr == 0) {
				currCam = bearCam;
				camCtr = 1;
			}
			else {
				currCam = myCam;
				camCtr = 0;
			}
		}
	}
	else if (action == GLFW_RELEASE) {
		keystate = 0;
	}
	else if (action == GLFW_REPEAT) {
		if (key == GLFW_KEY_W)
		{
			keystate = GLFW_KEY_W;
			currCam->translate(0);
		}
		else if (key == GLFW_KEY_A)
		{
			keystate = GLFW_KEY_A;
			currCam->translate(1);
		}
		else if (key == GLFW_KEY_S)
		{
			keystate = GLFW_KEY_S;
			currCam->translate(2);
		}
		else if (key == GLFW_KEY_D)
		{
			keystate = GLFW_KEY_D;
			currCam->translate(3);
		}
	}
	V = glm::lookAt(currCam->cam_pos, currCam->cam_look_at, currCam->cam_up);

}

void Window::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	glm::vec3 mousePos;
	float d;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) && !glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
	{
		float mouse_x = ((2.0f * (float)xpos) - Window::width) / Window::width;
		float mouse_y = (Window::height - (2.0f * (float)ypos)) / Window::height;
			glm::vec2 newPos(mouse_x, mouse_y);
			if (pressToggle == 0) {
				pressToggle = 1;
				activePt = manip_pt(xpos, ypos);//finds the point that your mouse clicks on
				moPos = glm::vec2(mouse_x,mouse_y);
			}
			if (activePt != nullptr) {
				activePt->move_pos(newPos - moPos, currCam);
				update_tracks(trecks);
				if (active_track->find_pt(track_pt).y > max_height) {
					max_height = active_track->find_pt(track_pt).y;
				}
			}

			moPos = glm::vec2(mouse_x, mouse_y);
	}
	else if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) && !glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
	{
		pressToggle = 0;
		activePt = nullptr;
	}
	else if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
	{
		if (mode == 4) {
			// manipulate the camera
			mousePos.x = ((2.0f * (float)xpos) - Window::width) / Window::width;
			mousePos.y = (Window::height - (2.0f * (float)ypos)) / Window::height;
			mousePos.z = 0.0f;
			d = length(mousePos);

			d = (d < 1.0f) ? d : 1.0f;

			if (pressToggle == 0) {
				pressToggle = 1;

				currCam->cursorPos = mousePos;
			}
			currCam->moveVV(mousePos);
			V = glm::lookAt(currCam->cam_pos, currCam->cam_look_at, currCam->cam_up);
		}
	}

}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (mode == 0) {

	}
	else if (mode == 1) {
		//directional light has no scroll functionality
	}
	else if (mode == 4) {//point light
		glm::vec3 tempor = normalize(currCam->cam_pos);

		currCam->cam_pos = currCam->cam_pos - ((float)(yoffset)*tempor);
		V = glm::lookAt(currCam->cam_pos, currCam->cam_look_at, currCam->cam_up);
	}
}

