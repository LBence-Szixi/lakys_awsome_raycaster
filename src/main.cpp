

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include "textures.h"

#include "texture_load.h"


// DEFINES
#define M_PI 3.14159265358979323846264338327950288419716939937510
#define TAU (M_PI * 2.0)
#define PI2 (M_PI / 2.0)
#define PI3 3 * (M_PI / 2.0)
#define DR 0.0174532925


// SETTINGS
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// CALLBACKS
void framebuffer_size_callback(GLFWwindow* window, int width, int height);  // Resize callback
static void error_callback(int error, const char* description); // Error callback

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods); // Key callback
char keys[1024]; // Array to store the state of each key

//void mouse_callback(GLFWwindow* window, double xposIn, double yposIn); // Mouse callback
//void scroll_callback(GLFWwindow* window, double x_offsetffset, double y_offsetffset); // Scroll callback

// FUNCTIONS
void processInput(GLFWwindow *window);

// DELTATIME
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
float currentFrame = 0.0f; // Time of current frame

// PLAYER
class Player
{
	public:
		float x, y;
		float delta_x, delta_y;
		float speed;

		float rot_speed;
		float angle;

		float fov; // Field of view

		Player(float x, float y, float speed, float rot_speed = 0.1f, float fov = 60.0f)
		{
			this->x = x;
			this->y = y;

			this->delta_x = 0;
			this->delta_y = 0;
			this->angle = 0;
			
			this->speed = speed;
			this->rot_speed = rot_speed;

			this->fov = fov;
		}
};

Player player(300, 300, 50, 5, 60);

float degToRad(int a) { return a*M_PI/180.0;}
float FixAng(float a){ if(a>359){ a-=360;} if(a<0){ a+=360;} return a;}
void draw_player2D();

// MAP
int mapW = 8;
int mapH = 8;
int mapS = mapW * mapH;
int map_walls[]=           //the map array. Edit to change level but keep the outer walls
{
 1,1,1,1,1,3,1,1,
 1,0,0,2,0,0,0,1,
 1,0,0,4,0,0,0,1,
 1,4,2,2,0,0,5,0,
 2,0,0,0,0,5,0,0,
 2,0,0,0,5,5,5,0,
 2,0,0,0,0,0,0,1,
 1,1,3,1,3,1,3,1,	
};

int map_floor[]=          //floors
{
 7,7,7,7,7,7,7,7,
 7,0,0,7,7,7,7,7,
 7,0,0,0,6,7,7,7,
 7,7,7,7,6,7,7,7,
 7,7,6,6,6,7,7,7,
 7,7,6,7,7,7,7,7,
 7,7,6,7,8,8,8,7,
 7,7,7,7,7,7,7,7,	
};

int map_ceiling[]=          //ceiling
{
 1,1,1,1,1,1,1,1,
 1,5,5,1,0,0,0,1,
 1,5,5,1,0,0,0,1,
 1,1,1,1,1,1,1,1,
 1,2,1,1,1,1,1,1,
 1,2,1,1,1,1,1,1,
 1,1,1,1,0,0,0,1,
 1,1,1,1,1,1,1,1,	
};

void draw_rays_2D();
void draw_map_2D();

// RAYS
float distance(float ax, float ay, float bx, float by) {
	return sqrt((bx-ax)*(bx-ax) + (by-ay)*(by-ay));
}
void draw_sky(float angle);
float render_distance = 16;
int resolution = 3;
float render_resolution = SCR_WIDTH / resolution;


// TEXTURES
ImageData sky_tex;

auto Textures = new ImageData[10];

int main()
{


	// loop through textures folder and load all textures
	for(int i = 0; i < 10; i++)
	{
		char path[256];
		sprintf(path, "textures/%d.png", i);
		Textures[i] = loadImageToArray(path);

		if(Textures[i].data == nullptr)
		{
			return 1;
		}
	}

	sky_tex = loadImageToArray("textures/sky.png");

	if(sky_tex.data == nullptr)
	{
		return 1;
	}



	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Laky's Awesome Raycaster", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Set callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, key_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, SCR_WIDTH, SCR_HEIGHT, 0.0f, 0.0f, 1.0f);

	// Place the player in the tile 4,3
	player.x = 7 * mapS - mapS/2;
	player.y = 7 *mapS - mapS/2;
	player.speed = 50;
	player.angle = 0;
	player.rot_speed = 5;

	player.fov = 60;

	// set player delta X and Y so that we don't need to rotate to move
	player.delta_x = cos(player.angle) * 5;
	player.delta_y = sin(player.angle) * 5;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// set title to FPS
		char title[256];
		sprintf(title, "Laky's Awesome Raycaster - FPS: %d", (int)round(1.0f / deltaTime));
		glfwSetWindowTitle(window, title);

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//draw_map_2D();
		//draw_player2D();

		// draw background
		draw_sky(player.angle);

		draw_rays_2D();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}


// PLAYER
void draw_player2D()
{
	glColor3f(1,1,0);
	glPointSize(8);
	glLineWidth(4);

	glBegin(GL_POINTS);
	glVertex2i(player.x,player.y);
	glEnd();

	glBegin(GL_LINES);
	glVertex2i(player.x,player.y);
	glVertex2i(player.x + player.delta_x * 5, player.y + player.delta_y * 5);
	glEnd();
}

// MAP 2D
void draw_map_2D()
{
	int x, y, x_offset, y_offset;
	
	for(y = 0; y < mapH; y++)
	{
		for(x = 0; x < mapW; x++)
		{
			// set color depending on whether the cell is a wall or not
			if(map_walls[y*mapW+x] > 0)
			{
				if(map_walls[y*mapW+x] == 4)
				{
					glColor3f(1,0.2,0.2);
				}
				else
				{
					glColor3f(1,1,1);
				}
				
			}
			else
			{
				glColor3f(0.0, 0.0, 0.0);
			}

			x_offset = x * mapS;
			y_offset = y * mapS;

			// draw tile; +1 and -1 offset to add border between tiles
			glBegin(GL_QUADS);
			glVertex2i(x_offset +1, y_offset +1);
			glVertex2i(x_offset +1, y_offset + mapS -1);
			glVertex2i(x_offset + mapS -1, y_offset + mapS -1);
			glVertex2i(x_offset + mapS -1, y_offset +1);
			glEnd();
		}
	}
}

// RAYS
void draw_rays_2D()
{
	int r;
	int mx, my, mp, dof;
	float rx, ry, ra, x_offset, y_offset;
	float total_dist;

	float ray_angle_increment = (player.fov * DR) / render_resolution; // Angle between each ray
	
	ra = player.angle - (player.fov/2.0f * DR); // Start at left edge of FOV
	if(ra < 0) { ra += TAU; }
	if(ra > TAU) { ra -= TAU; }

	// It's Ray castin time 🧙🏻


	for(r = 0; r < render_resolution; r++)
	{

		int vert_map_tex, horiz_map_tex = 0;

		total_dist = 0;

		/* Check the horizontal lines first 
		 - if looking up or down set the ray direction as an x and y position using some scary math
		 - if looking straight left or right, we aren't handling that yet so we just set the player position as the ray position
		 - then we loop through our "render distance", checking if we hit a wall
		 - if we hit a wall, we stop the loop
		 - else we just add the offset and try again???? idk how is this this simple but yeah that's all we need to do
		*/

		dof = 0;
		float dist_h = 1000000, hx = player.x, hy = player.y;
		float aTan = -1/tan(ra);

		if(sin(ra) > 0.001) // Looking up
		{
			ry = (((int)player.y>>6)<<6) + 64;
			rx = (player.y-ry) * aTan + player.x;
			y_offset = 64;
			x_offset = -y_offset * aTan;
		}
		else if(sin(ra) < -0.001) // Looking down
		{
			ry = (((int)player.y>>6)<<6) - 0.0001;
			rx = (player.y-ry) * aTan + player.x;
			y_offset = -64;
			x_offset = -y_offset * aTan;
		}
		else // Looking straight
		{
			rx = player.x;
			ry = player.y;
			dof = 8;
		}

		while(dof < 8)
		{
			mx = (int)(rx)>>6;
			my = (int)(ry)>>6;
			mp = my*mapW+mx;
			if(mp > 0 && mp < mapW*mapH && map_walls[mp] > 0)
			{
				horiz_map_tex = map_walls[mp]-1;

				hx = rx;
				hy = ry;
				dist_h = distance(player.x, player.y, hx, hy);
				dof = 8;
			}
			else
			{
				rx += x_offset;
				ry += y_offset;
				dof += 1;
			}
		}

		/* Now check the vertical lines
		 - if looking left or right set the ray direction as an x and y position using some scary math
		 - if looking straight up or down, we have already handled that so we just set the player position as the ray position
		 - then we loop through our "render distance", checking if we hit a wall
		 - if we hit a wall, we stop the loop
		 - once again somehow it is that simple! idk how but it is
		*/

		dof = 0;
		float dist_v = 1000000, vx = player.x, vy = player.y;
		float nTan = -tan(ra);

		if(cos(ra) < -0.001) // Looking left
		{
			rx = (((int)player.x>>6)<<6) - 0.0001;
			ry = (player.x-rx) * nTan + player.y;
			x_offset = -64;
			y_offset = -x_offset * nTan;
		}
		else if(cos(ra) > 0.001) // Looking right
		{
			rx = (((int)player.x>>6)<<6) + 64;
			ry = (player.x-rx) * nTan + player.y;
			x_offset = 64;
			y_offset = -x_offset * nTan;
		}
		else // Looking straight up/down
		{
			rx = player.x;
			ry = player.y;
			dof = 8;
		}

		while(dof < 8)
		{
			mx = (int)(rx)>>6;
			my = (int)(ry)>>6;
			mp = my*mapW+mx;
			if(mp > 0 && mp < mapW*mapH && map_walls[mp] > 0)
			{
				vert_map_tex = map_walls[mp]-1;

				vx = rx;
				vy = ry;
				dist_v = distance(player.x, player.y, vx, vy);
				dof = 8;
			}
			else
			{
				rx += x_offset;
				ry += y_offset;
				dof += 1;
			}
		}


		float shade = 1; // Simple shading for the walls

		// Use shortest distance
		if(dist_v < dist_h)
		{
			horiz_map_tex = vert_map_tex;

			rx = vx;
			ry = vy;
			total_dist = dist_v;
			
			shade = 0.7;
		}
		else
		{
			vert_map_tex = horiz_map_tex;

			rx = hx;
			ry = hy;
			total_dist = dist_h;
			
			shade = 1.0;
		}

		/*glLineWidth(3);
		glColor3f(0,0.8,0);
		glBegin(GL_LINES);
		glVertex2i(player.x, player.y);
		glVertex2i(rx, ry);
		glEnd();*/

		//draw_walls_3D(total_dist, r, ra);

		// Draw dem walls
		// We need to fix the fisheye effect first!!!!
		float ca = player.angle - ra;
		if(ca < 0) { ca += TAU; }
		if(ca > TAU) { ca -= TAU; }

		total_dist = total_dist * cos(ca); // Fix the fisheye effect

		// The longer the distance, the shorter the wall
		// This is because if we divide the height of the wall by the distance, the wall will appear shorter the further away it is

		float line_height = (mapS * SCR_HEIGHT) / total_dist; // The height of the line (1 ray = 1 line)

		// Texture mapping
		float tex_y_step = Textures[horiz_map_tex].height / (float)line_height;
		float tex_y_offset = 0;

		float textureDivisor = 2.0f * (32.0f / Textures[horiz_map_tex].width); // To properly scale the texture horizontally, no matter what resolution

		// Clip the line height to the screen height
		if(line_height > SCR_HEIGHT) { tex_y_offset = (line_height-SCR_HEIGHT)/2.0; line_height = SCR_HEIGHT; }
		float line_offset = (SCR_HEIGHT/2) - (line_height / 2); // Line offset to center the line

		// Draw textured walls
		float tex_y = tex_y_offset * tex_y_step; // (horiz_map_tex * 32) is the offset of the texture: 32 is the width of the texture, so 1 is the first texture, 2 is the second, etc.

		float tex_x;
		
		if (shade == 1) // Vertical wall
		{ 
			tex_x = (int)(rx / textureDivisor) % Textures[horiz_map_tex].width; 
			if (!(ra > M_PI && ra <= TAU)) { tex_x = (Textures[horiz_map_tex].width - 1) - tex_x; } // Facing south
		}  
		else // Horizontal wall
		{ 
			tex_x = (int)(ry / textureDivisor) % Textures[horiz_map_tex].width; 
			if (ra > M_PI / 2 && ra <= 3 * M_PI / 2) { tex_x = (Textures[horiz_map_tex].width - 1) - tex_x; } // Facing west
		}

		if(ra > 180) // Flip the texture if the ray is looking southwards
		{
			tex_x = (Textures[horiz_map_tex].width - 1) - tex_x;
		}
		

		// Draw walls
		glPointSize(resolution);
		glBegin(GL_POINTS);
		for(int y = 0; y < line_height; y++)
		{
			/*float color = All_Textures[(int)(tex_y) * 32 + (int)(tex_x)] * shade;

			// Color the walls
			if(horiz_map_tex==0){ glColor3f(color, color, color);} // Checkerboard white
			if(horiz_map_tex==1){ glColor3f(color, color/2.0, color/2.0);} // Brick red
			if(horiz_map_tex==2){ glColor3f(color/2.0, color/2.0, color    );} //window blue
			if(horiz_map_tex==3){ glColor3f(color/2.0, color, color/2.0);} //door green
			if(horiz_map_tex==4){ glColor3f(color, color*0.7, 0);} // L orange*/

			//glColor3f(color, color, color);

			int img_x = (int)(tex_x);
			int img_y = (int)(tex_y);

			int pixel = ((img_y * Textures[horiz_map_tex].width + img_x) * 3);
			float red = Textures[horiz_map_tex].data[pixel] / 255.0;
			float green = Textures[horiz_map_tex].data[pixel + 1] / 255.0;
			float blue = Textures[horiz_map_tex].data[pixel + 2] / 255.0;

			//glColor3f(Textures[horiz_map_tex].data[(img_y * Textures[horiz_map_tex].width + img_x) * 3] / 255.0, Textures[horiz_map_tex].data[(img_y * Textures[horiz_map_tex].width + img_x) * 3 + 1] / 255.0, Textures[horiz_map_tex].data[(img_y * Textures[horiz_map_tex].width + img_x) * 3 + 2] / 255.0);
			glColor3f(red * shade, green * shade, blue * shade);

			glVertex2i(r*resolution, y+line_offset);
			

			tex_y += tex_y_step;
		}
		glEnd();

		// Draw floors
		glPointSize(resolution);
		glBegin(GL_POINTS);
		//---draw floors---
		for(int y=line_offset+line_height;y<SCR_HEIGHT;y++)
		{
			float raFix = cos(player.angle - ra);

			// Step 1: Calculate the distance to the floor point
			// The further down the screen we look, the further away the floor point is
			float screen_y_ratio = (float)(y - SCR_HEIGHT/2) / (SCR_HEIGHT/2);
			float planeDistance = (mapS * SCR_HEIGHT) / std::max(1.0, (2.0 * y - SCR_HEIGHT));
			

			// Step 2: Calculate the actual floor point in world space
			float floor_x = player.x + cos(ra) * planeDistance;
			float floor_y = player.y + sin(ra) * planeDistance;

			/*float magic_number = (my_actual_screen_height / 320) * 158
			float floor_x=player.x/2 + cos(ra)*158*32/dy/raFix;
			float floor_y=player.y/2 - sin(ra)*158*32/dy/raFix;*/

			int tex_size = Textures[horiz_map_tex].width; // This should be 128 for your textures
			int tex_x = (int)((floor_x / mapS - (int)(floor_x / mapS)) * tex_size) & (tex_size - 1);
			int tex_y = (int)((floor_y / mapS - (int)(floor_y / mapS)) * tex_size) & (tex_size - 1);

			


			int mp= map_floor[(int)(floor_y / mapS) * mapW + (int)(floor_x / mapS)];
			//printf("mp: %d\n", mp);
			int pixel = ((int)(tex_y) * tex_size + (int)(tex_x)) * 3;
			float red = Textures[mp].data[pixel] / 255.0 * 0.7;
			float green = Textures[mp].data[pixel + 1] / 255.0  * 0.7;
			float blue = Textures[mp].data[pixel + 2] / 255.0  * 0.7;
			glColor3f(red,green,blue); glVertex2i(r*resolution,y);

			//---draw ceiling---
			mp= map_ceiling[(int)(floor_y / mapS) * mapW + (int)(floor_x / mapS)];
			//printf("mp: %d\n", mp);
			pixel = ((int)(tex_y) * tex_size + (int)(tex_x)) * 3;
			red = Textures[mp].data[pixel] / 255.0 * 0.7;
			green = Textures[mp].data[pixel + 1] / 255.0  * 0.7;
			blue = Textures[mp].data[pixel + 2] / 255.0  * 0.7;
			glColor3f(red,green,blue);
			if(mp > 0)
			{
				glVertex2i(r*resolution,SCR_HEIGHT-y);
			}
			
			/*mp = 0;
			pixel=(((int)(tex_y)&tex_modulo)*tex_size + ((int)(tex_x)&tex_modulo))*3;
			red = Textures[mp].data[pixel] / 255.0;
			green = Textures[mp].data[pixel + 1] / 255.0;
			blue = Textures[mp].data[pixel + 2] / 255.0;
			if(mp > 0)
			{
				glColor3f(red,green,blue);
				glVertex2i(r*resolution,SCR_HEIGHT-y);
			}*/
				
		}
		glEnd();

		ra += ray_angle_increment;
		if(ra < 0) { ra += TAU; }
		if(ra > TAU) { ra -= TAU; }
	}
}

void draw_sky(float angle)
{
	int x, y;


	glPointSize(resolution);
	glBegin(GL_POINTS);
	for(y = 0; y < sky_tex.height; y++)
	{
		for(x = 0; x < sky_tex.width; x++)
		{
			int x_offset = (int)(angle*250 - x);
			if(x_offset < 0) { x_offset += sky_tex.width; }
			x_offset = x_offset % sky_tex.width;
			//x_offset = -x_offset;
			int pixel = (y * sky_tex.width + x_offset) * 3;
			glColor3f(sky_tex.data[pixel] / 255.0, sky_tex.data[pixel + 1] / 255.0, sky_tex.data[pixel + 2] / 255.0);
			
			glVertex2i(x*resolution, y*2);
			
		}
	}
	glEnd();
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Rotation
	if(keys[GLFW_KEY_A])
	{
		player.angle -= player.rot_speed * deltaTime;

		if(player.angle < 0)
		{
			player.angle += TAU; // TAU is 2 * PI
		}

		player.delta_x = cos(player.angle) * 5;
		player.delta_y = sin(player.angle) * 5;
	}

	if(keys[GLFW_KEY_D])
	{
		player.angle += player.rot_speed * deltaTime;

		if(player.angle > TAU)
		{
			player.angle -= TAU; // TAU is 2 * PI
		}

		player.delta_x = cos(player.angle) * 5;
		player.delta_y = sin(player.angle) * 5;
	}

	//Strafing
	if(keys[GLFW_KEY_LEFT])
	{
		player.x -= player.speed * deltaTime;
	}
	if(keys[GLFW_KEY_RIGHT])
	{
		player.x += player.speed * deltaTime;
	}

	// Forward and backward
	
	// Get x and y offset for collision detection
	int x_offset, y_offset = 0;
	if(player.delta_x < 0) {x_offset = -20;} else {x_offset = 20;}
	if(player.delta_y < 0) {y_offset = -20;} else {y_offset = 20;}

	// Get grid position
	int ipx = player.x / mapS;
	int ipx_add_xo = (player.x + x_offset) / mapS;
	int ipx_sub_xo = (player.x - x_offset) / mapS;

	int ipy = player.y / mapS;
	int ipy_add_yo = (player.y + y_offset) / mapS;
	int ipy_sub_yo = (player.y - y_offset) / mapS;

	// Move player with collision detection
	if(keys[GLFW_KEY_W])
	{
		if(map_walls[ipy * mapW + ipx_add_xo] == 0)
		{
			player.x += player.delta_x * deltaTime * player.speed;
		}

		if(map_walls[ipy_add_yo * mapW + ipx] == 0)
		{
			player.y += player.delta_y * deltaTime * player.speed;
		}
	}
	if(keys[GLFW_KEY_S])
	{
		if(map_walls[ipy * mapW + ipx_sub_xo] == 0)
		{
			player.x -= player.delta_x * deltaTime * player.speed;
		}

		if(map_walls[ipy_sub_yo * mapW + ipx] == 0)
		{
			player.y -= player.delta_y * deltaTime * player.speed;
		}
	}

	// Interact
	if(keys[GLFW_KEY_E])
	{
		int x_offset, y_offset = 0;
		if(player.delta_x < 0) {x_offset = -25;} else {x_offset = 25;}
		if(player.delta_y < 0) {y_offset = -25;} else {y_offset = 25;}

		// Get grid position
		int ipx = player.x / mapS;
		int ipx_add_xo = (player.x + x_offset) / mapS;

		int ipy = player.y / mapS;
		int ipy_add_yo = (player.y + y_offset) / mapS;
		if(map_walls[ipy_add_yo * mapW + ipx_add_xo] == 4)
		{
			map_walls[ipy_add_yo * mapW + ipx_add_xo] = 0;
		}
	}
}


// CALLBACKS

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, SCR_WIDTH, SCR_HEIGHT, 0.0f, 0.0f, 1.0f);

}

// Error callback
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}
 
// Key callback
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (action == GLFW_PRESS)
	{
		keys[key] = 1;
	}
	else if (action == GLFW_RELEASE)
	{
		keys[key] = 0;
	}
}