#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <windows.h>

#define WIDTH           10
#define HEIGHT          20
#define X_OFFSET        2
#define SEGMENT_SIZE    4
#define INTERVAL        300
#define TETROMINO_CNT   7


COORD coord;  
CONSOLE_CURSOR_INFO info;

typedef enum
{
    black    = 0,
    yellow   = 1,
    blue     = 2,
    red      = 3,
    magenta  = 4,
    green    = 5,
    cyan     = 6,
    brown    = 7,
    grey     = 8,
    white    = 15
} eTetrominoColor;

int LHit(int base_x, int base_y, int rotation, int tetromino);
int RHit(int base_x, int base_y, int rotation, int tetromino);
int BHit(int base_x, int base_y, int rotation, int tetromino);
void HitInfo(int base_x, int base_y, int rotation, int tetromino, int hit, int blk_size);
void BlkSave(int base_x, int base_y, int rotation, int tetromino);

int  Rotate   (int px, int py, int rotation, int tetromino);

void Display  (int px, int py, eTetrominoColor color);
void Tetromino(int px, int py, int rotation, int tetromino);
void Wall();

int BlkSize(int tetromino);
int Next_Tetromino();

#if 0
int blk[7] = { 0x2222, 0x2260, 0x4B00, 0x4980, 0x0F00, 0x1D00, 0x1980 };
#else
int blk[7][16] =
{
        //4x4
        {0,0,1,0, 0,0,1,0, 0,0,1,0, 0,0,1,0}, 
        {0,0,0,0, 0,2,2,0, 0,2,2,0, 0,0,0,0},
        // 3x3
        {0,3,0, 0,3,0, 3,3,0, 0,0,0, 0,0,0,0},
        {0,4,0, 0,4,0, 0,4,4, 0,0,0, 0,0,0,0},
        {0,0,0, 0,5,5, 5,5,0, 0,0,0, 0,0,0,0},  
        {0,0,0, 6,6,6, 0,6,0, 0,0,0, 0,0,0,0},
        {0,0,0, 7,7,0, 0,7,7, 0,0,0, 0,0,0,0}                            
};
#endif

int stack[WIDTH+2][HEIGHT+1];

int row_cnt[HEIGHT];
int score = 0;

int main()
{ 
    HANDLE hd = GetStdHandle(STD_OUTPUT_HANDLE);
    clock_t c_begin, c_end;

    char ch;
    int coord_x   = WIDTH/2;
    int coord_y  = -3;
    int rotation  = 0; // 0: 0 deg, 1: 90 deg, 2: 180 deg, 3: 270 deg
    int tetromino;
    int next_tetromino;
    int hit       = 0;
    int drop_flag = 0;

    info.dwSize   = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(hd, &info);

    Wall();

    c_begin = clock();

    int flag = 1;
    tetromino = Next_Tetromino();

    while(1)
    {
        if(flag)
        {
                next_tetromino = Next_Tetromino();
                flag = 0;
        }

        if (kbhit()) {

            ch = getch(); 
            
            if (ch == 27)  break;
            else{
                     if(ch == 72)    { rotation++;  rotation %= 4; } //72 (up), 80 (down), 75 (left) and 77 (right). 32(drop)
                else if(ch == 75)    { coord_x -= 1; hit = LHit(coord_x, coord_y, rotation, tetromino); }
                else if(ch == 77)    { coord_x += 1; hit = RHit(coord_x, coord_y, rotation, tetromino); } 
                else if(ch == 32)    { drop_flag= 1; } 
            }
        }

        if(hit & 0x2) coord_x += 1;
        if(hit & 0x4) coord_x -= 1; 

        hit = BHit(coord_x, coord_y, rotation, tetromino);
        
        if(hit & 0x1) coord_y -= 1;

        if(hit & 0x8) break;

        Tetromino(coord_x , coord_y, rotation, tetromino);

        c_end = clock();

        if( (c_end-c_begin) >= INTERVAL || coord_y >= HEIGHT || (hit & 0x1) || drop_flag) 
        {

            c_begin = clock();

            if( (hit & 0x1) || coord_y >= HEIGHT) // y+3 >= HEIGHT) 
            {
                flag=1;
                coord_x  = WIDTH/2;
                coord_y = (BlkSize(tetromino) > 1) ? -2 : -3;
                tetromino = next_tetromino;
                drop_flag = 0; 
            }   
            else{
                coord_y ++;
            }
  
        }

    } // while

    Display((WIDTH + 5)*2, HEIGHT, yellow);
    printf("END of GAME !!!");

#ifdef TETRIS_SOUND
    PlaySound(TEXT("점프5.mp3"), NULL, SND_FILENAME);
#endif

    return 0;
}

void Display  (int x, int y, eTetrominoColor color)
{
    HANDLE hd = GetStdHandle(STD_OUTPUT_HANDLE);

    coord.X = x;  
    coord.Y = y;
    SetConsoleCursorPosition(hd, coord);
    SetConsoleTextAttribute(hd, color);  
}

void Wall()
{
    HANDLE hd = GetStdHandle(STD_OUTPUT_HANDLE);

    system("cls"); 

    memset(stack, 0, sizeof(stack)/sizeof(int));

    SetConsoleTextAttribute(hd, grey);

    for(int y_scan=0; y_scan<=HEIGHT; y_scan++)    
    {
        coord.Y = y_scan;

        for(int x_scan=0; x_scan<=WIDTH+1; x_scan++)    
        {

            if(y_scan == HEIGHT)
            {
                stack[x_scan][y_scan]=grey; 

                Display(x_scan*2, y_scan, grey);
                printf("■");
            }
            else
            {
                if(x_scan==0 || x_scan==WIDTH+1)
                {
                    stack[x_scan][y_scan]=grey;  

                    Display(x_scan*2, y_scan, grey);
                    printf("■");         
                }
                else{
                    stack[x_scan][y_scan]=0;
                }

            }
         }           //for(wy=1; wy<=4; wy++)   stack[wx][HEIGHT+wy]=16;
    }    
     
}

int Rotate(int x, int y, int rotation, int tetromino)
{
	int position;

	switch (rotation)
	{
        case  0:  position = y * ((tetromino > 1) ? 3: 4) + x;                                  break;
        case  1:  position = ((tetromino > 1) ? 6 : 12) + y - x * ((tetromino > 1) ? 3 : 4);	break;
        case  2:  position = ((tetromino > 1) ? 8 : 15) - y * ((tetromino > 1) ? 3 : 4) - x;	break;
        case  3:  position = ((tetromino > 1) ? 2 :  3) - y + x * ((tetromino > 1) ? 3 : 4);	break;
        default:  position =  y * ((tetromino > 1) ? 3: 4) + x;                                 break;				
    }								

	return position;
}

int LHit(int base_x, int base_y, int rotation, int tetromino)
{

    int blk_size = BlkSize(tetromino);
    int x_scan, y_scan;
    int hit = 0;

    hit=BHit(base_x, base_y, rotation, tetromino);

    for(x_scan = 0; x_scan < blk_size; x_scan++)
    {
        for(y_scan = 0; y_scan < blk_size; y_scan++)
        {
            if(blk[tetromino][Rotate(x_scan, y_scan, rotation, tetromino)])
            {
                if(stack[base_x+x_scan][base_y+y_scan] || base_x+x_scan == 0) 
                {   hit |= 0x2;
                    break;
                }
            }
        }
        if(hit) break;
    }

    if(hit & 0x1) BlkSave(base_x, base_y, rotation, tetromino);

    HitInfo(base_x, base_y, rotation, tetromino, hit, blk_size);

    return hit; 
}

int RHit(int base_x, int base_y, int rotation, int tetromino)
{

    int blk_size = BlkSize(tetromino);
    int x_scan, y_scan;
    int hit = 0;

    hit=BHit(base_x, base_y, rotation, tetromino);

    for(x_scan = (blk_size-1); x_scan >=0; x_scan--)
    {
        for(y_scan = 0; y_scan < blk_size; y_scan++)
        {
            if(blk[tetromino][Rotate(x_scan, y_scan, rotation, tetromino)])
            {
                if(stack[base_x+x_scan][base_y+y_scan]) 
                {
                    hit |= 0x4;
                    break;
                }
            }
        }
       if(hit) break;
    }

    if(hit & 0x1) BlkSave(base_x, base_y, rotation, tetromino);

    HitInfo(base_x, base_y, rotation, tetromino, hit, blk_size);

    return hit; 
}

int BHit(int base_x, int base_y, int rotation, int tetromino)
{
    int blk_size = BlkSize(tetromino);
    int hit = 0x0;

    for(int y_scan=(blk_size -1); y_scan>=0; y_scan--)
    {
        for(int x_scan = 0; x_scan < blk_size; x_scan++)
        {                
            if(blk[tetromino][Rotate(x_scan, y_scan, rotation, tetromino)])
            {
                if(base_y+y_scan >=0)
                {
                    if(stack[base_x+x_scan][base_y+y_scan]) 
                    {
                        if(base_x + x_scan > 0 && base_x + x_scan <= WIDTH)
                        {
                            if(base_y+y_scan ==      0) hit  = 0x8;
                            if(base_y+y_scan > 0 &&  base_y+y_scan<=HEIGHT) hit |= 0x1;
                        }
                        break;
                    }
                }
            }
        }

        if(hit) break;
    }

    if(hit) BlkSave(base_x, base_y, rotation, tetromino);

    HitInfo(base_x, base_y, rotation, tetromino, hit, blk_size);
    //
    return hit; 
}

void BlkSave(int base_x, int base_y, int rotation, int tetromino)
{
    int blk_size = BlkSize(tetromino);

    for(int y_scan=0; y_scan < blk_size; y_scan++)
    {
        for(int x_scan = 0; x_scan < blk_size; x_scan++)
        {     
            if(base_x+x_scan >0  && base_x+x_scan <= WIDTH)
            stack[base_x+x_scan][base_y+y_scan-1] |= blk[tetromino][Rotate(x_scan, y_scan, rotation, tetromino)];
        }
    }
}

void HitInfo(int base_x, int base_y, int rotation, int tetromino, int hit, int blk_size)
{
    int degree;

    switch(rotation)
    {
        case 0: degree =   0; break;
        case 1: degree =  90; break;
        case 2: degree = 180; break;
        case 3: degree = 270; break;                
    }

    char symbol;
    
    switch(tetromino)
    {
        case 0: symbol = 'I'; break;
        case 1: symbol = 'O'; break;
        case 2: symbol = 'J'; break;
        case 3: symbol = 'L'; break;   
        case 4: symbol = 'S'; break;
        case 5: symbol = 'T'; break;
        case 6: symbol = 'Z'; break;                        
    }

    Display((WIDTH + 5)*2, 0, yellow);
    printf("(%03d, %03d): %c-Tetromino(%d) %03d-Deg Hit=%d BLK_Size=%02d", base_x, base_y, symbol, tetromino, degree, hit, blk_size);
}


void Tetromino(int x, int y, int rotation, int tetromino)
{
    HANDLE hd = GetStdHandle(STD_OUTPUT_HANDLE);

    int blk_size = BlkSize(tetromino);

    for(int y_sweep = 0; y_sweep < HEIGHT; y_sweep++)
    {
        row_cnt[y_sweep] = 0;

        for(int x_sweep = 1; x_sweep <= WIDTH; x_sweep++)
        {
            coord.X = x_sweep*2; 
            coord.Y = y_sweep;
            SetConsoleCursorPosition(hd, coord);  

            if(stack[x_sweep][y_sweep])  row_cnt[y_sweep]++;

            if(x_sweep >= x && x_sweep <= x+(blk_size-1) && y_sweep >= y && y_sweep<= y+(blk_size-1))
            {    
                SetConsoleTextAttribute(hd, blk[tetromino][Rotate(x_sweep-x, y_sweep-y, rotation, tetromino)]);

                if(blk[tetromino][Rotate(x_sweep-x, y_sweep-y, rotation, tetromino)])  printf("■");
                else                                                                   printf(" ");
            }  
            else
            {
                if(stack[x_sweep][y_sweep])
                {
                    SetConsoleTextAttribute(hd, stack[x_sweep][y_sweep]);
                    printf("■");
                }  
                else    
                    printf(" ");
            }
           
        }
    }

    int y_shift = HEIGHT - 1;
    int y_flag  = 0;

    for(int y_sweep = HEIGHT - 1; y_sweep >=0; y_sweep--)
    {  
        if(row_cnt[y_sweep] != WIDTH)
        { 
            for(int x_sweep = 1; x_sweep <= WIDTH; x_sweep++)
            {
                stack[x_sweep][y_shift] = stack[x_sweep][y_sweep];

            }
            y_shift--;
            y_flag = 1;
        }
        else
        {
            if(y_flag)
            //PlaySound(TEXT("Vanish.wav"), NULL, SND_ASYNC);
            score++; score/(WIDTH-1);
            y_flag = 0;
        }
    }

    for(int y_sweep = y_shift; y_sweep >=0; y_sweep--)
    {  
            for(int x_sweep = 1; x_sweep <= WIDTH; x_sweep++)
            {
                stack[x_sweep][y_sweep] = 0;
            }
    }

    Display((WIDTH + 5)*2, 1, yellow);
    printf("SCORE %d", score);
}

int Next_Tetromino()
{
    srand(time(NULL));

    int  next_tetromino = rand()%TETROMINO_CNT;
    int size = BlkSize(next_tetromino);

    for(int x_preview= 0; x_preview < 4; x_preview++)
    {
        for(int y_preview= 0; y_preview < 4; y_preview++)
        {
            Display((WIDTH+5)*2 + x_preview*2, 5+y_preview, (eTetrominoColor)(next_tetromino+1));

            if((x_preview ==3 || y_preview ==3) && size==3)                              
            {
                printf(" ");
            }
            else
            {
                if(blk[next_tetromino][Rotate(x_preview, y_preview, 0, next_tetromino)])  printf("■");
                else                                                                      printf(" ");                                 
            }
            
        }
    }
    return next_tetromino;
}

int BlkSize(int tetromino)
{
    int blk_size = 4;
        
    switch(tetromino)
    {
        case 0: blk_size=4; break;
        case 1: blk_size=4; break;   
        case 2: blk_size=3; break; 
        case 3: blk_size=3; break; 
        case 4: blk_size=3; break; 
        case 5: blk_size=3; break; 
        case 6: blk_size=3; break; 
    }
    return blk_size;
}