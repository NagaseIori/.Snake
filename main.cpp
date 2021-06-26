#include <graphics.h>
#include <conio.h>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <random>
#include <queue>
#include <windows.h>
#include <mmsystem.h>
#define RELEASE
using namespace std;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

/* Other Tools */
inline int rand_int(int l, int r) { return uniform_int_distribution<int>(l, r)(rng); }
bool chance(int x)
{
	return rand_int(0, 100) <= x;
}
int manhattan_distance(int x1, int y1, int x2, int y2)
{
	return abs(x1 - x2) + abs(y1 - y2);
}

void itotchar(int x, TCHAR ch[])
{
	if (!x)
	{
		ch[0] = '0', ch[1] = 0;
		return;
	}
	int p = 0;
	if (x < 0)
	{
		p = 1;
		ch[0] = '-';
	}
	x = abs(x);
	while (x)
	{
		ch[p++] = x % 10 + '0', x /= 10;
	}
	int pj = ch[0] == '-';
	ch[p--] = 0;
	while (pj < p)
		swap(ch[p--], ch[pj++]);
}

const int M = 100;
int NX = 30, NY = 30;
int ma[M][M]; // over all map;
int food[M][M], food_t[M][M], food_ot[M][M];
int food_col[] = { 0, BGR(0xE74C3C), BGR(0xE74C3C), -1, BGR(0x9B59B6), BGR(0xE74C3C), BGR(0x5DADE2), BGR(0x9B59B6) };
int food_lcol[] = { 0, BGR(0xF1948A), BGR(0xF1948A), -1, BGR(0xC39BD3), BGR(0xF4D03F), BGR(0xAED6F1), BGR(0xF4D03F) };
int nx, ny = 1, dirx, diry = 1;
int room_speed = 60, ntiming, timing;
int hiscore = 0, nscore = 0, spdscr;
int BLOCK_SIZE = 16;
struct Snake
{
	int length, x, y;
} player;

/* Theme things */
int theme_col[] = { BGR(0x34495E), BGR(0x707B7C), BGR(0xec407a), BGR(0x1565c0), BLACK, 0xAAAAAA };
int theme_scol[] = { BGR(0xF4F6F6), BGR(0xF2F4F4), BGR(0xfce4ec), BGR(0xe3f2fd), WHITE, 0x222222 };
int theme_tot = 6, theme_now = 0;
int Main_Color, Side_Color;
void set_theme(int type)
{
	theme_now = type;
	Main_Color = theme_col[type];
	Side_Color = theme_scol[type];
	food_col[3] = food_lcol[3] = Main_Color;
	settextcolor(Main_Color);
}
void nxt_theme()
{
	set_theme(theme_now = (theme_now + 1) % theme_tot);
}

/* State Values */
bool b_BACKGROUND = false;
bool b_MULTIPLAYER = false;

/* Image Part */
char key[] = "shitrua";
void Image_Xor(ifstream& in, string out)
{
	ofstream fout(out, ios::binary);
	string str;
	stringstream buf;
	buf << in.rdbuf();
	str = buf.str();

	int keylength = strlen(key);
	for (int i = 0, j = 0; i < str.size(); i++, j++, j %= keylength)
	{
		str[i] ^= key[j];
	}
	fout << str;
	fout.close();
}

/* Main Part */
void Initialization();

queue<int> op_stacks; // operation sequence
void Check_Keyboard()
{
	char ch;
	while (_kbhit())
	{
		ch = _getch();
		if (ch == 'w')
			op_stacks.push(1);
		if (ch == 's')
			op_stacks.push(2);
		if (ch == 'a')
			op_stacks.push(3);
		if (ch == 'd')
			op_stacks.push(4);
		if (ch == 27)
			exit(0);
		if (ch == 'r')
			Initialization();
		if (ch == 'j')
			room_speed += 5, ntiming = min(ntiming, room_speed - 1);
		if (ch == 'k')
			room_speed -= 5, ntiming = min(ntiming, room_speed - 1);
		if (ch == 't')
			nxt_theme();
		room_speed = max(5, room_speed);
		printf("%d\n", ch);
	}
}

void Timing_Decrease(int time = 1, bool incfood = 1)
{
	for (int i = 0; i < NX; i++)
	{
		for (int j = 0; j < NY; j++)
		{
			if (ma[i][j] > 0)
				ma[i][j] -= min(time, ma[i][j]);
			if (food_t[i][j] > 0 && incfood)
			{
				food_t[i][j] -= min(time, food_t[i][j]);
				if (!food_t[i][j])
				{
					if (food[i][j] == 5)
						food[i][j] = 1, food_t[i][j] = -1;
					else
						food[i][j] = 0;
				}
			}
		}
	}
	if (spdscr > 10)
		spdscr--;
}

void Set_Food(int type = 1, int time = -1, int distance = 0, int bound = 0)
{
	if (type == 1 && chance(2))
		type = 5, time = rand_int(20, 40);
	if (type == 4 && chance(10))
		type = 7, time = rand_int(20, 40);
	int px, py;
	px = rand_int(bound, NX - 1 - bound);
	py = rand_int(bound, NY - 1 - bound);
	while (ma[px][py] || food_t[px][py] || manhattan_distance(px, py, player.x, player.y) <= distance)
	{
		px = rand_int(bound, NX - 1 - bound);
		py = rand_int(bound, NY - 1 - bound);
	}
	food[px][py] = type;
	food_ot[px][py] = food_t[px][py] = time;
	printf("Food: %d %d\n", px, py);
}

int Check_Round(int x, int y)
{
	int ans = 0;
	if (x == 0 || ma[x - 1][y] > 0 || food[x - 1][y] == 3)
		ans++;
	if (y == 0 || ma[x][y - 1] > 0 || food[x][y - 1] == 3)
		ans++;
	if (x == NX || ma[x + 1][y] > 0 || food[x + 1][y] == 3)
		ans++;
	if (y == NY || ma[x][y + 1] > 0 || food[x][y + 1] == 3)
		ans++;
	return ans - 1;
}

void Change_Length(int delta)
{
	if (delta > 0)
	{
		player.length += delta;
		Timing_Decrease(-delta + 1, false);
	}
	else if (delta < 0)
	{
		delta = -delta;
		delta = min(delta, player.length - 3);
		player.length -= delta;
		Timing_Decrease(delta + 1, false);
	}
}

bool Check_Impact()
{
	//printf("Check at: %d %d %d\n", player.x, player.y, food_t[player.x][player.y]);
	if (player.x < 0 || player.x == NX || player.y < 0 || player.y == NY)
		return true;
	if (ma[player.x][player.y] > 1)
		return true;
	if (food_t[player.x][player.y])
	{
		int type = food[player.x][player.y];
		if (type == 3)
			return true;
		if (type == 1 || type == 2)
			player.length++;
		if (type == 5)
			Change_Length(10), spdscr *= 10;
		if (type == 4)
			Change_Length(-1);
		if (type == 6)
			Change_Length(5), spdscr *= 3, set_theme(rand_int(0, theme_tot - 1));
		if (type == 7)
			Change_Length(-10), spdscr *= 10;
		if (chance(10))
		{
			Set_Food(6, rand_int(30, 60));
		}
		if (type == 1)
		{
			Set_Food(1);
			if (chance(5))
				Set_Food(2), Set_Food(2);
		}
		if (type == 5)
		{
			Set_Food(1);
			for (int i = 0; i < 10; i++)
				Set_Food(2);
		}
		if (chance(30))
		{
			int times = rand_int(1, 6);
			while (times--)
			{
				Set_Food(3, rand_int(50, 150), 10);
			}
		}
		if (chance(20))
		{
			Set_Food(4, rand_int(30, 100));
		}
		food_t[player.x][player.y] = 0;
		food[player.x][player.y] = 0;
		spdscr *= pow(3, Check_Round(player.x, player.y));
		if (room_speed > 60)
			spdscr /= 2;
		if (room_speed > 70)
			spdscr /= 2;
		if (room_speed > 85)
			spdscr /= 2;
		if (room_speed > 100)
			spdscr = 1;
		if (room_speed == 25)
			spdscr *= 2;
		if (room_speed == 20)
			spdscr *= 4;
		if (room_speed == 15)
			spdscr *= 8;
		if (room_speed == 10)
			spdscr *= 16;
		if (room_speed == 5)
			spdscr *= 32;
		nscore += spdscr, spdscr = NX * 3;
	}
	else
	{
		Timing_Decrease();
	}
	return false;
}

void Snake_Draw(int ox, int oy, int color = Main_Color, int size = BLOCK_SIZE)
{
	if (b_BACKGROUND)
		color = Side_Color;
	setfillcolor(color);
	setlinecolor(color);
	for (int i = 0; i < M; i++)
	{
		for (int j = 0; j < M; j++)
		{
			if ((!b_BACKGROUND && ma[j][i] > 0) || (b_BACKGROUND && ma[j][i] <= 0 && i < NX && j < NY))
				fillrectangle(ox + i * size, oy + j * size, ox + (i + 1) * size, oy + (j + 1) * size);
		}
	}
}

void Food_Draw(int ox, int oy, int size = BLOCK_SIZE)
{
	LINESTYLE origin;
	getlinestyle(&origin);
	setlinestyle(PS_SOLID, 3);
	for (int i = 0; i < NY; i++)
	{
		for (int j = 0; j < NX; j++)
		{
			if (food_t[j][i])
			{
				int type = food[j][i];
				setlinecolor(food_lcol[type]);
				setfillcolor(food_col[type]);
				fillrectangle(ox + i * size, oy + j * size, ox + (i + 1) * size, oy + (j + 1) * size);
			}
		}
	}
	setlinestyle(PS_SOLID, 3);
	for (int i = 0; i < NY; i++)
	{
		for (int j = 0; j < NX; j++)
		{
			if (food_t[j][i] > 0)
			{
				int type = food[j][i];
				setlinecolor(BGR(0x58D68D));
				setfillcolor(food_col[type]);
				line(ox + i * size - 2, oy + j * size, ox + i * size - 2, oy + j * size + BLOCK_SIZE * food_t[j][i] / food_ot[j][i]);
			}
		}
	}
	setlinestyle(&origin);
}

IMAGE img;
void Game_Draw(bool);
void Game_Over()
{
	hiscore = max(hiscore, nscore);
	Game_Draw(1);
	if (nscore >= 10000 && b_BACKGROUND)
	{
		ifstream fin("bg", ios::binary);
		Image_Xor(fin, "reward.jpg");
		fin.close();
	}
	while (_getch() != 'r')
		;
	Initialization();
}

void Step_Event()
{
	while (op_stacks.size())
	{
		int op = op_stacks.front();
		op_stacks.pop();
		switch (op)
		{
		case 1:
			nx = -1, ny = 0;
			break;
		case 2:
			nx = 1, ny = 0;
			break;
		case 3:
			nx = 0, ny = -1;
			break;
		case 4:
			nx = 0, ny = 1;
			break;
		}
		if ((nx && nx == -dirx) || (ny && ny == -diry))
		{
			nx = dirx, ny = diry;
			printf("INVALID!\n");
			continue;
		}
		break;
	}
	dirx = nx, diry = ny;
	player.x += dirx, player.y += diry;
	if (Check_Impact())
	{
		Game_Over();
	}
	ma[player.x][player.y] = player.length;
}

/* Particle Part */
int preset_color[] = { 0xe3f2fd, 0xe8eaf6, 0xfce4ec, 0xe1f5fe, 0xe0f7fa, 0xe8f5e9, 0xfff8e1, 0xfbe9e7, 0xeceff1 };
struct Particle
{
	double x, y, vx, vy;
	int color;
	Particle()
	{
		x = rand_int(0, 800), y = 608, vx = rand_int(-5, 5) / 5.0, vy = rand_int(-5, -1) / 5.0, color = preset_color[rand_int(0, 8)];
	}
	void Move() { x += vx, y += vy; }
	bool outside() { return x < 0 || x > 800 || y < 0 || y > 608; }
	void Draw(int col) { col = BGR(col), setlinecolor(col), setfillcolor(col), fillrectangle(x, y, x + 5, y + 5); }
	// void Debug() { printf("%d %d %d %d %d\n", x, y, vx, vy, color); }
};
vector<Particle> particles;
int n_particle = 100;

void Particle_Draw()
{
	for (int i = 0; i < rand_int(0, 3) && particles.size() < n_particle; i++)
	{
		particles.push_back(Particle());
	}
	for (Particle& x : particles)
		x.Draw(WHITE), x.Move(), x.Draw(x.color);
	for (int i = 0; i < particles.size(); i++)
		if (particles[i].outside())
		{
			particles[i].Draw(WHITE);
			particles.erase(particles.begin() + i);
		}
}

void Map_Draw(int ox, int oy, int nmap[][M], int size = 2)
{
	setlinecolor(Main_Color), setfillcolor(Side_Color), fillrectangle(ox, oy, ox + NX * size, oy + NY * size);
	for (int i = 0; i < NY; i++)
		for (int j = 0; j < NX; j++)
		{
			if (nmap[j][i])
			{
				setlinecolor(Main_Color);
				rectangle(ox + i * size, oy + j * size, ox + (i + 1) * size - 1, oy + (j + 1) * size - 1);
			}
		}
}

void UI_Draw(int ox, int oy, bool done = 0)
{
	TCHAR strscr[50];
	itotchar(nscore, strscr);
	outtextxy(ox, oy, _T("SCORE"));
	outtextxy(ox, oy + 40, strscr);
	if (done)
	{
		outtextxy(ox, oy + 160, _T(">w<"));
	}
	if (hiscore)
	{
		outtextxy(ox, oy + 80, _T("HISCORE"));
		itotchar(hiscore, strscr);
		outtextxy(ox, oy + 120, strscr);
	}
	// Map_Draw(ox, oy + 300, ma);
}

void Game_Draw(bool done = 0)
{
	int px = 10, py = 16 * 5;
	//if (!abnormal) cleardevice();
	Particle_Draw();
	if (b_BACKGROUND)
	{
		putimage(px, py, &img);
	}
	else
	{
		LINESTYLE origin;
		getlinestyle(&origin);
		setlinestyle(PS_SOLID, 3);
		setlinecolor(Main_Color), setfillcolor(Side_Color), fillrectangle(px, py, px + NX * BLOCK_SIZE, py + NY * BLOCK_SIZE);
		setlinestyle(&origin);
	}
	Snake_Draw(px, py);
	Food_Draw(px, py);
	if (b_BACKGROUND)
	{
		LINESTYLE origin;
		getlinestyle(&origin);
		setlinestyle(PS_SOLID, 3);
		setlinecolor(Main_Color), rectangle(px, py, px + NX * BLOCK_SIZE, py + NY * BLOCK_SIZE);
		setlinestyle(&origin);
	}
	UI_Draw(px + NX * BLOCK_SIZE + 30, py, done);
	FlushBatchDraw();
}
void Main_Loop()
{
	while (1)
	{
		Check_Keyboard();
		ntiming++, timing++;
		if (ntiming == room_speed)
		{
			ntiming = 0;
			Step_Event();
		}
		if (timing % 6 == 0)
			Game_Draw();
		Sleep(1);
	}
}

void Initial_Draw()
{
	settextstyle(100, 0, _T("Fixedsys"));
	outtextxy(240, 80, _T(".Snake"));
	settextstyle(40, 0, _T("Fixedsys"));
	outtextxy(600, 120, _T(">O<"));
	outtextxy(40, 320, _T("[W, A, S, D] to control .Snake"));
	outtextxy(40, 360, _T("[J, K] to adjust the speed"));
	outtextxy(40, 400, _T("[R] to restart   [ESC] to exit"));
	outtextxy(40, 440, _T("[T] to change theme"));
	outtextxy(40, 480, _T("find the difference,avoid some color"));
	_getch();
}

void Initialization()
{
	// room_speed = 60;
	settextcolor(Main_Color);
	setfillcolor(Main_Color);
	setlinecolor(Main_Color);
	setbkcolor(WHITE);
	cleardevice();
	memset(ma, 0, sizeof(ma));
	memset(food, 0, sizeof(food));
	memset(food_t, 0, sizeof(food_t));
	player.x = rand_int(5, NX - 5), player.y = rand_int(5, NY - 5);
	ma[player.x][player.y] = player.length = 3;
	Set_Food(1, -1, 0, 3);
	if (player.length == 1)
		nscore = 1893;
	else
		nscore = 0;
	spdscr = max(NX, NY) * 3;
	settextstyle(30, 0, _T("Fixedsys"));
	Game_Draw();
}

void Image_Init()
{
	ifstream filei("bg.jpg", ios::binary), fileb("bg", ios::binary);
	if (filei.good() && !fileb.good())
	{
		Image_Xor(filei, "bg");
		filei.close(), fileb.close();
		Image_Init();
		return;
	}
	else if (fileb.good())
	{
		Image_Xor(fileb, "temp");
		fileb.close();
		b_BACKGROUND = 1;
	}
	else
		b_BACKGROUND = 0;
	if (b_BACKGROUND)
	{
		filei.close();
		loadimage(&img, _T("temp"), NX * BLOCK_SIZE, NY * BLOCK_SIZE, true);
		remove("temp");
		printf("BACKGROUND ON\n");
	}
}

int main()
{
	/* Program Initialization */
	//PlaySound(TEXT("bgm.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	srand(time(NULL));
#ifndef RELEASE
	initgraph(800, 608, EW_SHOWCONSOLE);
#else
	initgraph(800, 608);
#endif
	set_theme(0);
	setfillcolor(Main_Color);
	setlinecolor(Main_Color);
	setbkcolor(WHITE);
	cleardevice();
	Image_Init();
	Initial_Draw();

	/* Game Loop */
	BeginBatchDraw();
	Initialization();
	Main_Loop();

	return 0;
}
